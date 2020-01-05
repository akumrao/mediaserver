
#include "net/netInterface.h"
//#include "http/websocket.h"
#include "http/client.h"
#include "net/IP.h"
#include "base/util.h"
#include "base/application.h"
#include "base/platform.h"
#include "http/websocket.h"
using std::endl;


namespace base {
    namespace net {

        ProgressSignal::ProgressSignal()
        : //sender(nullptr)
         current(0)
        , total(0)
        , latency(0)
        , totalTimeDiff(0) {
            LTrace("ProgressSignal");
        }

        void ProgressSignal::start() {
            start_time = base::Application::GetTime();
        }
           
        void ProgressSignal::update(int nread, ClientConnecton* conn) {
            assert(current <= total);
            current += nread;
            //emit(progress());

            double timeDiff = (double) ((base::Application::GetTime()) - end_time);
            latency = (latency + timeDiff) / 2;
            end_time = base::Application::GetTime();

            LTrace("cur ", current, "total ", total)
                    
            totalTimeDiff += timeDiff;
            if (current >= total) {
                RTrace("\"totalsize\":", double( current / 1000), ", \"time_s\":", double(end_time - start_time) / 1000.00);
            }

            if (totalTimeDiff >= 1000 || current >= total || ((int( progress()) % 10 == 0) && (totalTimeDiff > 10))) {
                totalTimeDiff = 0;
                RTrace("\"Speed_MBS\":", double( current  / ((end_time - start_time)*1000.00)), ", \"latency_ms\":", latency)

                std::ostringstream ss;
                if( conn->fnFormClose) //upload speed
                    ss << "{UplaodSpeed_MBS:" << double( current / ((end_time - start_time)*1000.00)) << ", latency_ms:" << latency << "}";
                else
                    ss << "{DownloadSpeed_MBS:" << double( current / ((end_time - start_time)*1000.00)) << ", latency_ms:" << latency << "}";

                if(conn->fnUpdateProgess)
                conn->fnUpdateProgess(ss.str());

            }

        }

        //
        // Client Connection
        //

        ClientConnecton::ClientConnecton(http_parser_type type) :  HttpBase(type) {
        }

        ClientConnecton::~ClientConnecton() {
        }

        void ClientConnecton::on_payload(const char* data, size_t len) {

            LTrace("ClientConnecton::on_payload")
        };

        HttpClient::HttpClient(Listener* listener, const URL& url, http_parser_type type)
        : TcpConnection(listener)
        , ClientConnecton(type)
        , listener(listener)
        , _url(url)
        , _connect(false)
        , _active(false)
        , _complete(false) {

            auto uri = url.pathEtc();
            if (!uri.empty())
                _request.setURI(uri);
            _request.setHost(url.host(), url.port());

            // Set default error status
            _response.setStatus(StatusCode::BadGateway);


            _parser.setObserver(this);
            if (type == HTTP_REQUEST)
                _parser.setRequest(&_request);
            else
                _parser.setResponse(&_response);

            // replaceAdapter(new ConnectionAdapter(this, HTTP_RESPONSE));

            if (url.scheme() == "ws") {

                //  conn->replaceAdapter(new ws::ConnectionAdapter(conn.get(), ws::ClientSide));
                wsAdapter = new WebSocketConnection(listener, this, ClientSide);
            }

        }

        HttpClient::~HttpClient() {
            LTrace("~HttpClient()")
        }

        void HttpClient::send() {
            connect();
        }

        void HttpClient::send(Request& req) {
            assert(!_connect);
            _request = req;
            connect();
        }

        void HttpClient::Close() {
            _connect = true;
            _active = true;
            TcpConnection::Close();
        }

         void HttpClient::tcpsend(const char* data, size_t len) {
    
             TcpConnection::send(data, len);
         }
         
        void HttpClient::send(const char* data, size_t len) {
            connect();
            
            if(wsAdapter)
            {
                wsAdapter->send(data,len );
                return;
            }

            if (_active)
                // Raw data will be pushed onto the Outgoing packet stream
                TcpConnection::send(data, len);
            else
                _outgoingBuffer.push_back(std::string((char*) data, len));
            return;
        }

        void HttpClient::send(const std::string &str) {
            send(str.c_str(), str.length());
        }

        void HttpClient::cbDnsResolve(addrinfo* res, std::string ip) {
            if (_connect) return;

            if (!_connect) {
                _connect = true;

                LTrace("Connecting ", ip, ":", _url.port())
                Connect(_url.host(), _url.port(), res);
            }

        }

        void HttpClient::connect() {
            if (_connect) return;
            LTrace("Resolve DNS ", _url.host());

            resolve(_url.host(), _url.port(), Application::uvGetLoop());
        }

        void HttpClient::setReadStream(std::ostream* os) {
            assert(!_connect);
            //Incoming.attach(new StreamWriter(os), -1, true);
            _readStream.reset(os);
        }


        //
        // Socket Callbacks

        void HttpClient::on_connect() {
            if (_active) return;
            LTrace("On_connect")

            // Set the connection to active

            if(wsAdapter)
            {
                wsAdapter->onSocketConnect();
                
                return;
            }
            else
            {
                sendHeader();
            }
            _active = true;

            // Emit the connect signal so raw connections like
            // websockets can kick off the data flow
            //   Connect.emit();

            // Flush queued packets
            if (!_outgoingBuffer.empty()) {
                // LTrace("Sending buffered: ", _outgoingBuffer.size())
                for (const auto& packet : _outgoingBuffer) {
                    TcpConnection::send((const char*) packet.c_str(), packet.length());
                }
                _outgoingBuffer.clear();
            } else {

                // sendHeader();
                // Send the header

            }

            // Send the outgoing HTTP header if it hasn't already been sent.
            // Note the first call to socket().send() will flush headers.
            // Note if there are stream adapters we wait for the stream to push
            // through any custom headers. See ChunkedAdapter::emitHeader
            //if (Outgoing.numAdapters() == 0) {
            //    // LTrace("On connect: Send header")
            //    sendHeader();
            //}
            if (fnConnect)
                fnConnect(this);
        }

        void HttpClient::on_read(const char* data, size_t len) {

            // LTrace("On socket recv: ", len);
             //LTrace("On socket recv: ", data);
            recvBytes += len;
            
            if(wsAdapter)
            {
                wsAdapter->onSocketRecv( std::string((char*)data, len));
                return;
            }
            // TcpConnection::on_read( data, len);
            _parser.parse((const char*) data, len);
            
            
            
            //LInfo("On socket recv: ", len , " : " , recvBytes, " : ", end_time- start_time );

        }
        // Connection Callbacks

        void HttpClient::onHeaders() {
            LTrace("On headers")
                    OutgoingProgress.total = _response.getContentLength();
        }

        void HttpClient::on_payload(const char* data, size_t len) {
            LTrace("HttpClient On payload: ", data)

                    //// Update download progress
                    //IncomingProgress.update(buffer.size());

                    //// Write to the incoming packet stream if adapters are attached
                    //if (Incoming.numAdapters() > 0 || Incoming.emitter.nslots() > 0) {
                    //    // if (!Incoming.active());
                    //    //     throw std::runtime_error("startInputStream() must be called");
                    //    Incoming.write(bufferCast<const char*>(buffer), buffer.size());
                    //}

                    // Write to the STL read stream if available
            if (_readStream) {
               // LTrace("Stream len: ", len)
                //LTrace("Stream data: ", data)
                _readStream->write((const char*) data, len);
                _readStream->flush();
                //Close();
            }
            
            if(fnPayload)
             fnPayload(this,data, len);
        }

        void HttpClient::onComplete() {
            LTrace("On complete")

            assert(!_complete);


            // Release any file handles
            if (_readStream) {
                auto fstream = dynamic_cast<std::ofstream*> (_readStream.get());
                if (fstream) {
                    // LTrace("Closing file stream")
                    fstream->close();
                }
            }
            _complete = true; // in case close() is called inside callback

            if(fnComplete)
            fnComplete(_response);
        }

        void HttpClient::on_close() {
            LTrace("On close")

            if (!_complete)
                onComplete();
            
          //  if(fnClose)
           // fnClose(this);

        }

        long HttpClient::sendHeader() {

            LTrace("TcpHTTPConnection::sendHeader()")

            if (!_shouldSendHeader)
                return 0;
            _shouldSendHeader = false;
            assert(outgoingHeader());

            // std::ostringstream os;
            // outgoingHeader()->write(os);
            // std::string head(os.str().c_str(), os.str().length());

            std::string head;
            head.reserve(256);
            outgoingHeader()->write(head);

            // Send headers directly to the Socket,
            // bypassing the ConnectionAdapter

            LTrace("TcpHTTPConnection::sendHeader:head")

            STrace << head;
            TcpConnection::send((const char*) head.c_str(), head.length());
            return head.length();
        }

        Message * HttpClient::incomingHeader() {
            return reinterpret_cast<Message*> (&_response);
        }

        Message * HttpClient::outgoingHeader() {
            return reinterpret_cast<Message*> (&_request);
        }

        //
        // HTTPs Client
        //
        //
        // ClientSec Connection
        //

        /********************************************************************************************************************************/
        /*
               HttpsClient::HttpsClient(Listener* listener, const URL& url, http_parser_type type, size_t bufferSize)
               : SslConnection(listener, bufferSize), ClientConnecton(type), listener(listener)
               , _url(url)
               , _connect(false)
               , _active(false)
               , _complete(false) {

                   auto uri = url.pathEtc();
                   if (!uri.empty())
                       _request.setURI(uri);
                   _request.setHost(url.host(), url.port());

                   // Set default error status
                   _response.setStatus(StatusCode::BadGateway);


                   _parser.setObserver(this);
                   if (type == HTTP_REQUEST)
                       _parser.setRequest(&_request);
                   else
                       _parser.setResponse(&_response);

                   // replaceAdapter(new ConnectionAdapter(this, HTTP_RESPONSE));

                   if (url.scheme() == "ws") {

                       //  conn->replaceAdapter(new ws::ConnectionAdapter(conn.get(), ws::ClientSide));
                       wsAdapter = new WebSocketConnection(listener, this, ClientSide);
                   }


               }

               HttpsClient::~HttpsClient() {
                   // LTrace("Destroy")
               }

               void HttpsClient::send() {
                   connect();
               }

               void HttpsClient::send(Request& req) {
                   assert(!_connect);
                   _request = req;
                   connect();
               }

               void HttpsClient::send(const char* data, size_t len) {
                   connect();

                   if (_active)
                       // Raw data will be pushed onto the Outgoing packet stream
                       SslConnection::send(data, len);
                   else
                       _outgoingBuffer.push_back(std::string((char*) data, len));
                   return;
               }

               void HttpsClient::send(const std::string &str) {
                   connect();

                   if (_active)
                       // Raw data will be pushed onto the Outgoing packet stream
                       //  TcpConnectionBase::Write(str.c_str(), str.length());
                       // else
                       _outgoingBuffer.push_back(str);
                   return;
               }

               void HttpsClient::cbDnsResolve(addrinfo* res, std::string ip) {

                   end_time = base::Application::GetTime();

                   LInfo("{Resolve time(ms) ", _url.host(), " : ", (end_time - start_time), "}")

                   if (!_connect) {
                       _connect = true;

                       start_time = base::Application::GetTime();
                       LTrace("Connecting ", ip, ":", _url.port())
                       Connect(_url.host(), _url.port(), res);
                   }

               }

               void HttpsClient::connect() {
                   LTrace("Resolve DNS ", _url.host());

                   start_time = base::Application::GetTime();

                   resolve(_url.host(), _url.port(), Application::uvGetLoop());
               }

               void HttpsClient::setReadStream(std::ostream* os) {
                   assert(!_connect);

                   //Incoming.attach(new StreamWriter(os), -1, true);
                   _readStream.reset(os);
               }

               //
               // Socket Callbacks

               void HttpsClient::on_connect() {
                   LTrace("On_connect")

                   end_time = base::Application::GetTime();
                   LInfo("{Connect time(ms) ", _url.host(), " : ", (end_time - start_time), "}")

                   start_time = base::Application::GetTime();
                   // Set the connection to active
                   _active = true;

                   // Emit the connect signal so raw connections like
                   // websockets can kick off the data flow
                   //   Connect.emit();

                   // Flush queued packets
                   if (!_outgoingBuffer.empty()) {
                       // LTrace("Sending buffered: ", _outgoingBuffer.size())
                       for (const auto& packet : _outgoingBuffer) {
                           SslConnection::send((const char*) packet.c_str(), packet.length());
                       }
                       _outgoingBuffer.clear();
                   } else {

                       // Send the header
                       sendHeader();
                   }

                   // Send the outgoing HTTP header if it hasn't already been sent.
                   // Note the first call to socket().send() will flush headers.
                   // Note if there are stream adapters we wait for the stream to push
                   // through any custom headers. See ChunkedAdapter::emitHeader
                   //if (Outgoing.numAdapters() == 0) {
                   //    // LTrace("On connect: Send header")
                   //    sendHeader();
                   //}
               }

               void HttpsClient::on_read(const char* data, size_t len) {

                   LTrace("On socket recv: ", len);
                   // LTrace("On socket recv: ", data);

                   recvBytes += len;
                   //HttpsConnection::on_read( data, len);

                   _parser.parse((const char*) data, len);
                   // onPayload( data, len);

                   //LInfo("On socket recv: ", len , " : " , recvBytes, " : ", end_time- start_time );

                   static double latency = 0;

                   latency = latency + (double) ((base::Application::GetTime()) - end_time);

                   latency = latency / 2;

                   end_time = base::Application::GetTime();
                   LInfo("{Dowloadspeed Bits/s ", _url.host(), " : ", double( double(recvBytes)*8.0 * 1000.00 / ((end_time - start_time)*1.00)), ", Latency(ms) ", _url.host(), " : ", latency, "}")



               }
               // Connection Callbacks

               void HttpsClient::onHeaders() {
                   // LTrace("On headers")
                   //IncomingProgress.total = _response.getContentLength();

                   // Headers.emit(_response);
               }

               void HttpsClient::on_payload(const char* data, size_t len) {
                   // LTrace("On payload: ", buffer.size())

                   //// Update download progress
                   //IncomingProgress.update(buffer.size());

                   //// Write to the incoming packet stream if adapters are attached
                   //if (Incoming.numAdapters() > 0 || Incoming.emitter.nslots() > 0) {
                   //    // if (!Incoming.active());
                   //    //     throw std::runtime_error("startInputStream() must be called");
                   //    Incoming.write(bufferCast<const char*>(buffer), buffer.size());
                   //}

                   // Write to the STL read stream if available
                   if (_readStream) {
                       LTrace("Stream len: ", len)
                       LTrace("Stream data: ", data)
                       _readStream->write((const char*) data, len);
                       _readStream->flush();
                       //Close();
                   }

                   // Payload.emit(buffer);
               }

               void HttpsClient::onComplete() {
                   // LTrace("On complete")

                   assert(!_complete);
                   _complete = true; // in case close() is called inside callback

                   // Release any file handles
                   if (_readStream) {
                       auto fstream = dynamic_cast<std::ofstream*> (_readStream.get());
                       if (fstream) {
                           // LTrace("Closing file stream")
                           fstream->close();
                       }
                   }

                   //   Complete.emit(_response);
               }

               void HttpsClient::on_close() {
                   // LTrace("On close")

                   if (!_complete)
                       onComplete();
                   //Close.emit(*this);
               }

               long HttpsClient::sendHeader() {

                   LTrace("TcpHTTPConnection::sendHeader()")

                   if (!_shouldSendHeader)
                       return 0;
                   _shouldSendHeader = false;
                   assert(outgoingHeader());

                   // std::ostringstream os;
                   // outgoingHeader()->write(os);
                   // std::string head(os.str().c_str(), os.str().length());

                   std::string head;
                   head.reserve(256);
                   outgoingHeader()->write(head);

                   // Send headers directly to the Socket,
                   // bypassing the ConnectionAdapter

                   LTrace("TcpHTTPConnection::sendHeader:head")

                   STrace << head;
                   SslConnection::send((const char*) head.c_str(), head.length());
                   return head.length();
               }

               Message * HttpsClient::incomingHeader() {"./mediaserver/src/http/src/w*.cpp
                   return reinterpret_cast<Message*> (&_response);
               }

               Message * HttpsClient::outgoingHeader() {

                   return reinterpret_cast<Message*> (&_request);
               }

               //
               // HTTP Client
               //
         */

        /************************************************************************************************************************/
        Client::Client(URL url) : _url(url) {
            // LTrace("Create")
        }

        Client::Client() {

        }

        void Client::createConnection(const std::string& protocol, const std::string &ip, int port, const std::string& query) {
            std::ostringstream url;
            url << protocol << "://" + ip << ":" << port << query << std::endl;
            _url = url.str();

        }

        Client::~Client() {
            LTrace("Destroy")
            shutdown();
        }

        void Client::shutdown() {
            if (clientConn) {
                clientConn->Close();
                delete clientConn;
                clientConn = nullptr;
            }
        }

        void Client::start() {
            if (_url.scheme() == "http" || _url.scheme() == "ws") {
                clientConn = new HttpClient(nullptr, _url, HTTP_RESPONSE);
            } else if (_url.scheme() == "https" || _url.scheme() == "wss") {
                //clientConn = new HttpsClient(nullptr, _url, HTTP_RESPONSE, 6553688);
            }
        }

        void Client::on_read(Listener* connection, const uint8_t* data, size_t len) {

        }

    } // namespace net
} // namespace base
