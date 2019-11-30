

#include "http/client.h"

#include "base/util.h"
#include "base/application.h"

using std::endl;


namespace base {
    namespace net {
        //
        // Client Connection
        //

        ClientConnection::ClientConnection(Listener* listener, const URL& url, http_parser_type type, size_t bufferSize)
        : TcpConnectionBase(bufferSize), listener(listener), _parser(type), _shouldSendHeader(true)
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

        }

        ClientConnection::~ClientConnection() {
            // LTrace("Destroy")
        }

        void ClientConnection::Send() {
            connect();
        }

        void ClientConnection::Send(Request& req) {
            assert(!_connect);
            _request = req;
            connect();
        }

        void ClientConnection::Send(const uint8_t* data, size_t len) {
            connect();

            if (_active)
                // Raw data will be pushed onto the Outgoing packet stream
                TcpConnectionBase::Write(data, len);
            else
                _outgoingBuffer.push_back(std::string((char*) data, len));
            return;
        }

        void ClientConnection::cbDnsResolve(addrinfo* res) {

            if (!_connect) {
                _connect = true;
                
            std::string servIp;
            uint16_t servPort;
            int family;
            IP::GetAddressInfo(
                    reinterpret_cast<struct sockaddr*> (res), family, servIp, servPort);

                
                LTrace("Connecting " , servIp, ":",servPort)
                Connect(_url.host(), _url.port(), res);
            }

        }

        void ClientConnection::connect() {
            LTrace("Resolve DNS ", _url.host());
            resolve(_url.host(), _url.port(), Application::uvGetLoop());
        }

        void ClientConnection::setReadStream(std::ostream* os) {
            assert(!_connect);

            //Incoming.attach(new StreamWriter(os), -1, true);
            _readStream.reset(os);
        }

        Message* ClientConnection::incomingHeader() {
            return static_cast<Message*> (&_response);
        }

        Message* ClientConnection::outgoingHeader() {
            return static_cast<Message*> (&_request);
        }


        //
        // Socket Callbacks

        void ClientConnection::on_connect() {
            LTrace("On_connect")

            // Set the connection to active
            _active = true;

            // Emit the connect signal so raw connections like
            // websockets can kick off the data flow
            //   Connect.emit();

            // Flush queued packets
            if (!_outgoingBuffer.empty()) {
                // LTrace("Sending buffered: ", _outgoingBuffer.size())
                for (const auto& packet : _outgoingBuffer) {
                    TcpConnectionBase::Write((const uint8_t*) packet.c_str(), packet.length());
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

        void ClientConnection::UserOnTcpConnectionRead(const uint8_t* data, size_t len) {

            LTrace("On socket recv: ", len);
            LTrace("On socket recv: ", data);
            if (this->listener)
                this->listener->OnTcpConnectionPacketReceived(this, data, len);
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
            }
        }
        // Connection Callbacks

        void ClientConnection::onHeaders() {
            // LTrace("On headers")
            //IncomingProgress.total = _response.getContentLength();

            // Headers.emit(_response);
        }

        void ClientConnection::onPayload(const uint8_t* data, size_t len) {
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
            TcpConnectionBase::Write((const uint8_t*) head.c_str(), head.length());
            return head.length();
        }

        void ClientConnection::onParserHeader(const std::string& /* name */,
                const std::string& /* value */) {
        }

        void ClientConnection::onParserHeadersEnd(bool upgrade) {
            LTrace("On headers end: ", _parser.upgrade())


            onHeaders();

            // Set the position to the end of the headers once
            // they have been handled. Subsequent body chunks will
            // now start at the correct position.
            // _connection.incomingBuffer().position(_parser._parser.nread);
        }

        void ClientConnection::onParserChunk(const char* data, size_t len) {
            LTrace("On parser chunk: ", len)
              //  UserOnTcpConnectionRead((const uint8_t*) buf , len);
                     onPayload((const uint8_t*) data, len);
            
        }

        void ClientConnection::onParserEnd() {
            LTrace("On parser end")

            onComplete();
        }

        void ClientConnection::onParserError(const base::Error& err) {
            LWarn("On parser error: ", err.message)

#if 0
                    // HACK: Handle those peski flash policy requests here
                    auto base = dynamic_cast<net::TCPSocket*> (_connection.socket().get());
            if (base && std::string(base->buffer().data(), 22) == "<policy-file-request/>") {

                // Send an all access policy file by default
                // TODO: User specified flash policy
                std::string policy;

                // Add the following headers for HTTP policy response
                // policy += "HTTP/1.1 200 OK\r\nContent-Type: text/x-cross-domain-policy\r\nX-Permitted-Cross-Domain-Policies: all\r\n\r\n";
                policy += "<?xml version=\"1.0\"?><cross-domain-policy><allow-access-from domain=\"*\" to-ports=\"*\" /></cross-domain-policy>";
                base->send(policy.c_str(), policy.length() + 1);
            }
#endif


            // _connection->setError(err.message);

            Close(); // do we want to force this?
        }

        bool ClientConnection::shouldSendHeader() const {
            return _shouldSendHeader;
        }

        void ClientConnection::shouldSendHeader(bool flag) {
            _shouldSendHeader = flag;
        }




        //
        // HTTP Client
        //

        /************************************************************************************************************************/
        Client::Client(URL url) : _url(url) {
            // LTrace("Create")
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

        void Client::OnTcpConnectionPacketReceived(ClientConnection* connection, const uint8_t* data, size_t len) {

        }

    } // namespace net
} // namespace base
