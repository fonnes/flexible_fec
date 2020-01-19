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

Boolean RTPPacket::padding() {
    return EXTRACT_BIT(5, fContent[0]) == 1 ? True : False;
}

u_int16_t RTPPacket::sequenceNumber() {
    return (((u_int16_t)fContent[2]) << 8) | fContent[3];
}
