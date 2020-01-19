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

Boolean RTPPacket::extension() {
    return EXTRACT_BIT(4, fContent[0]) == 1 ? True : False;
}

Boolean RTPPacket::marker() {
    return EXTRACT_BIT(0, fContent[1]) == 1 ? True : False;
}

u_int16_t RTPPacket::sequenceNumber() {
    return (((u_int16_t)fContent[2]) << 8) | fContent[3];
}

u_int32_t RTPPacket::timestamp() {
    return
        (((u_int32_t)fContent[4]) << 24) |
        (((u_int32_t)fContent[5]) << 16) |
        (((u_int32_t)fContent[6]) << 8) |
        ((u_int32_t)fContent[7]);
}

u_int32_t RTPPacket::ssrcIdentifier() {
    return
        (((u_int32_t)fContent[8]) << 24) |
        (((u_int32_t)fContent[9]) << 16) |
        (((u_int32_t)fContent[10]) << 8) |
        ((u_int32_t)fContent[11]);
}
