#include "include/FECMultiplexor.hh"

FECMultiplexor::FECMultiplexor(UsageEnvironment& env) : FramedSource(env) {
	if (fEventTriggerId == 0)
		fEventTriggerId = envir().taskScheduler().createEventTrigger(deliverFrame0);
}

FECMultiplexor::~FECMultiplexor() {
	std::queue<RTPPacket*> empty;
   	std::swap(fRTPPackets, empty);

	// Reclaim our 'event trigger'
    envir().taskScheduler().deleteEventTrigger(fEventTriggerId);
    fEventTriggerId = 0;
}

void FECMultiplexor::doGetNextFrame() {
	/*if (fRTPPackets == NULL) {
		handleClosure();
		return;
	}*/
	if (fRTPPackets.size() > 0) deliverFrame();
}

void FECMultiplexor::deliverFrame() {
	if (!isCurrentlyAwaitingData()) return; // we're not ready for the data yet

	RTPPacket* rtpPacket = fRTPPackets.front();
	fRTPPackets.pop();

	if (rtpPacket->size() > fMaxSize) {
		fFrameSize = fMaxSize;
		fNumTruncatedBytes = rtpPacket->size() - fMaxSize;
  	}
	else fFrameSize = rtpPacket->size();

	gettimeofday(&fPresentationTime, NULL);
	memmove(fTo, rtpPacket->content(), fFrameSize);

	delete rtpPacket;

	FramedSource::afterGetting(this);
}

void FECMultiplexor::deliverFrame0(void* clientData) {
	((FECMultiplexor*)clientData)->deliverFrame();
}
