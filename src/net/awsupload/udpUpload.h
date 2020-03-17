#ifndef UDP_UPLOAD_H
#define UDP_UPLOAD_H

// #define UdpDataSize 70 for textfile
 
#define UdpDataSize 32*1024
#define clientCount 100*1024
#define serverCount 512*1024

//type 0 for first upd packet. It contains file name
//type 1 for contains file data in chunks



struct Packet{
    uint8_t type;
    uint16_t payload_number;
    uint16_t payloadlen;
    char payload[UdpDataSize];
};


#endif  //UDP_UPLOAD_H