#ifndef _FEC_MULTIPLEXOR_HH
#define _FEC_MULTIPLEXOR_HH

#include <queue>
#include "FramedSource.hh"
#include "RTPPacket.hh"

class FECMultiplexor: public FramedSource {
protected:
    FECMultiplexor(UsageEnvironment& env);
    ~FECMultiplexor();

protected:
    std::queue<RTPPacket*> fRTPPackets;
    EventTriggerId fEventTriggerId;

private:
    virtual void doGetNextFrame();
    static void deliverFrame0(void* clientData);
    void deliverFrame();
};
#endif
