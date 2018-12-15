#include "liveMedia.hh"
#include "GroupsockHelper.hh"

#include "BasicUsageEnvironment.hh"
#include "../liveMedia/include/FECSink.hh"
#include "../liveMedia/include/FEC2DParityMultiplexor.hh"

void afterPlaying(void* clientData); // forward

// A structure to hold the state of the current session.
// It is used in the "afterPlaying()" function to clean up the session.
struct sessionState_t {
    FEC2DParityMultiplexor* fec2DParityMultiplexor;

    FramedSource* rtpSource;
    FramedSource* fecSource;
    FramedSource* fecInterleavedSource;

    FECSink* rtpSink;
    FECSink* fecSink;
    FECSink* fecInterleavedSink;
    //FileSink* fileSink;
    BasicUDPSink* bus;

    RTCPInstance* rtcpInstance;
} sessionState;

UsageEnvironment* env;

int main(int argc, char** argv) {
    TaskScheduler* scheduler = BasicTaskScheduler::createNew();
    env = BasicUsageEnvironment::createNew(*scheduler);

    char const* sessionAddressStr = argv[1];
    u_int8_t column = atoi(argv[2]);
    u_int8_t row = atoi(argv[3]);
    long long repairWindow = atoi(argv[4]);

    sessionState.fec2DParityMultiplexor = FEC2DParityMultiplexor::createNew(*env, row, column, repairWindow);

    sessionState.rtpSink = FECSink::createNew(*env, sessionState.fec2DParityMultiplexor);
    sessionState.fecSink = FECSink::createNew(*env, sessionState.fec2DParityMultiplexor);
    sessionState.fecInterleavedSink = FECSink::createNew(*env, sessionState.fec2DParityMultiplexor);
    //sessionState.fileSink = FileSink::createNew(*env, "khgjgygyj.264");

    const unsigned char ttl = 1; // low, in case routers don't admin scope

    const unsigned short rtpPortNum = 18888;
    const unsigned short rtcpPortNum = rtpPortNum+1;
    const unsigned short fecPortNum = 18890;
    const unsigned short fecInterleavedPortNum = 18892;
    const unsigned short forwardPortNum = 18894;

    struct in_addr sessionAddress;
    sessionAddress.s_addr = our_inet_addr(sessionAddressStr);
    const Port rtpPort(rtpPortNum);
    const Port fecPort(fecPortNum);
    const Port fecInterleavedPort(fecInterleavedPortNum);
    const Port forwardPort(forwardPortNum);

    Groupsock rtpGroupsock(*env, sessionAddress, rtpPort, ttl);
    Groupsock fecGroupsock(*env, sessionAddress, fecPort, ttl);
    Groupsock fecInterleavedGroupsock(*env, sessionAddress, fecInterleavedPort, ttl);

    struct in_addr mySessionAddr;
    mySessionAddr.s_addr = our_inet_addr("232.126.107.111");
    Groupsock forwardGroupsock(*env, mySessionAddr, forwardPort, 255);


    //test:
    sessionState.bus = BasicUDPSink::createNew(*env, &forwardGroupsock, 100000);


    sessionState.fecSource = BasicUDPSource::createNew(*env, &rtpGroupsock);
    sessionState.rtpSource = BasicUDPSource::createNew(*env, &fecGroupsock);
    sessionState.fecInterleavedSource = BasicUDPSource::createNew(*env, &fecInterleavedGroupsock);

    // Finally, start receiving the multicast stream:
    *env << "Beginning receiving multicast stream...\n";
    sessionState.rtpSink->startPlaying(*sessionState.rtpSource, afterPlaying, NULL);
    sessionState.fecSink->startPlaying(*sessionState.fecSource, afterPlaying, NULL);
    sessionState.fecInterleavedSink->startPlaying(*sessionState.fecInterleavedSource, afterPlaying, NULL);
    //sessionState.fileSink->startPlaying(*sessionState.fec2DParityMultiplexor, afterPlaying, NULL);

    sessionState.bus->startPlaying(*sessionState.fec2DParityMultiplexor, afterPlaying, NULL);

    env->taskScheduler().doEventLoop(); // does not return

    return 0; // only to prevent compiler warning
}


void afterPlaying(void* /*clientData*/) {
    *env << "...done receiving\n";

    // End by closing the media:
    Medium::close(sessionState.rtcpInstance); // Note: Sends a RTCP BYE
    //Medium::close(sessionState.sink);
    //Medium::close(sessionState.fecSource);
}
