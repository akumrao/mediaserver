/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
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

            virtual void cbDnsResolve(addrinfo* res, std::string ip) {
                LTrace("GetAddrInfoReq::cbDnsResolve");
            }

    
            static void on_resolved(uv_getaddrinfo_t* handle, int status, struct addrinfo* res) {
                struct getaddrinfo_req* req;
                
                GetAddrInfoReq *obj = (GetAddrInfoReq*) handle->data;
                
                if (status < 0 || !res) {
                LTrace(  "getaddrinfo callback error ", uv_err_name(status));
                obj->cbDnsResolve(nullptr, "");
                return;
            }
                
                char addr[17] = {'\0'};
                uv_ip4_name((struct sockaddr_in*) res->ai_addr, addr, 16);
                LTrace("address ",  addr);
                
                // uv_tcp_connect(connect_req, socket, (const struct sockaddr*) res->ai_addr, on_connect);


          
                obj->cbDnsResolve(res, addr);

                uv_freeaddrinfo(res);

            }

            void resolve(const std::string& host, int port, uv_loop_t * loop = Application::uvGetLoop()) {

                req.data = this;
                int r;

                struct addrinfo hints;
                //hints.ai_family = PF_INET;
                //hints.ai_socktype = SOCK_STREAM;
                //hints.ai_protocol = IPPROTO_TCP;
               // hints.ai_flags = 0;

                r = uv_getaddrinfo(loop,
                        &req,
                        on_resolved,
                        host.c_str(),
                        util::itostr(port).c_str(),
                                   nullptr);
                assert(r == 0);
                
                
                
            }

            uv_getaddrinfo_t req;
        };

    } // namespace net
} // base


#endif /* DNS_H */

