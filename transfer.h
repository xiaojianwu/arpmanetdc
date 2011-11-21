#ifndef TRANSFER_H
#define TRANSFER_H

#include <QObject>
#include <QByteArray>
#include <QHostAddress>
#include <QList>
#include <QFile>
#include <QTimer>

#define TRANSFER_STATE_PAUSED 0
#define TRANSFER_STATE_INITIALIZING 1
#define TRANSFER_STATE_RUNNING 2
#define TRANSFER_STATE_STALLED 3
#define TRANSFER_STATE_ABORTING 4
#define TRANSFER_STATE_FINISHED 5

#define TRANSFER_TYPE_UPLOAD 0
#define TRANSFER_TYPE_DOWNLOAD 1

enum DownloadProtocolInstructions
{
    DataPacket=0xaa,
    ProtocolADataPacket=0x21,
    ProtocolBDataPacket=0x22,
    ProtocolCDataPacket=0x23,
    ProtocolDDataPacket=0x24,
    ProtocolAControlPacket=0x41,
    ProtocolBControlPacket=0x42,
    ProtocolCControlPacket=0x43,
    ProtocolDControlPacket=0x44
};

class Transfer : public QObject
{
    Q_OBJECT
public:
    explicit Transfer(QObject *parent = 0);
    virtual ~Transfer();

signals:
    void abort(Transfer*);
    void hashBucketRequest(QByteArray &rootTTH, int &bucketNumber, QByteArray *bucket);
    void TTHTreeRequest(QByteArray &rootTTH, QHostAddress &hostAddr);
    void transmitDatagram(QHostAddress &dstHost, QByteArray &datagram);

public slots:
    void setFileName(QString &filename);
    void setTTH(QByteArray &tth);
    void setFileOffset(quint64 offset);
    void setSegmentLength(quint64 length);
    void setRemoteHost(QHostAddress remote);
    void setTransferProtocol(quint8 protocol);
    QByteArray* getTTH();
    QString* getFileName();
    QHostAddress* getRemoteHost();
    quint64* getTransferRate();
    int* getTransferStatus();
    int* getTransferProgress();
    void hashBucketReply(int &bucketNumber, QByteArray &bucketTTH);
    void addPeer(QHostAddress &peer);
    void TTHTreeReply(QByteArray &rootTTH, QByteArray &tree);
    virtual void incomingDataPacket(quint8 transferProtocolVersion, quint64 &offset, QByteArray &data);
    virtual int getTransferType() = 0;
    virtual void startTransfer() = 0;
    virtual void pauseTransfer() = 0;
    virtual void abortTransfer() = 0;
    virtual void transferRateCalculation() = 0;

 //   virtual void receiveData(QByteArray &data) = 0;

protected:
    QByteArray TTH;
    QString filePathName;
    quint8 transferProtocol;
    QHostAddress remoteHost;
    quint64 fileOffset;
    quint64 segmentLength;
    int status;
    QList<QHostAddress> listOfPeers;
    QTimer *transferRateCalculationTimer;
    quint64 transferRate;
    int transferProgress;
};

#endif // TRANSFER_H
