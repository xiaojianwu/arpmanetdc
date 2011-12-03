#include "fstptransfersegment.h"

FSTPTransferSegment::FSTPTransferSegment(Transfer *parent) : TransferSegment(parent)
{
    requestingOffset = 0;
    //requestingLength = 65536;
    requestingLength = 131072;
    requestingTargetOffset = 0;
    retransmitTimeoutCounter = 0;
    retransmitRetryCounter = 0;

    pParent = parent;
}

FSTPTransferSegment::~FSTPTransferSegment()
{
    if (inputFile.isOpen())
        inputFile.close();
}

void FSTPTransferSegment::setFileName(QString filename)
{
    filePathName = filename;
    inputFile.setFileName(filePathName);
    inputFile.open(QIODevice::ReadOnly);
    fileSize = inputFile.size();
}

void FSTPTransferSegment::setFileSize(quint64 size)
{
    fileSize = size;
}

void FSTPTransferSegment::setLastBucketNumber(int n)
{
    lastBucketNumber = n;
}

void FSTPTransferSegment::setLastBucketSize(int size)
{
    lastBucketSize = size;
}

void FSTPTransferSegment::startUploading()
{
    if (segmentStart > fileSize)
        return;
    else if (segmentStart + segmentLength > fileSize)
        segmentLength = fileSize - segmentStart;

    const char * f = (char*)inputFile.map(segmentStart, segmentLength);
    quint64 wptr = 0;
    QByteArray header;
    header.append(DataPacket);
    header.append(FailsafeTransferProtocol);
    QByteArray data(QByteArray::fromRawData(f, segmentLength));
    while (wptr < segmentLength)
    {
        QByteArray *packet = new QByteArray(header);
        packet->append(toQByteArray((quint64)(segmentStart + wptr)));
        packet->append(TTH);
        //qDebug() << "Write data segmentStart " << segmentStart << " segmentLength " << segmentLength << " wptr " << wptr;
        if (wptr + PACKET_DATA_MTU < segmentLength)
        {
            packet->append(data.mid(wptr, PACKET_DATA_MTU));
            wptr += PACKET_DATA_MTU;
        }
        else
        {
            packet->append(data.mid(wptr, segmentLength - wptr));
            wptr += segmentLength - wptr;
        }
        emit transmitDatagram(remoteHost, packet);
    }
    inputFile.unmap((unsigned char *)f);
}

void FSTPTransferSegment::startDownloading()
{
    segmentStartTime = QDateTime::currentMSecsSinceEpoch();
    requestingOffset = segmentStart;
    qDebug() << "FSTPTransferSegment::startDownloading() call checkSendDownloadRequest()";
    checkSendDownloadRequest(FailsafeTransferProtocol, remoteHost, TTH, requestingOffset, requestingLength);
    status = TRANSFER_STATE_RUNNING;
}

inline void FSTPTransferSegment::checkSendDownloadRequest(quint8 protocol, QHostAddress peer, QByteArray TTH,
                                                       quint64 requestingOffset, qint64 requestingLength)
{
    if (segmentEnd < requestingOffset + requestingLength)
        requestingLength = segmentEnd - requestingOffset;
    if (requestingLength > 0)
    {
        qDebug() << "FSTPTransferSegment::checkSendDownloadRequest() emit sendDownloadRequest() peer tth offset length "
                 << peer << TTH.toBase64() << requestingOffset << requestingLength;
        emit sendDownloadRequest(protocol, peer, TTH, requestingOffset, requestingLength);
    }
}

void FSTPTransferSegment::incomingDataPacket(quint64 offset, QByteArray data)
{
    if (offset > requestingOffset)
    {
        status = TRANSFER_STATE_STALLED;
        return;
    }
    if (offset < requestingOffset)
        return;

    status = TRANSFER_STATE_RUNNING;
    packetsSinceUpdate++;

    int bucketNumber = calculateBucketNumber(offset);
    if (!pDownloadBucketTable->contains(bucketNumber))
    {
        QByteArray *bucket = new QByteArray();
        pDownloadBucketTable->insert(bucketNumber, bucket);
    }
    if ((pDownloadBucketTable->value(bucketNumber)->length() + data.length()) > HASH_BUCKET_SIZE)
    {
        int bucketRemaining = HASH_BUCKET_SIZE - pDownloadBucketTable->value(bucketNumber)->length();
        pDownloadBucketTable->value(bucketNumber)->append(data.mid(0, bucketRemaining));
        if (!pDownloadBucketTable->contains(bucketNumber + 1))
        {
            QByteArray *nextBucket = new QByteArray(data.mid(bucketRemaining));
            pDownloadBucketTable->insert(bucketNumber + 1, nextBucket);
        }
        // there should be no else - if the next bucket exists and data is sticking over, there is an error,
        // since we segment on bucket boundaries. tth checksumming will catch the problems.
    }
    else
        pDownloadBucketTable->value(bucketNumber)->append(data);

    if (pDownloadBucketTable->value(bucketNumber)->length() == HASH_BUCKET_SIZE)
    {
        emit hashBucketRequest(TTH, bucketNumber, pDownloadBucketTable->value(bucketNumber));
    }

    // these last bucket numbers are for the *segment*, not the file.
    // the length check is for in case it is also the last segment of the file.
    if ((bucketNumber == lastBucketNumber) && (lastBucketSize == pDownloadBucketTable->value(bucketNumber)->length())) // EoF
    {
        status = TRANSFER_STATE_FINISHED;  // local segment
        emit hashBucketRequest(TTH, bucketNumber, pDownloadBucketTable->value(bucketNumber));
        return;
    }

    requestingOffset += data.length();
    if (requestingOffset == requestingTargetOffset)
    {
        if (requestingLength <= FSTP_TRANSFER_MAXIMUM_SEGMENT / 2)
            requestingLength *= 2;

        requestingTargetOffset += requestingLength;
        qDebug() << "FSTPTransferSegment::incomingDataPacket() call checkSendDownloadRequest()";
        checkSendDownloadRequest(FailsafeTransferProtocol, remoteHost, TTH, requestingOffset, requestingLength);
    }
    if (requestingOffset >= segmentEnd)
    {
        emit requestNextSegment(this);
    }
}

void FSTPTransferSegment::transferTimerEvent()
{
    if (status == TRANSFER_STATE_STALLED)
    {
        // Transfer some data
        if (requestingLength > FSTP_TRANSFER_MINIMUM_SEGMENT)
            requestingLength /= 2;
        status = TRANSFER_STATE_RUNNING;
        requestingTargetOffset = requestingOffset + requestingLength;
        qDebug() << "FSTPTransferSegment::transferTimerEvent() call checkSendDownloadRequest()" << requestingOffset << requestingLength;
        checkSendDownloadRequest(FailsafeTransferProtocol, remoteHost, TTH, requestingOffset, requestingLength);
        retransmitRetryCounter++;
        if (retransmitRetryCounter == 30)
        {
            status = TRANSFER_STATE_FAILED;
            emit transferRequestFailed(this);
        }
    }
    else if (status == TRANSFER_STATE_RUNNING)
    {
        if (packetsSinceUpdate == 0)
        {
            retransmitTimeoutCounter++;
            if (retransmitTimeoutCounter > 20)
            {
                status = TRANSFER_STATE_STALLED;
                retransmitTimeoutCounter = 0;
            }
        }
        else
        {
            packetsSinceUpdate = 0;
            //By adding this we only set a transfer as STALLED when 20 *sequential* updates produced no packets, vs just 20 overall
            retransmitTimeoutCounter = 0;
        }
    }
}

inline int FSTPTransferSegment::calculateBucketNumber(quint64 fileOffset)
{
    return (int)(fileOffset >> 20);
}

