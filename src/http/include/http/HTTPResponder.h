/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Responder.h
 * Author: root
 *
 * Created on July 7, 2021, 11:35 AM
 */

#include "http/HttpServer.h"
#include "base/logger.h"

#ifndef HTTP_RESPONDER_H
#define HTTP_RESPONDER_H

namespace base {
    namespace net {


        class BasicResponder : public ServerResponder
        /// Basic server responder (make echo?)
        {
        public:

            BasicResponder(net::HttpBase* conn) :
            ServerResponder(conn) {
                STrace << "BasicResponder" << std::endl;
            }

            virtual void onClose() {
                ;
                LDebug("On close")

            }
            
            void sendResponse(std::string &result, bool success);
            

            void onRequest(net::Request& request, net::Response& response) ;
        };

        class render_baton;
        class HttpResponder : public ServerResponder
        /// Basic server responder (make echo?)
        {
        public:

            HttpResponder(net::HttpBase* conn) ;
            
            ~HttpResponder();

            virtual void onClose(); 
            
            render_baton *closure {nullptr};

            void onRequest(net::Request& request, net::Response& response);
        };

        class StreamingResponderFactory : public ServerConnectionFactory {
        public:

            ServerResponder* createResponder(HttpBase* conn) {

                auto& request = conn->_request;

                // Log incoming requests
                STrace << "Incoming connection from " << ": Request:\n" << request << std::endl;

                SDebug << "Incoming connection from: " << request.getHost() << " method: " << request.getMethod() << " uri: <<  " << request.getURI() << std::endl;

                // Handle websocket connections
                if (request.getMethod() == "GET") { // || request.has("Sec-WebSocket-Key")) {
                    return new HttpResponder(conn);
                } else {
                    return new BasicResponder(conn);
                }


            }
        };


    }
}
#endif /* RESPONDER_H */

