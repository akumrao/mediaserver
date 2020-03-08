#ifndef TCP_UPLOAD_H
#define TCP_UPLOAD_H



//type 0 for init. Request Port no.
//type 1 for init. Server Return UDP Port no.

//type 2 for retransmission. from UDP server to TCP Client

struct TcpPacket{
    uint8_t type;
    uint16_t sequence_number;
  };

  
#endif  //TCP_UPLOAD_H
