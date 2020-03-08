#ifndef UDP_UPLOAD_H
#define UDP_UPLOAD_H


#define UdpDataSize 32768
#define clientCount 100*1024
#define serverCount 512*1024

struct Packet{
    uint8_t type;
    uint16_t payload_number;
    uint16_t payloadlen;
    char payload[UdpDataSize];
};


#endif  //UDP_UPLOAD_H