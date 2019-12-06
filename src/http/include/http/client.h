

#ifndef HTTP_Client_H
#define HTTP_Client_H


#include "http/parser.h"
#include "http/url.h"
#include "base/logger.h"
#include "net/dns.h"
#include <functional> 
#include "http/HttpConn.h"

namespace base {
    namespace net {

        class ClientConnection: public HttpConnection, public GetAddrInfoReq  {
        public:

        public:
            ClientConnection(Listener* listener, const URL& url, http_parser_type type = HTTP_RESPONSE, size_t bufferSize = 65536);

            ~ClientConnection() override;
            void connect();

        public:
            virtual void send(const char* data, size_t len);
            virtual void send();
            virtual void send(Request& req);
            virtual void send(const std::string &str);
            
            void on_connect();
            void on_close();

            virtual void cbDnsResolve(addrinfo* res,std::string ip);

            /* Pure virtual methods inherited from ::TcpHTTPConnection. */
        public:
            void on_read(const char* data, size_t len);

          /*  /// HTTP Parser interface
            virtual void onParserHeader(const std::string& name, const std::string& value);
            virtual void onParserHeadersEnd(bool upgrade);
            virtual void onParserChunk(const char* buf, size_t len);
            virtual void onParserError(const base::Error& err);
            virtual void onParserEnd();
         */
            /// HTTP connection and server interface
            virtual void onHeaders();
            virtual void on_payload(const uint8_t* data, size_t len);
            virtual void onComplete();
            std::function<void(const std::string&)> fnPayload; ///< Signals when raw data is received
            std::function<void(const Response&)> fnComplete;     ///< Signals when the HTTP transaction is complete
            std::function<void(ClientConnection&)> fnClose;          
          
       
       
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



        class Client:public Listener  {
        public:
            Client();
            void createConnection(const std::string& protocol, const std::string &ip, int port,  const std::string& query);
            Client(URL url);
            ~Client();

            void start();

            void shutdown();

            void on_close(ClientConnection* connection);

            void on_read(ClientConnection* connection, const uint8_t* data, size_t len);

            
            ClientConnection *clientConn;

        protected:
            
            URL _url;
          
        };
    } // namespace net
} // base

#endif

