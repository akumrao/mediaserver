

#ifndef HTTPS_Client_H
#define HTTPS_Client_H


#include "http/client.h"
#include "http/url.h"
#include "net/dns.h"
#include "net/SslConnection.h"
#include "http/websocket.h"
namespace base {
    namespace net {
        

        class HttpsClient : public SslConnection, public GetAddrInfoReq, public ClientConnecton{
        public:

            HttpsClient(const std::string& protocol, const std::string &ip, int port, const std::string& query);
            HttpsClient(URL url);
            HttpsClient(Listener* listener, const URL& url, http_parser_type type = HTTP_RESPONSE);
            ~HttpsClient() override;
        private:
           
            void Int();
           
            void connect();

        public:
            void tcpsend(const char* data, size_t len);
            void send(const char* data, size_t len);
            void send();
            void send(Request& req);
            void send(const std::string &str);
            void Close();

            void on_connect();
            void on_close();

            virtual void cbDnsResolve(addrinfo* res, std::string ip);

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

