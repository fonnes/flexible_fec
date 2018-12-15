#include "include/RTPPacket.hh"

//TODO: Handle extension header?
RTPPacket* RTPPacket::createNew(unsigned char* rtpPacket, unsigned rtpPacketSize) {
    return new RTPPacket(rtpPacket, rtpPacketSize);
}

RTPPacket::RTPPacket(unsigned char* content, unsigned size) {
    fContent = new unsigned char[size];
    for (int i = 0; i < size; i++) {
        fContent[i] = content[i];
    }
    fSize = size;
}

RTPPacket::~RTPPacket() {
    delete[] fContent;
}
