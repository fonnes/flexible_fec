#ifndef _RTP_PACKET_HH
#define _RTP_PACKET_HH

#define EXTRACT_BIT(n, x) ((x >> n) & 0x01)
#define EXTRACT_BIT_RANGE(from, to, data) ((typeof(data))(data << (sizeof(data) * 8 - to)) >> (from + (sizeof(data) * 8 - to)))

#define RTP_HEADER_SIZE 12
#define EXTENSION_HEADER_ID_SIZE 2

#include <liveMedia.hh>

class RTPPacket {
public:
    static RTPPacket* createNew(unsigned char* rtpPacket, unsigned rtpPacketSize);
    RTPPacket(unsigned char* content, unsigned size);
    virtual ~RTPPacket();

    // Version
    Boolean padding();
    Boolean extension();
    // CC
    Boolean marker();
    // PT
    u_int16_t sequenceNumber();
    // Timestamp
    // SSRC identifier
    // CSRC identifiers



    // TODO: Remove
    unsigned char* content() const { return fContent; }
    unsigned size() const { return fSize; }

private:
    unsigned char* fContent;
    unsigned fSize;
};
#endif
