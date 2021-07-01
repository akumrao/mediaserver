/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */



#ifndef HTTP_Client_H
#define HTTP_Client_H


#include "http/client.h"
#include "http/url.h"
#include "net/dns.h"
//#include "http/HttpsConn.h"
#include "http/HttpConn.h"
namespace base {
    namespace net {
        

        class HttpClient : public TcpConnectionBase, public GetAddrInfoReq, public ClientConnecton {
        public:

            HttpClient(const std::string& protocol, const std::string &ip, int port, const std::string& query);
            HttpClient(URL url);
            HttpClient(Listener* listener, const URL& url, http_parser_type type = HTTP_RESPONSE);
            ~HttpClient() override;
        private:
           
            void Int();
           
            void connect();

        public:
            void tcpsend(const char* data, size_t len) override;
            void send(const char* data, size_t len) override;
            void send() override;
            void send(Request& req) override;
            void send(const std::string &str) override;
            void Close() override;
            

            void on_connect() override;
            void on_close() override;

            virtual void cbDnsResolve(addrinfo* res, std::string ip) override;

            /* Pure virtual methods inherited from ::TcpHTTPConnection. */
        public:
            void on_read(const char* data, size_t len) override;

            /*  /// HTTP Parser interface
              virtual void onParserHeader(const std::string& name, const std::string& value);
              virtual void onParserHeadersEnd(bool upgrade);
              virtual void onParserChunk(const char* buf, size_t len);
              virtual void onParserError(const base::Error& err);
              virtual void onParserEnd();
             */
            /// HTTP connection and server interface
            virtual void onHeaders() override;
            void on_payload(const char* data, size_t len) override;
            virtual void onComplete() override;
  

            //   Message* incomingHeader();
            //   Message* outgoingHeader();

            /// Send the outdoing HTTP header.
            virtual long sendHeader();

            /// Return true if headers should be automatically sent.
            bool shouldSendHeader() const;

            /// Set true to prevent auto-sending HTTP headers.
            void shouldSendHeader(bool flag);

            void setReadStream(std::ostream* os) override;

        private:
            // Passed by argument.
            Listener* listener{ nullptr};
            WebSocketConnection *wsAdapter{ nullptr};

        public:
            // Request _request;
            // Response _response;
            //Parser _parser;

            Message* incomingHeader() override;
            Message* outgoingHeader() override;


            URL _url;
            bool _connect;
            bool _active;
            bool _complete;
            std::vector<std::string> _outgoingBuffer;
            std::unique_ptr<std::ostream> _readStream;
            
           std::stringstream* readStream() override
           {
		      return (std::stringstream*)_readStream.get();
           }


        };

        /********************************************************************************************************************/
        /*       class HttpsClient : public SslConnection, public GetAddrInfoReq, public ClientConnecton {
               public:

               public:
                   HttpsClient(Listener* listener, const URL& url, http_parser_type type = HTTP_RESPONSE, size_t bufferSize = 65536);

                   ~HttpsClient() override;
                   void connect();
                    void Close();

               public:
                   virtual void send(const char* data, size_t len);
                   virtual void send();
                   virtual void send(Request& req);
                   virtual void send(const std::string &str);

                   void on_connect();
                   void on_close();

                   virtual void cbDnsResolve(addrinfo* res, std::string ip);


        public:
            void on_read(const char* data, size_t len);

            int64_t start_time;
            int64_t end_time;


            /// HTTP connection and server interface
            virtual void onHeaders();
            void on_payload(const char* data, size_t len);
            virtual void onComplete();
      
            //   Message* incomingHeader();
            //   Message* outgoingHeader();

            /// Send the outdoing HTTP header.
            virtual long sendHeader();

            /// Return true if headers should be automatically sent.
            bool shouldSendHeader() const;

            /// Set true to prevent auto-sending HTTP headers.
            void shouldSendHeader(bool flag);

            void setReadStream(std::ostream* os);

        private:
            // Passed by argument.
            Listener* listener{ nullptr};
            WebSocketConnection *wsAdapter{ nullptr};
            //  WebSocketConnection::Listener *wsListener{ nullptr};
            // Others.
            // size_t frameStart{ 0}; // Where the latest frame starts.
            //  size_t recvBytes{ 0};
            //  size_t sentBytes{ 0};

        public:
            // Request _request;
            // Response _response;
            //Parser _parser;

            Message* incomingHeader();
            Message* outgoingHeader();


            URL _url;
            bool _connect;
            bool _active;
            bool _complete;
            std::vector<std::string> _outgoingBuffer;
            std::unique_ptr<std::ostream> _readStream;


        };
*/
        /*********************************************************************************************************************/

      
    } // namespace net
} // base

#endif

