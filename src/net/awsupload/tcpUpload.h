/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#ifndef TCP_UPLOAD_H
#define TCP_UPLOAD_H



//type 0 for init. Request Port no.
//type 1 for init. Server Return UDP Port no.

//type 2 for retransmission. from UDP server to TCP Client

//type 3 Percentage uploaded. From UDP server to TCP client

// type 4 HeaderNot received yet. From UDP server to TCP client

struct TcpPacket{
    uint8_t type;
    uint32_t sequence_number;
  };

  
#endif  //TCP_UPLOAD_H
