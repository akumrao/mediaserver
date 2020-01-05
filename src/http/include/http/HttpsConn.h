/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   HTTPConnection.h
 * Author: root
 *
 * Created on November 18, 2019, 9:42 AM
 */

#ifndef HTTPSCONNECTION_H
#define HTTPSCONNECTION_H

//#include "net/TcpConnection.h"

#include "net/SslConnection.h"
#include "http/parser.h"


namespace base {
    namespace net {

        class ServerResponder;
        class WebSocketConnection;

        class HttpsConnection : public SslConnection, public HttpBase {
        public:

        public:
            HttpsConnection(Listener* listener, http_parser_type type);
            ~HttpsConnection() override;

        public:
            void send(const char* data, size_t len);
            void Close();

            /* Pure virtual methods inherited from ::HttpsConnection. */
        public:
            void on_read(const char* data, size_t len) override;
            void on_close() override;
/*
            /// HTTP Parser interface
            virtual void onParserHeader(const std::string& name, const std::string& value);
            virtual void onParserHeadersEnd(bool upgrade);
            virtual void onParserChunk(const char* buf, size_t len);
            virtual void onParserError(const base::Error& err);
            virtual void onParserEnd();
*/
            /// HTTP connection and server interface
            virtual void onHeaders();
            virtual void on_payload(const char* data, size_t len);
            virtual void onComplete();
            // virtual void onClose() ;

         //   Message* incomingHeader();
         //   Message* outgoingHeader();

            /// Send the outdoing HTTP header.
            virtual long sendHeader();

            WebSocketConnection *wsAdapter{ nullptr};

        private:
            // Passed by argument.
            Listener* listener{ nullptr};



        public:

            ServerResponder* _responder{nullptr};
        protected:
           
        

        };




    } // namespace net
} // base


#endif /* HTTPSCONNECTION_H */

