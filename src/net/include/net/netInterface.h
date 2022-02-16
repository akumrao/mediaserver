/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#ifndef NET_INTERFACE_LIST_H
#define NET_INTERFACE_LIST_H
#include <stddef.h>
#include <string>
namespace base
{
    namespace net
    {

        class Listener
            {
            public:
                Listener(){}
                virtual void on_read( Listener* conn, const char* data, size_t len){}
                virtual void on_connect(Listener* conn) { }
                virtual void on_close(Listener* conn){}
                
                virtual void on_wsread( Listener* conn, const char* data, size_t len){}
                virtual void on_wsconnect(Listener* conn) { }
                virtual void on_wsclose(Listener* conn){}
                
                virtual void send(const char* data, size_t len){}

                virtual const std::string& GetLocalIp() const{ return ip_port;}
                virtual const std::string& GetPeerIp() const{return ip_port;}
                
                virtual uint16_t GetLocalPort() const { return 0;}
                virtual uint16_t GetPeerPort() const {return 0;}
                
                      /////////////////////////////////////////
                virtual void on_header(Listener* conn) { }
        
                std::string ip_port="please overide it";

                
            };
            
            
                 
    } // namespace net
} // namespace base


#endif  //NET_INTERFACE_H
