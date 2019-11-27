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

namespace base {

    namespace net {

        struct GetAddrInfoReq {

            virtual void cbDnsResolve(addrinfo* res) {
                LTrace("GetAddrInfoReq::cbDnsResolve");
            }

            static void getaddrinfo_cb(uv_getaddrinfo_t* handle, int status, struct addrinfo* res) {
                struct getaddrinfo_req* req;

                assert(status == 0);

                GetAddrInfoReq *obj = (GetAddrInfoReq*) handle->data;
                obj->cbDnsResolve(res);

                uv_freeaddrinfo(res);

            }

            void resolve(const std::string& host, int port, uv_loop_t * loop = uv_default_loop()) {

                req.data = this;
                int r;

                r = uv_getaddrinfo(loop,
                        &req,
                        getaddrinfo_cb,
                        host.c_str(),
                        util::itostr(port).c_str(),
                        NULL);
                assert(r == 0);
            }

            uv_getaddrinfo_t req;
        };

    } // namespace net
} // base


#endif /* DNS_H */

