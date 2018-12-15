#include "include/FECCluster.hh"
#include <iostream>

FECCluster* FECCluster::createNew(u_int16_t base, u_int8_t row, u_int8_t column) {
    return new FECCluster(base, row, column);
}

FECCluster::FECCluster(u_int16_t base, u_int8_t row, u_int8_t column) {
    fBase = base;
    fRow = row;
    fColumn = column;

    int size = (row + 1) * (column + 1) - 1;
    fRTPPackets = new RTPPacket*[size];
    for (int i = 0; i < size; i++) fRTPPackets[i] = NULL;

    struct timeval tp;
	gettimeofday(&tp, NULL);
	fTimestamp = (long long) tp.tv_sec * 1000L + tp.tv_usec / 1000;
}

void FECCluster::insertPacket(RTPPacket* rtpPacket) {
    int index = getIndex(rtpPacket);
    fRTPPackets[index] = rtpPacket;
}

int FECCluster::getIndex(RTPPacket* rtpPacket) {
	int payload = EXTRACT_BIT_RANGE(0, 7, rtpPacket->content()[1]);
	if (payload == 115) {
		u_int16_t base = (((u_int16_t)rtpPacket->content()[20]) << 8) | rtpPacket->content()[21];
        u_int16_t prelimIndex = base - fBase;
		return prelimIndex / fColumn * (fColumn + 1) + fColumn;
	}
	else if (payload == 116) {
		u_int16_t columnBase = (((u_int16_t)rtpPacket->content()[20]) << 8) | rtpPacket->content()[21];
        u_int16_t prelimIndex = columnBase - fBase;
		return prelimIndex + fRow * (fColumn + 1);
	}
	else {
		u_int16_t seqNum = (((u_int16_t)rtpPacket->content()[2]) << 8) | rtpPacket->content()[3];
		u_int16_t prelimIndex = seqNum - fBase;
		return prelimIndex + prelimIndex / fColumn;
	}
}

Boolean FECCluster::seqNumInCluster(u_int16_t seqNum) {
    u_int16_t diff = seqNum - fBase;
    return diff < sourcePacketCount();
}

Boolean FECCluster::allRTPPacketsArePresent() {
    int size = sourcePacketCount();
	for (int i = 0; i < size; i++) {
		int index = i + i / fColumn;
		if (fRTPPackets[index] == NULL) return False;
	}
	return True;
}

Boolean FECCluster::hasExpired(long long repairWindow) {
    struct timeval tp;
    gettimeofday(&tp, NULL);
    long long now = (long long) tp.tv_sec * 1000L + tp.tv_usec / 1000;

    return now > (fTimestamp + repairWindow);
}

Boolean FECCluster::hasOnlyNullPackets() {
    int size = sourcePacketCount();
	for (int i = 0; i < size; i++) {
		if (fRTPPackets[i] != NULL) return False;
	}
	return True;
}
