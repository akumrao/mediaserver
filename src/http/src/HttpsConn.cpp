/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */


/* 
 * File:   HTTPConnection.cpp
 * Author: Arvind Umrao <akumrao@yahoo.com>
 * 
 * Created on November 18, 2019, 9:42 AM
 */
#include "net/netInterface.h"
#include "http/parser.h"
#include "http/HttpsConn.h"
#include "base/base.h"
#include "base/logger.h"
#include "http/websocket.h"
#include "http/responder.h"

namespace base {
    namespace net {

        HttpsConnection::HttpsConnection(Listener* listener, http_parser_type type)
        : SslConnection(true),
           listener(listener),HttpBase(type),wsAdapter(nullptr){


            //_parser.setObserver(this);
            //if (type == HTTP_REQUEST)
            //    _parser.setRequest(&_request);
         //   else
             //   _parser.setResponse(&_response);

        }

        HttpsConnection::~HttpsConnection() {
            if(wsAdapter)
            {    
                 SInfo <<  "wsAdapter delete connection " << wsAdapter; 
                delete wsAdapter;
                wsAdapter = nullptr;
                 
            }
            
            SInfo << "~HttpsConnection()";
        }

        void HttpsConnection::on_read(const char* data, size_t len) {

            LTrace("on_read()" )
                    
             LTrace( data )
                    
            if(wsAdapter)
            {
                wsAdapter->onSocketRecv( std::string((char*)data, len));
                return;
            }
            
             _parser.parse((const char*) data, len);
             
            //if(!wsAdapter)
            //this->listener->on_read(this, data, len);
        }
        
          void HttpsConnection::on_close() {

            SInfo << "HttpsConnection::on_close()";
                    
            if (_responder) {
                _responder->onClose();
                delete _responder;
            }
             
            this->listener->on_close(this);
        }
        long HttpsConnection::sendHeader() {
            if (!_shouldSendHeader)
                return 0 ;
            _shouldSendHeader = false;
            assert(outgoingHeader());

             LTrace("HttpsConnection::sendHeader()")
                     
            // std::ostringstream os;
            // outgoingHeader()->write(os);
            // std::string head(os.str().c_str(), os.str().length());

            std::string head;
            head.reserve(256);
            outgoingHeader()->write(head);

            // Send headers directly to the Socket,
            // bypassing the WebSocketConnection

             STrace << head;
             SslConnection::send(head.c_str(), head.length());
             return head.length();
        }
        
        void HttpsConnection::Close()
        {
            TcpConnectionBase::Close();
        }
        
        
        void  HttpsConnection::tcpsend(const char* data, size_t len, onSendCallback cb)
        {
               SslConnection::tcpsend(data, len, cb); // arvind
        }
        
        void HttpsConnection::send(const char* data, size_t len, bool binary) {

             LTrace("HttpsConnection::send()")
            
             if (shouldSendHeader())
            {
                long res = sendHeader();

                // The initial packet may be empty to push the headers through
                if (len == 0)
                    return ;
            }

            // Update sent bytes.
           // this->sentBytes += len;

            // Write according to Framing RFC 4571.

            //       uint8_t frameLen[2];

            // Utils::Byte::Set2Bytes(frameLen, 0, len);
            // TcpConnectionBase::Write(frameLen, 2, data, len);
             SslConnection::send(data, len);
        }

     

        void HttpsConnection::onHeaders() {


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
                           
                        if(wsAdapter)
                               delete wsAdapter;
                          wsAdapter = new WebSocketConnection( listener, this, ServerSide);
                          SInfo <<  "wsAdapter new connection " << wsAdapter;  

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
            else
            {

            // Notify the server the connection is ready for data flow
            //   _server.onConnectionReady(*this);

            // Instantiate the responder now that request headers have been parsed
                this->listener->on_header(this);
            }

            // Upgraded connections don't receive the onHeaders callback
            if (_responder && !_upgrade)
                _responder->onHeaders(_request);
        }


        void HttpsConnection::on_payload(const char* data, size_t len){

        }

        void HttpsConnection::onComplete() {

            if (_responder)
                _responder->onRequest(_request, _response);
        }


    } // namespace net
} // base
