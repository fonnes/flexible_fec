#include <liveMedia.hh>
#include <BasicUsageEnvironment.hh>
#include <GroupsockHelper.hh>

#include "../liveMedia/include/FECGroupsock.hh"
#include "../liveMedia/include/FECNonInterleavedSource.hh"

UsageEnvironment* env;
char const* inputFileName = "";
//char const* inputFileName = "test.264";
//char const* inputFileName = "/Users/simen/Documents/uio/preliminary/final.264";
H264VideoStreamFramer* videoSource;
RTPSink* videoSink;

//http://lists.live555.com/pipermail/live-devel/2015-August/019579.html
FramedSource* fecNonInterleavedSource;
FramedSource* fecInterleavedSource;

RTPSink* fecNonInterleavedSink;
RTPSink* fecInterleavedSink;

void play(); // forward

int main(int argc, char** argv) {
    TaskScheduler* scheduler = BasicTaskScheduler::createNew();
    env = BasicUsageEnvironment::createNew(*scheduler);

    struct in_addr destinationAddress;
    destinationAddress.s_addr = our_inet_addr("232.126.107.222"); //chooseRandomIPv4SSMAddress(*env);

    const unsigned short rtpPortNum = 18888;
    const unsigned short rtcpPortNum = rtpPortNum+1;
    const unsigned short fecNonInterleavedPortNum = rtcpPortNum+1;
    const unsigned short fecInterleavedPortNum = fecNonInterleavedPortNum+2;

    const unsigned char ttl = 255;

    const Port rtpPort(rtpPortNum);
    const Port rtcpPort(rtcpPortNum);
    const Port fecNonInterleavedPort(fecNonInterleavedPortNum);
    const Port fecInterleavedPort(fecInterleavedPortNum);

    u_int8_t column = atoi(argv[1]);
    u_int8_t row = atoi(argv[2]);
    inputFileName = argv[3];

    FECNonInterleavedSource* myFECNonInterleavedSource = FECNonInterleavedSource::createNew(*env, row, column);
    FECInterleavedSource* myFECInterleavedSource = FECInterleavedSource::createNew(*env, row, column);

    FECGroupsock rtpGroupsock(*env, destinationAddress, rtpPort, ttl, myFECNonInterleavedSource, myFECInterleavedSource);
    rtpGroupsock.multicastSendOnly(); // we're a SSM source

    Groupsock rtcpGroupsock(*env, destinationAddress, rtcpPort, ttl);
    rtcpGroupsock.multicastSendOnly(); // we're a SSM source

    Groupsock fecNonInterleavedGroupsock(*env, destinationAddress, fecNonInterleavedPort, ttl);
    fecNonInterleavedGroupsock.multicastSendOnly(); // we're a SSM source

    Groupsock fecInterleavedGroupsock(*env, destinationAddress, fecInterleavedPort, ttl);
    fecInterleavedGroupsock.multicastSendOnly(); // we're a SSM source

    fecNonInterleavedSource = myFECNonInterleavedSource;
    fecInterleavedSource = myFECInterleavedSource;

    // Create a 'H265 Video RTP' sink from the RTP 'groupsock':
    OutPacketBuffer::maxSize = 100000;
    videoSink = H264VideoRTPSink::createNew(*env, &rtpGroupsock, 96);
    ((H264VideoRTPSink*)videoSink)->setPacketSizes(1444, 1444); //HAX















    fecNonInterleavedSink = SimpleRTPSink::createNew(*env, &fecNonInterleavedGroupsock, 115, 96000, "fec", "", 1, False, False);   //kanskje false, false
    fecInterleavedSink = SimpleRTPSink::createNew(*env, &fecInterleavedGroupsock, 116, 96000, "fec", "", 1, False, False);

    // Create (and start) a 'RTCP instance' for this RTP sink:
    const unsigned estimatedSessionBandwidth = 500; // in kbps; for RTCP b/w share
    const unsigned maxCNAMElen = 100;
    unsigned char CNAME[maxCNAMElen+1];
    gethostname((char*)CNAME, maxCNAMElen);
    CNAME[maxCNAMElen] = '\0'; // just in case
    RTCPInstance* rtcp = RTCPInstance::createNew(*env, &rtcpGroupsock, estimatedSessionBandwidth, CNAME, videoSink, NULL, True);
    // Note: This starts RTCP running automatically

    RTSPServer* rtspServer = RTSPServer::createNew(*env, 8554);
    if (rtspServer == NULL) {
        *env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
        exit(1);
    }
    ServerMediaSession* sms = ServerMediaSession::createNew(*env, "testStream", inputFileName, "Session streamed by \"testH264VideoStreamer\"", True /*SSM*/);
    sms->addSubsession(PassiveServerMediaSubsession::createNew(*videoSink, rtcp));
    rtspServer->addServerMediaSession(sms);

    char* url = rtspServer->rtspURL(sms);
    *env << "Play this stream using the URL \"" << url << "\"\n";
    delete[] url;

    // Start the streaming:
    *env << "Beginning streaming...\n";
    play();

    u_int8_t first = destinationAddress.s_addr >> 24;
    u_int8_t second = destinationAddress.s_addr >> 16;
    u_int8_t third = destinationAddress.s_addr >> 8;
    u_int8_t fourth = destinationAddress.s_addr;

    *env << fourth << "." << third << "." << second << "." << first << "\n";

    env->taskScheduler().doEventLoop(); // does not return

    return 0; // only to prevent compiler warning
}

void afterPlaying(void* /*clientData*/) {
  *env << "...done reading from file\n";
  videoSink->stopPlaying();
  fecNonInterleavedSink->stopPlaying();
  fecInterleavedSink->stopPlaying();
  Medium::close(videoSource);
  Medium::close(fecNonInterleavedSource);
  Medium::close(fecInterleavedSource);

  exit(0);
  //play();
}

void play() {
    // Open the input file as a 'byte-stream file source':
    ByteStreamFileSource* fileSource = ByteStreamFileSource::createNew(*env, inputFileName);
    if (fileSource == NULL) {
        *env << "Unable to open file \"" << inputFileName << "\" as a byte-stream file source\n";
        exit(1);
    }

    FramedSource* videoES = fileSource;

    // Create a framer for the Video Elementary Stream:
    videoSource = H264VideoStreamFramer::createNew(*env, videoES);

    // Finally, start playing:
    *env << "Beginning to read from file...\n";
    videoSink->startPlaying(*videoSource, afterPlaying, videoSink);

    fecNonInterleavedSink->startPlaying(*fecNonInterleavedSource, afterPlaying, fecNonInterleavedSink);
    fecInterleavedSink->startPlaying(*fecInterleavedSource, afterPlaying, fecInterleavedSink);

}
