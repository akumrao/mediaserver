/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#ifndef UDP_UPLOAD_H
#define UDP_UPLOAD_H

// #define UdpDataSize 70 for textfile
#define UdpDataSize 1024 
//#define UdpDataSize 32*1024

#define clientCount 100*1024
#define serverCount 512*1024

//type 0 for first upd packet. It contains file name
//type 1 for contains file data in chunks



struct Packet{
    uint8_t type;
    uint32_t payload_number;
    uint32_t payloadlen;
    char payload[UdpDataSize];
};


#endif  //UDP_UPLOAD_H
