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
//#include "base/application.h"
#include "http/responder.h"

namespace base {
    namespace net {
        
 TcpHTTPConnection::TcpHTTPConnection(Listener* listener, http_parser_type type, size_t bufferSize)
        : TcpConnectionBase(bufferSize), listener(listener), _parser(type) {


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

            this->listener->OnTcpConnectionPacketReceived(this, data, len);
        }

        void TcpHTTPConnection::Send(const uint8_t* data, size_t len) {


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


                    this->listener->onHeaders(this);

            // Set the position to the end of the headers once
            // they have been handled. Subsequent body chunks will
            // now start at the correct position.
            // _connection.incomingBuffer().position(_parser._parser.nread);
        }

        void TcpHTTPConnection::onParserChunk(const char* buf, size_t len) {
            LTrace("On parser chunk: ", len)

                    // Dispatch the payload
                    /* if (_connection)
                     {
                         net::SocketAdapter::onSocketRecv(*_connection->socket().get(),
                                 mutableBuffer(const_cast<char*> (buf), len),
                                 _connection->socket()->peerAddress());
                     }*/
        }

        void TcpHTTPConnection::onParserEnd() {
            LTrace("On parser end")

                    this->listener->onComplete(this);
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

    } // namespace net
} // base