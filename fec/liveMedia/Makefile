LIVE_DIR = ../../libs/live/
INCLUDES = -I$(LIVE_DIR)UsageEnvironment/include -I$(LIVE_DIR)groupsock/include -I$(LIVE_DIR)liveMedia/include -I$(LIVE_DIR)BasicUsageEnvironment/include -I.
LIBS = $(LIVE_DIR)liveMedia/libliveMedia.a $(LIVE_DIR)groupsock/libgroupsock.a $(LIVE_DIR)BasicUsageEnvironment/libBasicUsageEnvironment.a $(LIVE_DIR)UsageEnvironment/libUsageEnvironment.a

all:
	c++ -c -g $(INCLUDES)  -DBSD=1 -O -DSOCKLEN_T=socklen_t -DHAVE_SOCKADDR_LEN=1 -DTIME_BASE=int -Wall ./FECGroupsock.cpp ./FECNonInterleavedSource.cpp ./RTPPacket.cpp ./FECPacket.cpp ./FECInterleavedSource.cpp ./FECEncoder.cpp ./FECSink.cpp ./FECMultiplexor.cpp ./FECDecoder.cpp ./FECCluster.cpp ./FEC2DParityMultiplexor.cpp

clean:
	rm -f *.o
