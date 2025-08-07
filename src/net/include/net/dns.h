/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */


/* 
 * File:   DNS.h
 * Author: root
 *
 * Created on November 26, 2019, 2:29 PM
 */

#ifndef DNS_H
#define DNS_H

#include "uv.h"
#include <string>
#include "base/util.h"
#include "base/application.h"

namespace base {

    namespace net {

        struct GetAddrInfoReq {

//            virtual void cbDnsResolve(addrinfo* res, std::string ip, int port,  void* ptr) {
//               // LTrace("GetAddrInfoReq::cbDnsResolve");
//            }
            
            
            virtual void cbDnsResolve(addrinfo* res) {
               // LTrace("GetAddrInfoReq::cbDnsResolve");
            }

    
            static void on_resolved(uv_getaddrinfo_t* handle, int status, struct addrinfo* res) ;

            void resolve(const std::string& host, int port, uv_loop_t * loop = Application::uvGetLoop(), void* ptr=nullptr) ;

            uv_getaddrinfo_t *req;
            
             //void* clsPtr{nullptr}; 

        };
        
        
        struct GetNameInfoReq {

            virtual void cbNameResolve(  const char* hostname, const char* service,  void* data) {
               // LTrace("GetAddrInfoReq::cbDnsResolve");
            }

            static void on_resolvedIP(uv_getnameinfo_t* handle, int status, const char* hostname, const char* service); 

            void resolveIP(sockaddr_storage &addrStorage,  uv_loop_t * loop, void* ptr=nullptr) ;

            uv_getnameinfo_t *req{nullptr};
            
            struct stTmp
            {
                void* clsPtr{nullptr}; 
                void* data{nullptr}; 
            };

        };
        
        

    } // namespace net
} // base


#endif /* DNS_H */

