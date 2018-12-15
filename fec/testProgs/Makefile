LIVE_DIR = ../../libs/live/
INCLUDES = -I$(LIVE_DIR)UsageEnvironment/include -I$(LIVE_DIR)groupsock/include -I$(LIVE_DIR)liveMedia/include -I$(LIVE_DIR)BasicUsageEnvironment/include -I.
LIBS = $(LIVE_DIR)liveMedia/libliveMedia.a $(LIVE_DIR)groupsock/libgroupsock.a $(LIVE_DIR)BasicUsageEnvironment/libBasicUsageEnvironment.a $(LIVE_DIR)UsageEnvironment/libUsageEnvironment.a

TEST_H264_VIDEO_STREAMER = testH264VideoStreamer
CLIENT = Client
FEC_GROUPSOCK = ../liveMedia/FECGroupsock
FEC_NON_INTERLEAVED_SOURCE = ../liveMedia/FECNonInterleavedSource
RTP_PACKET = ../liveMedia/RTPPacket
FEC_PACKET = ../liveMedia/FECPacket
FEC_INTERLEAVED_SOURCE = ../liveMedia/FECInterleavedSource
FEC_ENCODER = ../liveMedia/FECEncoder
FEC_SINK = ../liveMedia/FECSink
FEC_MULTIPLEXOR = ../liveMedia/FECMultiplexor
FEC_DECODER = ../liveMedia/FECDecoder
FEC_CLUSTER = ../liveMedia/FECCluster
FEC_2D_PARITY_MULTIPLEXOR = ../liveMedia/FEC2DParityMultiplexor

main:
	c++ -c -g $(INCLUDES)  -DBSD=1 -O -DSOCKLEN_T=socklen_t -DHAVE_SOCKADDR_LEN=1 -DTIME_BASE=int -Wall ./$(TEST_H264_VIDEO_STREAMER).cpp
	c++ -g -o $(TEST_H264_VIDEO_STREAMER) -L. $(FEC_GROUPSOCK).o $(TEST_H264_VIDEO_STREAMER).o $(FEC_NON_INTERLEAVED_SOURCE).o $(RTP_PACKET).o $(FEC_PACKET).o $(FEC_INTERLEAVED_SOURCE).o $(FEC_ENCODER).o $(LIBS)

client:
	c++ -c $(INCLUDES)  -DBSD=1 -O -DSOCKLEN_T=socklen_t -DHAVE_SOCKADDR_LEN=1 -DTIME_BASE=int -Wall ./Client.cpp
	c++ -o client -L. $(CLIENT).o $(FEC_SINK).o $(FEC_MULTIPLEXOR).o $(RTP_PACKET).o $(FEC_PACKET).o $(FEC_DECODER).o $(FEC_CLUSTER).o $(FEC_2D_PARITY_MULTIPLEXOR).o $(LIBS)

clean:
	rm -f $(TEST_H264_VIDEO_STREAMER)
	rm -f client
	rm -f *.o
