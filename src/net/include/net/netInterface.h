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
                Listener(){};
                virtual void on_read( Listener* conn, const char* data, size_t len){};
                virtual void on_connect(Listener* conn) { };
                virtual void on_close(Listener* conn){};
                virtual void send(const char* data, size_t len){};

                virtual const std::string& GetLocalIp() const{ return ip_port;}
                virtual const std::string& GetPeerIp() const{return ip_port;}
                
                      /////////////////////////////////////////
                virtual void on_header(Listener* conn) { };
        
                std::string ip_port="please overide it";

                
            };
            
            
                 
    } // namespace net
} // namespace base


#endif  //NET_INTERFACE_H
