

#ifndef HTTP_Client_H
#define HTTP_Client_H


#include "http/parser.h"
#include "http/url.h"
#include "base/logger.h"
#include "net/dns.h"
#include <functional> 
//#include "http/HttpsConn.h"
#include "http/HttpConn.h"
namespace base {
    namespace net {
            // HTTP progress signal for upload and download progress notifications.
        class  ProgressSignal 
        {
        public:
            void* sender;
            uint64_t current;
            uint64_t total;

            ProgressSignal()
                : sender(nullptr)
                , current(0)
                , total(0)
            {
            }

            double progress() const { return (current / (total * 1.0)) * 100; }

            void update(int nread)
            {
                assert(current <= total);
                current += nread;
//                emit(progress());
            }
        };

        
        class ClientConnecton : public HttpBase {
          public:
           ClientConnecton(http_parser_type type);
            
            virtual ~ClientConnecton() ;

             

            std::function<void(const std::string&) > fnLoad; ///< Signals when raw data is received
            std::function<void(const Response&) > fnComplete; ///< Signals when the HTTP transaction is complete
            std::function<void(ClientConnecton&) > fnClose;
            std::function<void(ClientConnecton*) > fnConnect;
            

            virtual void setReadStream(std::ostream* os) {
            };

       
            
            virtual void send(const char* data, size_t len){};
            virtual void send(){};
            virtual void send(Request& req){};
            virtual void send(const std::string &str){};
            

            void Close() {
            };

            virtual void onHeaders() {
            };
            virtual void on_payload(const char* data, size_t len);

            virtual void onComplete() {
            };
    
            
            ProgressSignal IncomingProgress; ///< Fired on download progress
            ProgressSignal OutgoingProgress; ///< Fired on upload progress
            
        };

        class HttpClient : public TcpConnection, public GetAddrInfoReq, public ClientConnecton {
        public:

        public:
            HttpClient(Listener* listener, const URL& url, http_parser_type type = HTTP_RESPONSE, size_t bufferSize = 65536);

            ~HttpClient() override;
            void connect();

        public:
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

            int64_t start_time;
            int64_t end_time;

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
            //bool bClosing{false};
           // WebSocketConnection *wsAdapter{ nullptr};
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

        class Client : public Listener {
        public:
            Client();
            void createConnection(const std::string& protocol, const std::string &ip, int port, const std::string& query);
            Client(URL url);
            virtual ~Client();

            void start();

            void shutdown();

            void on_close(Listener* connection){};

            void on_read(Listener* connection, const uint8_t* data, size_t len);


            ClientConnecton *clientConn;

        protected:

            URL _url;
       };
    } // namespace net
} // base

#endif

