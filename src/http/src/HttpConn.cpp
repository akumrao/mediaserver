/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   HTTPConnection.cpp
 * Author: root
 * 
 * Created on November 18, 2019, 9:42 AM
 */

#include "http/HttpConn.h"
#include "base/base.h"
#include "base/logger.h"
#include "http/websocket.h"
#include "http/responder.h"

namespace base {
    namespace net {

        TcpHTTPConnection::TcpHTTPConnection(Listener* listener, http_parser_type type, WebSocketConnection::Listener * wslis, size_t bufferSize)
        : TcpConnectionBase(bufferSize),
           listener(listener), wsListener(wslis), _parser(type),wsAdapter(nullptr), _shouldSendHeader(true) {


            _parser.setObserver(this);
            if (type == HTTP_REQUEST)
                _parser.setRequest(&_request);
            else
                _parser.setResponse(&_response);

        }

        TcpHTTPConnection::~TcpHTTPConnection() {
            LTrace("~TcpHTTPConnection()")
        }

        void TcpHTTPConnection::UserOnTcpConnectionRead(const uint8_t* data, size_t len) {

            if(wsAdapter)
            {
                wsAdapter->onSocketRecv( std::string((char*)data, len));
                return;
            }
            this->listener->OnTcpConnectionPacketReceived(this, data, len);
        }
        long TcpHTTPConnection::sendHeader() {
            if (!_shouldSendHeader)
                return 0 ;
            _shouldSendHeader = false;
            assert(outgoingHeader());

             LTrace("TcpHTTPConnection::sendHeader()")
                     
            // std::ostringstream os;
            // outgoingHeader()->write(os);
            // std::string head(os.str().c_str(), os.str().length());

            std::string head;
            head.reserve(256);
            outgoingHeader()->write(head);

            // Send headers directly to the Socket,
            // bypassing the WebSocketConnection

             STrace << head;
             TcpConnectionBase::Write((const uint8_t*)head.c_str(), head.length());
             return head.length();
        }
        
        void TcpHTTPConnection::Send(const uint8_t* data, size_t len) {

             LTrace("TcpHTTPConnection::send()")
            
             if (shouldSendHeader())
            {
                long res = sendHeader();

                // The initial packet may be empty to push the headers through
                if (len == 0)
                    return ;
            }

            // Update sent bytes.
            this->sentBytes += len;

            // Write according to Framing RFC 4571.

            //       uint8_t frameLen[2];

            // Utils::Byte::Set2Bytes(frameLen, 0, len);
            // TcpConnectionBase::Write(frameLen, 2, data, len);
            TcpConnectionBase::Write(data, len);
        }

        void TcpHTTPConnection::onParserHeader(const std::string& /* name */,
                const std::string& /* value */) {
        }

        void TcpHTTPConnection::onParserHeadersEnd(bool upgrade) {
            LTrace("On headers end: ", _parser.upgrade())


                    //this->listener->onHeaders(this);
            onHeaders();

            // Set the position to the end of the headers once
            // they have been handled. Subsequent body chunks will
            // now start at the correct position.
            // _connection.incomingBuffer().position(_parser._parser.nread);
        }

        void TcpHTTPConnection::onParserChunk(const char* buf, size_t len) {
            LTrace("On parser chunk: ", len)
            //abort();
               
                    // Dispatch the payload
             onPayload((const uint8_t*) buf, len);
                 
        }

        void TcpHTTPConnection::onParserEnd() {
            LTrace("On parser end")

            onComplete();
        }

        void TcpHTTPConnection::onParserError(const base::Error& err) {
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

        void TcpHTTPConnection::onHeaders() {


            bool _upgrade = _parser.upgrade();
            if (_upgrade && util::icompare(_request.get("Upgrade", ""), "websocket") == 0) {
                // if (util::icompare(request().get("Connection", ""), "upgrade") == 0 &&
                //     util::icompare(request().get("Upgrade", ""), "websocket") == 0){LOG_CALL;
                LTrace("Upgrading to WebSocket: ", _request)

                        // Note: To upgrade the connection we need to replace the
                        // underlying SocketAdapter instance. Since we are currently
                        // inside the default WebSocketConnection's HTTP parser callback
                        // scope we just swap the SocketAdapter instance pointers and do
                        // a deferred delete on the old adapter. No more callbacks will be
                        // received from the old adapter after replaceAdapter is called.
                          wsAdapter = new WebSocketConnection(  wsListener, this, ServerSide);
                        //   replaceAdapter(wsAdapter);

                           // Send the handshake request to the WS adapter for handling.
                           // If the request fails the underlying socket will be closed
                           // resulting in the destruction of the current connection.

                           // std::ostringstream oss;
                           // request().write(oss);
                           // request().clear();
                           // std::string buffer(oss.str());

                           std::string buffer;
                           buffer.reserve(256);
                           _request.write(buffer);
                           _request.clear();

                           wsAdapter->onSocketRecv( buffer);
            }

            // Notify the server the connection is ready for data flow
            //   _server.onConnectionReady(*this);

            // Instantiate the responder now that request headers have been parsed
            this->listener->onHeaders(this);

            // Upgraded connections don't receive the onHeaders callback
            if (_responder && !_upgrade)
                _responder->onHeaders(_request);
        }

 

        bool TcpHTTPConnection::shouldSendHeader() const {
            return _shouldSendHeader;
        }

        void TcpHTTPConnection::shouldSendHeader(bool flag) {
            _shouldSendHeader = flag;
        }

        void TcpHTTPConnection::onPayload(const uint8_t* data, size_t len){

        }

        void TcpHTTPConnection::onComplete() {

            if (_responder)
                _responder->onRequest(_request, _response);
        }

     //   void TcpHTTPConnection::onClose() {

         //   if (_responder)
             //   _responder->onClose();
       // }

        Message* TcpHTTPConnection::incomingHeader() {
            return reinterpret_cast<Message*> (&_request);
        }

        Message* TcpHTTPConnection::outgoingHeader() {

            return reinterpret_cast<Message*> (&_response);
        }

    } // namespace net
} // base