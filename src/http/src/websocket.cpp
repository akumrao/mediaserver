/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */



#include "net/netInterface.h"
#include "http/HttpConn.h"
#include "http/websocket.h"
#include "base/base64.h"
//#include "crypto/hash.h"
#include "base/sha1.h"
//#include "http/client.h"
#include "http/HttpConn.h"
#include "base/logger.h"
//#include "base/numeric.h"

#include <stdexcept>
#include <inttypes.h>


using std::endl;


namespace base {
    namespace net {

        WebSocketConnection::WebSocketConnection(Listener* listener,  HttpBase* connection,  Mode mode)  
         : _request(connection->_request),
         _response(connection->_response), 
         _connection(connection),
          listener(listener),
         framer(mode)
        {
            _connection->shouldSendHeader(false);
            
            dummy_timer.cb_timeout = std::bind(&WebSocketConnection::dummy_timer_cb, this);
            dummy_timer.Start(7,7);
        }

        bool WebSocketConnection::shutdown(uint16_t statusCode, const std::string& statusMessage) {
            char buffer[256];
            BitWriter writer(buffer, 256);
            writer.putU16(statusCode);
            writer.put(statusMessage);

            assert(socket);
             send(buffer, writer.position(),
                    unsigned(FrameFlags::Fin) | unsigned(Opcode::Close));
            
            return true;
        }
        
        bool WebSocketConnection::pong() {
            char buffer[2];
            BitWriter writer(buffer, 2);
            //writer.putU16(statusCode);
            //writer.put(statusMessage);

            assert(socket);
             send(buffer, writer.position(),
                    unsigned(FrameFlags::Fin) | unsigned(Opcode::Pong));
            
            return true;
        }

        void WebSocketConnection::send(const char* data, size_t len, bool binary , onSendCallback cb) {
           // LTrace("Send: ", len, ": ", std::string(data, len))
            assert(framer.handshakeComplete());

            // Set default text flag if none specified
            
            int flags;
            if (!binary)
                flags = SendFlags::Text;
            else
                 flags = SendFlags::Binary;
                
            // Frame and send the data
            Buffer buffer;
            buffer.reserve(len + WebSocketFramer::MAX_HEADER_LENGTH);
            BitWriter writer(buffer);
            framer.writeFrame(data, len, flags, writer);

            assert(socket);
            _connection->tcpsend((const char*) writer.begin(), writer.position(), cb);
            
            
        }

        void WebSocketConnection::sendClientRequest() {
            framer.createClientHandshakeRequest(_request);

            std::ostringstream oss;
            _request.write(oss);
            LTrace("Client request: ", oss.str())

            assert(socket);
            _connection->tcpsend((const char*) oss.str().c_str(), oss.str().length(),  nullptr);
        }

    
        void WebSocketConnection::onHandshakeComplete() {
            LTrace("onHandshakeComplete");

            if(_connection->fnConnect)
            _connection->fnConnect(_connection);
            
            if(listener)
            listener->on_connect( this);
            // Call net::SocketEmitter::onSocketConnect to notify handlers that data may flow
            //net::SocketEmitter::onSocketConnect(*socket.get());
            

            
        }
        
        void WebSocketConnection::push( const char* data, size_t len, bool binary, int frametype )
        { 
            dummy_mutex.lock();
            
            Store store;
            store.binary = binary;
            store.buff =  std::string(data, len );
            store.frametype = frametype;//  1 ftype, 2 moov , 3 first moof & mdat( idr or I frame),   4 P or B frames cane be dropped
            
            if (frametype == first_frame || first_frame >=3 )
            {
		if(first_frame  < 6)
                ++first_frame;
                dummy_queue.push(store);
            }
            
            dummy_mutex.unlock();
        }

        void WebSocketConnection::dummy_timer_cb() {
          
            
//            static int x = 0;
//            char tmp[5];
//            sprintf( tmp,  "%d", x++ );
//            
//            send(tmp,5 , false);
            
          //  SInfo << "WebSocketConnection " <<    uv_thread_self();
            
            
             //net::HttpConnection* cn = (net::HttpConnection*)connection;
                
           // int qsize =  ((net::HttpConnection*)_connection)->write_queue_size();

            auto cb = onSendCallback([this](bool sent)
            {
                    if (sent)
                    {
                        --this->qsize;
                    }
            }
            );

           
            while(dummy_queue.size())
            {  
                Store tmp;
                dummy_mutex.lock();
            
                tmp = dummy_queue.front();
                dummy_queue.pop();
                dummy_mutex.unlock();
                //1 ftype, 2 moov, 3 first moof(idr frame),   4 P or B frames cane be dropped
               if(  true  ||  (!dropping &&  qsize < 45   )  ||  (dropping &&  qsize < 25 && tmp.frametype == 1 )   )     /// 25  1 2 3 4 4 4 4 4 ( recent files)
               {
                   if (tmp.buff.length())
                   {  
                       ++qsize;
                       send(&tmp.buff[0], tmp.buff.length(), tmp.binary, cb);
                   }
                    
                    dropping = false;
               }
               else
               {
                   dropping = true;
                   SInfo << "dropping frame, storage queueze " <<  dummy_queue.size() << " pending Queue Size "  <<   qsize;
                   
               }
            }
            
            
           
           
        }
        
        void WebSocketConnection::handleServerRequest(const std::string & buffer) {
            LTrace("Server request: ", buffer)

            Parser parser(&_request);
            if (!parser.parse(buffer.c_str(), buffer.size())) {
                throw std::runtime_error("WebSocket error: Cannot parse request: Incomplete HTTP message");
            }

            LTrace("Verifying handshake: ", _request)

                    // Allow the application to verify the incoming request.
                    // TODO: Handle authentication
                    // VerifyServerRequest.emit(request);

                    // Verify the WebSocket handshake request
            try {
                framer.acceptServerRequest(_request, _response);
                LTrace("Handshake success")
            } catch (std::exception& exc) {
                LWarn("Handshake failed: ", exc.what())
            }

            // Allow the application to override the response
            // PrepareServerResponse.emit(response);

            // Send response
            std::ostringstream oss;
            _response.write(oss);

            LTrace(oss.str());
            
            _connection->send( (const char*) oss.str().c_str(), oss.str().length());
        }

        void WebSocketConnection::onSocketConnect() {
            LTrace("On connect")


            // Send the WS handshake request
            // The Connect signal will be sent after the
            // handshake is complete
            sendClientRequest();
        }

        void WebSocketConnection::onSocketRecv( std::string buffer) {
            //LTrace("On recv: ", buffer.size())

            if (framer.handshakeComplete()) 
            {

                // Note: The spec wants us to buffer partial frames, but our
                // software does not require this feature, and furthermore
                // it goes against our nocopy where possible policy.
                // This may need to change in the future, but for now
                // we just parse and emit packets as they arrive.
                //
                // Incoming frames may be joined, so we parse them
                // in a loop until the read buffer is empty.
                 WebSocketFrameType wsFrameTyp;
                 
                if(buffer.size() < 3) 
                {
                     storeBuf = storeBuf + buffer;
                     wsFrameTyp= WebSocketFrameType::INCOMPLETE_FRAME;
                     
                     SInfo << "Incomplete frame buffer, continue for more buffer to make a  ws frame.. "  << this;
                     
                     return;
                }


                if( !storeBuf.empty())
                {
                    buffer = storeBuf + buffer;
                    storeBuf.clear();
                }
                
               
                
                unsigned char msg_opcode = buffer[0] & 0x0F;
                unsigned char msg_fin = (buffer[0] >> 7) & 0x01;
                //unsigned char msg_masked = (buffer[1] >> 7) & 0x01;
                
                switch(msg_opcode)
                        
                {
                    case 0x0:
                    {
                        wsFrameTyp= (msg_fin)?CONTINUATION_FRAME:INCOMPLETE_CONTINUATION_FRAME;   
                    }
                    break;
                    case 0x1:
                    {
                        wsFrameTyp= (msg_fin)?TEXT_FRAME:INCOMPLETE_TEXT_FRAME;
                    }
                    break;
                    case 0x2:
                    {
                        wsFrameTyp= (msg_fin)?BINARY_FRAME:INCOMPLETE_BINARY_FRAME;
                    }
                    break; 
                    
                    case 0x8:
                    {
                        wsFrameTyp= CLOSE_FRAME; 
                        
                         SInfo << "Close "  << this;
                         
                         if(listener)
                        listener->on_close(listener);

                         if(_connection)
                        _connection->Close();
                         
                         return;
                    }
                    break;

                    case 0x9:
                    {
                        wsFrameTyp= PING_FRAME; 
                        SInfo << "Ping "  << this;
                        pong();
                        return;
                    }
                    break;
                    case 0xA:
                    {
                      
                        wsFrameTyp= PONG_FRAME;
                        SInfo << "Pong "  << this;
                    }
                    break;
                    default:
                    {
                        wsFrameTyp= ERROR_FRAME;  
                        SError << "Error frame "  << this;
                    }
                    
                };
                
      

                BitReader reader(buffer);

                size_t total = reader.available();
                size_t offset = reader.position();
                while (offset < total) 
                {
                    char* payload = nullptr;
                    uint64_t payloadLength = 0;
                  
                    // Parse a frame to throw
                    // int payloadLength = framer.readFrame(reader);
                    payloadLength = framer.readFrame(reader, payload);
                   /// assert(payload);
                     if (payloadLength)
                     {
                       
 
                        // Update the next frame offset
                        offset = reader.position(); // + payloadLength;
                        if (offset < total)
                            LTrace("Splitting joined packet at ", offset, " of ", total)


                        // Emit the result packet
                        assert(payload);
                        assert(payloadLength);
                        if(listener)
                        listener->on_read( this,(const char*) payload, payloadLength );
                        
                       // net::SocketEmitter::onSocketRecv(*socket.get(),
                        //  mutableBuffer(payload, (size_t)payloadLength),
                        // peerAddress);
                        if(_connection->fnPayload)
                        _connection->fnPayload(_connection,payload , payloadLength);
                     }
                     else
                     {
                            storeBuf = buffer.substr(offset);
                             return ;
                     }
                }//end while
                assert(offset == total);
            } 
            else 
            {
                try {
                    if (framer.mode() == ClientSide)
                        handleClientResponse(buffer);
                    else
                        handleServerRequest(buffer);
                } catch (std::exception& exc) {
                    LError("Read error: ", exc.what())
                    //socket->setError(exc.what());
                }
                return;
            }
        }

        void WebSocketConnection::onClose() {
            LTrace("On close")

            // Reset state so the connection can be reused
            _request.clear();
            _response.clear();
            framer._headerState = 0;
            framer._frameFlags = 0;

            this->listener->on_close( this);
            // Emit closed event
            //net::SocketEmitter::onSocketClose(*socket.get());
        }

        
      void WebSocketConnection::handleClientResponse(const std::string& buffer) {
            LTrace("Client response: ", buffer)

            const char *data = buffer.c_str();
            Parser parser(&_response);
            size_t nparsed = parser.parse(buffer.c_str(), buffer.size());
            if (nparsed == 0) {
                throw std::runtime_error(
                        "WebSocket error: Cannot parse response: Incomplete HTTP message");
            }

            // TODO: Handle resending request for authentication
            // Should we implement some king of callback for this?

            // Parse and check the response

            if (framer.checkClientHandshakeResponse(_response)) {
                LTrace("Handshake success")
                onHandshakeComplete();
            }

            // If there is remaining data in the packet (packets may be joined)
            // then send it back through the socket recv method.
            size_t remaining = buffer.size() - nparsed;
            if (remaining) {
                onSocketRecv(std::string(&data[nparsed], remaining));
            }
        }


        //
        // WebSocket Connection Adapter
        //

        WebSocketConnection::~WebSocketConnection() {
        }

    
        /*******************************************************************************************************************/
        //
        // WebSocket Framer
        //

        WebSocketFramer::WebSocketFramer(Mode mode)
        : _mode(mode)
        , _frameFlags(0)
        , _headerState(0)
        , _maskPayload(mode == ClientSide) {
        }

        WebSocketFramer::~WebSocketFramer() {
        }

        std::string createKey() {
            return base64::encode(util::randomString(16));
        }

        std::string computeAccept(const std::string& key) {
            std::string accept(key + ProtocolGuid);
            
           // crypto::Hash engine("SHA1");
           // engine.update(key + ProtocolGuid);
           //  return base64::encode(engine.digest());
            
            sha1::SHA1_CTX context;
            sha1::reid_SHA1_Init(&context);
            sha1::reid_SHA1_Update(&context, (uint8_t*)accept.c_str(), accept.length());
            std::vector<uint8_t> digest(SHA1_DIGEST_SIZE);
            sha1::reid_SHA1_Final(&context, &digest[0]);
            return base64::encode(digest);
           
          //  LTrace(base64::encode(engine.digest()));
          //  LTrace(base64::encode(digest));
          
           
        }

        void WebSocketFramer::createClientHandshakeRequest(Request& request) {
            assert(_mode == ClientSide);
            assert(_headerState == 0);

            // Send the handshake requestWebSocket
            _key = createKey();
            // request.clear();
            request.setChunkedTransferEncoding(false);
            request.set("Connection", "Upgrade");
            request.set("Upgrade", "websocket");
            request.set("Sec-WebSocket-Version", ProtocolVersion);
            assert(request.has("Sec-WebSocket-Version"));
            request.set("Sec-WebSocket-Key", _key);
            assert(request.has("Sec-WebSocket-Key"));
            _headerState++;
        }

        bool WebSocketFramer::checkClientHandshakeResponse(Response& response) {
            assert(_mode == ClientSide);
            assert(_headerState == 1);

            switch (response.getStatus()) {
                case StatusCode::SwitchingProtocols:
                {

                    // Complete handshake or throw
                    completeClientHandshake(response);

                    // Success
                    return true;
                }
                case StatusCode::Unauthorized:
                {
                    assert(0 && "authentication not implemented");
                    throw std::runtime_error("WebSocket error: Authentication not implemented"); // ErrorNoHandshake
                }
                    // case StatusCode::UpgradeRequired: {
                    //     // The latest node `ws` package always returns a 426 Upgrade
                    //     Required
                    //     // response, so resend the client websocket updrage request.
                    //     _headerState--;
                    //
                    //     // Need to resend request
                    //     return false;
                    // }
                default:
                    throw std::runtime_error("WebSocket error: Cannot upgrade to WebSocket connection: " + response.getReason()); // ErrorNoHandshake
            }
        }

        void WebSocketFramer::acceptServerRequest(Request& request, Response& response) {
            assert(_mode == ServerSide);

            if ((util::icompare(request.get("Connection", ""), "upgrade") == 0 ||
                    util::icompare(request.get("Connection", ""), "keep-alive, Upgrade") == 0) &&
                    util::icompare(request.get("Upgrade", ""), "websocket") == 0) {
                std::string version = request.get("Sec-WebSocket-Version", "");
                if (version.empty())
                    throw std::runtime_error("WebSocket error: Missing Sec-WebSocket-Version in handshake request"); //, ErrorHandshakeNoVersion
                if (version != ProtocolVersion)
                    throw std::runtime_error("WebSocket error: Unsupported WebSocket version requested: " + version); //, ErrorHandshakeUnsupportedVersion
                std::string key = util::trim(request.get("Sec-WebSocket-Key", ""));
                if (key.empty())
                    throw std::runtime_error("WebSocket error: Missing Sec-WebSocket-Key in handshake request"); //, ErrorHandshakeNoKey

                response.setStatus(StatusCode::SwitchingProtocols);
                response.set("Upgrade", "websocket");
                response.set("Connection", "Upgrade");
                response.set("Sec-WebSocket-Accept", computeAccept(key));

                // Set headerState 2 since the handshake was accepted.
                _headerState = 2;
            } else
                throw std::runtime_error("WebSocket error: No WebSocket handshake"); // ErrorNoHandshake
        }

        size_t WebSocketFramer::writeFrame(const char* data, size_t len, int flags, BitWriter& frame) {
            assert(flags == SendFlags::Text || flags == SendFlags::Binary);
            assert(frame.position() == 0);
            // assert(frame.limit() >= size_t(len + MAX_HEADER_LENGTH));

            frame.putU8(static_cast<uint8_t> (flags));
            uint8_t lenByte(0);
            if (_maskPayload) {
                lenByte |= FRAME_FLAG_MASK;
            }
            if (len < 126) {
                lenByte |= static_cast<uint8_t> (len);
                frame.putU8(lenByte);
            } else if (len < 65536) {
                lenByte |= 126;
                frame.putU8(lenByte);
                frame.putU16(static_cast<uint16_t> (len));
            } else {
                lenByte |= 127;
                frame.putU8(lenByte);
                //frame.putU64(static_cast<uint16_t> (len));// websocket lager number > 65536 writeFrame bug
                frame.putU64(static_cast<uint64_t>(len));
            }

            if (_maskPayload) {
                auto mask = _rnd.next();
                auto m = reinterpret_cast<const char*> (&mask);
                auto b = reinterpret_cast<const char*> (data);
                frame.put(m, 4);
                // auto p = frame.current();
                for (unsigned i = 0; i < len; i++) {
                    // p[i] = b[i] ^ m[i % 4];
                    frame.putU8(b[i] ^ m[i % 4]);
                }
            } else {
                // memcpy(frame.current(), data, len); // offset?
                frame.put(data, len);
            }

            // Update frame length to include payload plus header
            // frame.skip(len);

            // STrace << "Write frame: "
            //      << "\n\tinputLength: " << len
            //      << "\n\tframePosition: " << frame.position()
            //      << "\n\tframeLimit: " << frame.limit()
            //      << "\n\tframeAvailable: " << frame.available()
            //      << endl;

            return frame.position();
        }
        

        uint64_t WebSocketFramer::readFrame(BitReader& frame, char*& payload) {
            assert(handshakeComplete());
            uint64_t limit = frame.limit();
            size_t offset = frame.position();
            // assert(offset == 0);


//            if( frame.available() < 3)
//            {
//                return 0;
//            }

            // Read the frame header
            char header[MAX_HEADER_LENGTH];
            BitReader headerReader(header, MAX_HEADER_LENGTH);
            frame.get(header, 2);
            uint8_t lengthByte = static_cast<uint8_t> (header[1]);
            int maskOffset = 0;
            if (lengthByte & FRAME_FLAG_MASK)
                maskOffset += 4;
            lengthByte &= 0x7f;


            if( lengthByte >frame.available())
            {
                //LTrace( util::format("WebSocket error: Insufficient buffer for payload ")); //, ErrorPayloadTooBig
                return 0;
            }

            if (lengthByte + 2 + maskOffset < MAX_HEADER_LENGTH) {
                frame.get(header + 2, lengthByte + maskOffset);
            } else {
                frame.get(header + 2, MAX_HEADER_LENGTH - 2);
            }

            // Reserved fields
            frame.skip(2);

            // Parse frame header
            uint8_t flags;
            char mask[4];
            headerReader.getU8(flags);
            headerReader.getU8(lengthByte);
            _frameFlags = flags;
            uint64_t payloadLength = 0;
            int payloadOffset = 2;
            if ((lengthByte & 0x7f) == 127) {
                uint64_t l;
                headerReader.getU64(l);
                if (l > limit)
                {
                    
                   // LTrace( util::format("WebSocket error: Insufficient buffer for payload size %" PRIu64, l)); //, ErrorPayloadTooBig
                    return 0;
                }
                payloadLength = l;
                payloadOffset += 8;
            } else if ((lengthByte & 0x7f) == 126) {
                uint16_t l;
                headerReader.getU16(l);
                if (l > limit)
                {
                    
                   // LTrace( util::format("WebSocket error: Insufficient buffer for payload size %" PRIu64, l)); //, ErrorPayloadTooBig
                    return 0;
                }
		payloadLength = l;
                payloadOffset += 2;
            } else {
                uint8_t l = lengthByte & 0x7f;
                if (l > limit)
                 {
                    
                    LTrace( util::format("WebSocket error: Insufficient buffer for payload size %" PRIu64, l)); //, ErrorPayloadTooBig
                    return 0;
                }
                payloadLength = l;
            }
            if (lengthByte & FRAME_FLAG_MASK) {
                headerReader.get(mask, 4);
                payloadOffset += 4;
            }

            if (payloadLength > limit)
             {

                    LError( "WebSocket error: Incomplete frame received"); //, ErrorPayloadTooBig
                    return 0;
             }

            // Get a reference to the start of the payload
            payload = reinterpret_cast<char*> (
                    const_cast<char*> (frame.begin() + (offset + payloadOffset)));

            // Unmask the payload if required
            if (lengthByte & FRAME_FLAG_MASK) {
                auto p = reinterpret_cast<char*> (payload); // frame.data());
                for (uint64_t i = 0; i < payloadLength; i++) {
                    p[i] ^= mask[i % 4];
                }
            }


            if (offset + payloadOffset + payloadLength > limit)
            {

                LError( "WebSocket error: Incomplete frame received"); //, ErrorPayloadTooBig
                return 0;
            }

            // Update frame length to include payload plus header
            frame.seek(size_t(offset + payloadOffset + payloadLength));// arvind need to fix
            // frame.limit(offset + payloadOffset + payloadLength);
            // int frameLength = (offset + payloadOffset);
            // assert(frame.position() == (offset + payloadOffset));

            return payloadLength;
        }

        void WebSocketFramer::completeClientHandshake(Response& response) {
            assert(_mode == ClientSide);
            assert(_headerState == 1);

            std::string connection = response.get("Connection", "");
            if (util::icompare(connection, "Upgrade") != 0)
                throw std::runtime_error("WebSocket error: No \"Connection: Upgrade\" header in handshake response"); //, ErrorNoHandshake
            std::string upgrade = response.get("Upgrade", "");
            if (util::icompare(upgrade, "websocket") != 0)
                throw std::runtime_error("WebSocket error: No \"Upgrade: websocket\" header in handshake response"); //, ErrorNoHandshake
            std::string accept = response.get("Sec-WebSocket-Accept", "");
            if (accept != computeAccept(_key))
                throw std::runtime_error("WebSocket error: Invalid or missing Sec-WebSocket-Accept header in handshake esponse"); //, ErrorNoHandshake

            _headerState++;
            assert(handshakeComplete());
        }

        Mode WebSocketFramer::mode() const {
            return _mode;
        }

        bool WebSocketFramer::handshakeComplete() const {
            return _headerState == 2;
        }

        int WebSocketFramer::frameFlags() const {
            return _frameFlags;
        }

        bool WebSocketFramer::mustMaskPayload() const {
            return _maskPayload;
        }


    } // namespace net
} // namespace base
