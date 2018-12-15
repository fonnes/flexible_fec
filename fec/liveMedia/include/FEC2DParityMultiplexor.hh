#ifndef _FEC_2D_PARITY_MULTIPLEXOR_HH
#define _FEC_2D_PARITY_MULTIPLEXOR_HH

#include <vector>
#include "FECMultiplexor.hh"
#include "FECPacket.hh"
#include "FECDecoder.hh"
#include "FECCluster.hh"

class FEC2DParityMultiplexor : public FECMultiplexor {
public:
    static FEC2DParityMultiplexor* createNew(UsageEnvironment& env, u_int8_t row, u_int8_t column, long long repairWindow);
    FEC2DParityMultiplexor(UsageEnvironment& env, u_int8_t row, u_int8_t column, long long repairWindow);
    ~FEC2DParityMultiplexor();
    void pushFECRTPPacket(unsigned char* buffer, unsigned bufferSize);

public:
    Boolean first;


    std::vector<RTPPacket*> emergencyBuffer;
    std::vector<FECCluster*> superBuffer;

    u_int16_t currentSequenceNumber;
private:
    static void sendNext(void* firstArg);
    void repairPackets();

    void findBase(int* didFind, u_int16_t* newBase);
    FECCluster* findCluster(u_int16_t seqNum);
    void handleEmergencyBuffer(u_int16_t base);
    void flushCluster(RTPPacket** cluster);

    void updateCurrentSequenceNumber(u_int16_t newSeqnum, unsigned sourcePacketCount);

    void setHostSSRCIfNotSet(RTPPacket* rtpPacket);

    void setBaseIfNotSet(RTPPacket* rtpPacket);

    void insertPacket(RTPPacket* rtpPacket);

    void printSuperBuffer();

private:
    long long fRepairWindow;

    Boolean ssrcWasSet;
    unsigned hostSSRC;

    u_int8_t fRow;
    u_int8_t fColumn;

    int timeoutCounter;
    int whenToTimeout;

};
#endif
