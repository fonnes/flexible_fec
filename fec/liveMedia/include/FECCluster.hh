#ifndef _FEC_CLUSTER_HH
#define _FEC_CLUSTER_HH

#include "RTPPacket.hh"
#include <liveMedia.hh>

class FECCluster {
public:
    static FECCluster* createNew(u_int16_t base, u_int8_t row, u_int8_t column);
    void insertPacket(RTPPacket* rtpPacket);
    Boolean seqNumInCluster(u_int16_t seqNum);
    Boolean allRTPPacketsArePresent();
    Boolean hasExpired(long long repairWindow);

    Boolean hasOnlyNullPackets();


    RTPPacket** rtpPackets() const {return fRTPPackets;}
    u_int16_t base() const {return fBase;}
    long long timestamp() const {return fTimestamp;}

    int sourcePacketCount() const {return fRow * fColumn;}

private:
    FECCluster(u_int16_t base, u_int8_t row, u_int8_t column);
    int getIndex(RTPPacket* rtpPacket);
private:
    RTPPacket** fRTPPackets;
    u_int16_t fBase;
    long long fTimestamp;
    u_int8_t fRow;
    u_int8_t fColumn;
};
#endif
