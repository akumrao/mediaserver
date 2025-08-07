
#include "net/dns.h"

#include "base/logger.h"

namespace base {

    namespace net {

      
               void GetAddrInfoReq::on_resolved(uv_getaddrinfo_t* handle, int status, struct addrinfo* res) {
                //struct getaddrinfo_req* req;
                
                GetAddrInfoReq *obj = (GetAddrInfoReq*) handle->data;
               
                if (status < 0 || !res) {
                    LError(  "getaddrinfo callback error ", uv_err_name(status));
                    
                    if(res)
                    uv_freeaddrinfo(res);
                    delete handle;
                    
                    //obj->cbDnsResolve(nullptr, "",0);
                    return;
                }
                
                char addr[40] = {'\0'};
                int port =0; 

                struct addrinfo* start =  res;
                
//                for (;res != NULL; res = res->ai_next) 
//                { 
//                    
//                    if (res->ai_family == AF_INET) {
//                        // ipv4
//                        //char c[17] = { '\0' };
//                        
//                        sockaddr_in* tmp  =   (sockaddr_in*) res->ai_addr;
//                        port= htons(tmp->sin_port);
//                        uv_ip4_name(tmp, addr, 16);
//                        
//        
//                        
//                    } else if (res->ai_family == AF_INET6) {
//                        // ipv6
//                        //char c[40] = { '\0' };
//                        sockaddr_in6* tmp  =   (sockaddr_in6*) res->ai_addr;
//                        port= htons(tmp->sin6_port);
//                        uv_ip6_name(tmp, addr, 39);
//                    }
//                    LTrace("address ",  addr);
//                    // uv_tcp_connect(connect_req, socket, (const struct sockaddr*) res->ai_addr, on_connect);
//
//                    obj->cbDnsResolve(res, addr, port, obj->clsPtr);
//                }
                
                obj->cbDnsResolve(start);
                
                uv_freeaddrinfo(start);
                
                delete handle;

            }

            void GetAddrInfoReq::resolve(const std::string& host, int port, uv_loop_t * loop, void* ptr) {

                req = new uv_getaddrinfo_t; 
                //this->clsPtr =ptr;
                req->data = this;
                int r;

                struct addrinfo hints;
//                hints.ai_family = PF_INET;
//                hints.ai_socktype = SOCK_STREAM;
//                hints.ai_protocol = IPPROTO_TCP;
//                hints.ai_flags = 0;
                hints.ai_family = AF_UNSPEC;
                hints.ai_socktype = SOCK_DGRAM;
                hints.ai_protocol = IPPROTO_UDP;
                hints.ai_flags = AI_ADDRCONFIG;

                r = uv_getaddrinfo(loop,
                        req,
                        on_resolved,
                        host.c_str(),
                        util::itostr(port).c_str(),
                                  &hints);
                assert(r == 0);

            }
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            // resolve ip to hostname and service number
            void  GetNameInfoReq::on_resolvedIP(uv_getnameinfo_t* handle, int status, const char* hostname, const char* service) 
            {
                  stTmp *tmp       =(stTmp * )handle->data;
                 
                GetNameInfoReq *obj = (GetNameInfoReq*) tmp->clsPtr;
               
                if (status < 0 ) {
                    LError(  "getnameinfo callback error ", uv_err_name(status));
                    delete tmp;
                    delete handle;
                    //obj->cbDnsResolve(nullptr, "",0);
                    return;
                }
                    STrace << "Reolved Hostname:" <<  hostname << " cls obj ptr " << tmp->data;
                    STrace << "Resolved Service:" <<  service;
                    obj->cbNameResolve(hostname, service, tmp->data);
                
                    delete tmp;
                    
                    delete handle;
            }
                
              void GetNameInfoReq::resolveIP(sockaddr_storage &addrStorage,  uv_loop_t * loop, void* ptr) 
              {
                req = new uv_getnameinfo_t; 
                
                stTmp *tmp = new stTmp;
                tmp->clsPtr = this ;
                tmp->data = ptr;
                
                req->data = tmp;
                int r;

                r = uv_getnameinfo(loop, req, on_resolvedIP, (const struct sockaddr*)&addrStorage,  NI_NUMERICHOST | NI_NUMERICSERV | NI_DGRAM); 
                assert(r == 0);

            }
            
        
           

    } // namespace net
} // base

