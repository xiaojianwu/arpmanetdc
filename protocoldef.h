#ifndef PROTOCOLDEF_H
#define PROTOCOLDEF_H

enum TransferProtocol
{
    ProtocolA=0x01,
    ProtocolB=0x02,
    ProtocolC=0x04,
    ProtocolD=0x08
};

enum MajorPacketType
{
    DataPacket=0xaa,
    UnicastPacket=0x55,
    BroadcastPacket=0x5a,
    MulticastPacket=0xa5
};

enum MinorPacketType
{
    SearchRequestPacket=0x11,
    SearchForwardRequestPacket=0x12,
    SearchResultPacket=0x13,
    TTHSearchRequestPacket=0x14,
    TTHSearchForwardRequestPacket=0x15,
    TTHSearchResultPacket=0x16,
    TransferErrorPacket=0x20,
    DownloadRequestPacket=0x21,
    DataPacketA=0x31,
    DataPacketB=0x32,
    DataPacketC=0x33,
    DataPacketD=0x34,
    TTHTreeRequestPacket=0x41,
    TTHTreeReplyPacket=0x42,
    AnnouncePacket=0x71,
    AnnounceForwardRequestPacket=0x72,
    AnnounceForwardedPacket=0x73,
    AnnounceReplyPacket=0x74,
    RequestBucketPacket=0x81,
    RequestAllBucketsPacket=0x82,
    BucketExchangePacket=0x83,
    CIDPingPacket=0x91,
    CIDPingForwardRequestPacket=0x92,
    CIDPingForwardedPacket=0x93,
    CIDPingReplyPacket=0x94,
    RevConnectPacket=0xa1,
    RevConnectReplyPacket=0xa2
};

enum DownloadProtocolInstructions
{
    ProtocolADataPacket=0x21,
    ProtocolBDataPacket=0x22,
    ProtocolCDataPacket=0x23,
    ProtocolDDataPacket=0x24,
    ProtocolAControlPacket=0x41,
    ProtocolBControlPacket=0x42,
    ProtocolCControlPacket=0x43,
    ProtocolDControlPacket=0x44
};

#endif // PROTOCOLDEF_H