
#include "net/netInterface.h"
#include "http/websocket.h"
#include "http/client.h"
#include "net/IP.h"
#include "base/util.h"
#include "base/application.h"

using std::endl;


namespace base {
    namespace net {
        //
        // Client Connection
        //

        ClientConnection::ClientConnection(Listener* listener, const URL& url, http_parser_type type, size_t bufferSize)
        : HttpConnection(listener, type, bufferSize)
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

        ClientConnection::~ClientConnection() {
            // LTrace("Destroy")
        }

        void ClientConnection::send() {
            connect();
        }

        void ClientConnection::send(Request& req) {
            assert(!_connect);
            _request = req;
            connect();
        }

        void ClientConnection::send(const char* data, size_t len) {
            connect();

            if (_active)
                // Raw data will be pushed onto the Outgoing packet stream
                HttpConnection::send(data, len);
            else
                _outgoingBuffer.push_back(std::string((char*) data, len));
            return;
        }

        void ClientConnection::send(const std::string &str) {
            connect();

            if (_active)
                // Raw data will be pushed onto the Outgoing packet stream
                //  TcpConnectionBase::Write(str.c_str(), str.length());
                // else
                _outgoingBuffer.push_back(str);
            return;
        }

        void ClientConnection::cbDnsResolve(addrinfo* res, std::string ip) {

             end_time = base::Application::GetTime();
             
             LInfo( "{Resolve time(ms) ",_url.host(), " : ", (end_time- start_time ), "}" )
            
            if (!_connect) {
                _connect = true;
       
                start_time = base::Application::GetTime();
                LTrace("Connecting ", ip, ":", _url.port())
                Connect(_url.host(), _url.port(), res);
            }

        }

        void ClientConnection::connect() {
            LTrace("Resolve DNS ", _url.host());
            
            start_time = base::Application::GetTime();
            
            resolve(_url.host(), _url.port(), Application::uvGetLoop());
        }

        void ClientConnection::setReadStream(std::ostream* os) {
            assert(!_connect);

            //Incoming.attach(new StreamWriter(os), -1, true);
            _readStream.reset(os);
        }


        //
        // Socket Callbacks

        void ClientConnection::on_connect() {
            LTrace("On_connect")

            end_time = base::Application::GetTime();
            LInfo( "{Connect time(ms) ",_url.host(), " : ", (end_time- start_time ), "}" )         
                    
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
                    TcpConnection::send((const char*) packet.c_str(), packet.length());
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

        void ClientConnection::on_read(const char* data, size_t len) {

            LTrace("On socket recv: ", len);
           // LTrace("On socket recv: ", data);
            
             recvBytes+= len;            
            HttpConnection::on_read( data, len);
            
            //LInfo("On socket recv: ", len , " : " , recvBytes, " : ", end_time- start_time );
            
            static double latency = 0; 
            
            latency = latency + (double)((int64_t) (base::Application::GetTime()) - end_time);
                    
            latency =latency/2;
         
            LInfo( "{Latency(ms) ",_url.host(), " : ", latency , "}" )  

            end_time = base::Application::GetTime();
            LInfo( "{Dowloadspeed Bits/s ",_url.host(), " : ", double( double(recvBytes)*8.0*1000.00 / ((end_time- start_time )*1.00)), "}" )  
                    
            
           /* if (this->listener)
                this->listener->on_read(this, data, len);
            else {



                if (_parser.complete()) {
                    // Buggy HTTP servers might send late data or multiple responses,
                    // in which case the parser state might already be HPE_OK.
                    // In this case we discard the late message and log the error here,
                    // rather than complicate the app with this error handling logic.
                    // This issue was noted using Webrick with Ruby 1.9.
                    LWarn("Dropping late HTTP response: ", data)
                    return;
                }

                // Parse incoming HTTP messages
                _parser.parse((const char*) data, len);
                // onPayload( data, len);
            }*/
        }
        // Connection Callbacks

        void ClientConnection::onHeaders() {
            // LTrace("On headers")
            //IncomingProgress.total = _response.getContentLength();

            // Headers.emit(_response);
        }

        void ClientConnection::on_payload(const uint8_t* data, size_t len) {
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

        void ClientConnection::onComplete() {
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

        void ClientConnection::on_close() {
            // LTrace("On close")

            if (!_complete)
                onComplete();
            //Close.emit(*this);
        }

        long ClientConnection::sendHeader() {

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


        Message * ClientConnection::incomingHeader() {
            return reinterpret_cast<Message*> (&_response );
        }

        Message * ClientConnection::outgoingHeader() {

            return reinterpret_cast<Message*> (&_request);
        }

        //
        // HTTP Client
        //

        /************************************************************************************************************************/
        Client::Client(URL url) : _url(url) {
            // LTrace("Create")
        }

        Client::Client()
        {
            
        }
        void Client::createConnection(const std::string& protocol, const std::string &ip, int port, const std::string& query) {
            std::ostringstream url;
            url << protocol << "://" + ip << ":" << port << query << std::endl;
            _url = url.str();

        }

        Client::~Client() {
            // LTrace("Destroy")
            shutdown();
        }

        void Client::shutdown() {
            if (clientConn) {
                delete clientConn;
                clientConn = nullptr;
            }
        }

        void Client::start() {
            clientConn = new ClientConnection(nullptr, _url, HTTP_RESPONSE, 6553688);
        }

        void Client::on_read(ClientConnection* connection, const uint8_t* data, size_t len) {

        }

    } // namespace net
} // namespace base
