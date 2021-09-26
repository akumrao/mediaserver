/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */



#ifndef NET_WebSocket_H
#define NET_WebSocket_H


#include "base/base.h"
#include "base/buffer.h"
#include "http/parser.h"
#include "http/request.h"
#include "http/response.h"
#include "base/random.h"
#include "base/Timer.h"

#include  <mutex>
#include  <queue>





namespace base {
    namespace net {

        enum Mode {
            ServerSide, ///< Server-side WebSocket.
            ClientSide ///< Client-side WebSocket.
        };


        /// Frame header flags.

        enum class FrameFlags {
            Fin = 0x80, ///< FIN bit: final fragment of a multi-fragment message.
            Rsv1 = 0x40, ///< Reserved for future use. Must be zero.
            Rsv2 = 0x20, ///< Reserved for future use. Must be zero.
            Rsv3 = 0x10, ///< Reserved for future use. Must be zero.
        };


        /// Frame header opcodes.

        enum class Opcode {
            Continuation = 0x00, ///< Continuation frame.
            Text = 0x01, ///< Text frame.
            Binary = 0x02, ///< Binary frame.
            Close = 0x08, ///< Close connection.
            Ping = 0x09, ///< Ping frame.
            Pong = 0x0a, ///< Pong frame.
            Bitmask = 0x0f ///< Bit mask for opcodes.
        };
        
        
        enum WebSocketFrameType {
                ERROR_FRAME=0xFF00,
                INCOMPLETE_FRAME=0xFE00,

                OPENING_FRAME=0x3300,
                CLOSING_FRAME=0x3400,

                INCOMPLETE_TEXT_FRAME=0x01,
                INCOMPLETE_BINARY_FRAME=0x02,
                INCOMPLETE_CONTINUATION_FRAME=0x03,
                
                TEXT_FRAME=0x81,
                BINARY_FRAME=0x82,
                CONTINUATION_FRAME=0x83,

                // Control frame can not be fragmented
                CLOSE_FRAME=0x18,
                PING_FRAME=0x19,
                PONG_FRAME=0x1A
                        
        };



        /// Combined header flags and opcodes for identifying
        /// the payload type of sent frames.

        enum SendFlags {
            Text = unsigned(FrameFlags::Fin) | unsigned(Opcode::Text),
            Binary = unsigned(FrameFlags::Fin) | unsigned(Opcode::Binary)
        };


        /// StatusCodes for CLOSE frames sent with shutdown().

        enum StatusCodes {
            StatusNormalClose = 1000,
            StatusEndpointGoingAway = 1001,
            StatusProtocolError = 1002,
            StatusPayloadNotAcceptable = 1003,
            StatusReserved = 1004,
            StatusReservedNoStatusCode = 1005,
            StatusReservedAbnormalClose = 1006,
            StatusMalformedPayload = 1007,
            StatusPolicyViolation = 1008,
            StatusPayloadTooBig = 1009,
            StatusExtensionRequired = 1010,
            StatusUnexpectedCondition = 1011,
            StatusReservedTLSFailure = 1015
        };


        /// These error codes can be obtained from WebSocket exceptions
        /// to determine the exact cause of the error.

        enum ErrorCodes {
            ErrorNoHandshake = 1, ///< No Connection: Upgrade or Upgrade: websocket header in handshake request.
            ErrorHandshakeNoVersion = 2, ///< No Sec-WebSocket-Version header in handshake request.
            ErrorHandshakeUnsupportedVersion = 3, ///< Unsupported WebSocket version requested by client.
            ErrorHandshakeNoKey = 4, ///< No Sec-WebSocket-Key header in handshake request.
            ErrorHandshakeAccept = 5, ///< No Sec-WebSocket-Accept header or wrong value.
            ErrorUnauthorized = 6, ///< The server rejected the username or password for authentication.
            ErrorPayloadTooBig = 10, ///< Payload too big for supplied buffer.
            ErrorIncompleteFrame = 11 ///< Incomplete frame received.
        };


        static std::string ProtocolGuid = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

        /// The WebSocket protocol version supported (13).
        static std::string ProtocolVersion = "13";


        //
        // WebSocket Framer
        //

        /// This class implements a WebSocket parser according
        /// to the WebSocket protocol described in RFC 6455.

        class  WebSocketFramer {
        public:
            /// Creates a Socket using the given Socket.
            WebSocketFramer(Mode mode);

            virtual ~WebSocketFramer();

            /// Writes a WebSocket protocol frame from the given data.
            virtual size_t writeFrame(const char* data, size_t len, int flags, BitWriter& frame);

            /// Reads a single WebSocket frame from the given buffer (frame).
            ///
            /// The actual payload length is returned, and the beginning of the
            /// payload buffer will be assigned in the second (payload) argument.
            /// No data is copied.
            ///
            /// If the frame is invalid or too big an exception will be thrown.
            /// Return true when the handshake has completed successfully.
            virtual uint64_t readFrame(BitReader& frame, char*& payload);

            bool handshakeComplete() const;

            //
            /// Server side

            void acceptServerRequest(Request& request, Response& response);

            //
            /// Client side

            /// Sends the initial WS handshake HTTP request.
            /// void sendHandshakeRequest();

            /// Appends the WS hanshake HTTP request hearers.
            void createClientHandshakeRequest(Request& request);

            /// Checks the veracity the HTTP handshake response.
            /// Returns true on success, false if the request should
            /// be resent (in case of authentication), or throws on error.
            bool checkClientHandshakeResponse(Response& response);

            /// Verifies the handshake response or thrown and exception.
            void completeClientHandshake(Response& response);

        public:
            /// Returns the frame flags of the most recently received frame.
            /// Set by readFrame()
            int frameFlags() const;

            /// Returns true if the payload must be masched.
            /// Used by writeFrame()
            bool mustMaskPayload() const;

            Mode mode() const;

            enum {
                FRAME_FLAG_MASK = 0x80,
                MAX_HEADER_LENGTH = 14
            };

        public:
            Mode _mode;
            int _frameFlags;
            int _headerState;
            bool _maskPayload;
            Random _rnd;
            std::string _key; // client handshake key

            //friend class WebSocketAdapter;
        };




        /// WebSocket class which belongs to a HTTP Connection.
        //class HttpConnection;
        class WebSocketConnection: public Listener{
        public:
            
            
                   
            Listener* listener{ nullptr};
            
            WebSocketConnection(Listener* listener, HttpBase* connection, Mode mode);

            virtual ~WebSocketConnection() ;

            void onSocketRecv( std::string buffer);

            std::string storeBuf;

            void send(const char* data, size_t len, bool binary =false);
            

           // void send(const char* data, size_t len, int flags) ; // flags = Text || Binary

            bool shutdown(uint16_t statusCode, const std::string& statusMessage);
            bool pong();
            
            void dummy_timer_cb();
            
            void push( const char* data, size_t len, bool binary);
            //
            /// Client side

            virtual void sendClientRequest();
            virtual void handleClientResponse(const std::string& buffer);
            // virtual void prepareClientRequest(Request& request);
            // virtual void verifyClientResponse(Response& response);

            //
            /// Server side

            virtual void handleServerRequest(const std::string& buffer);
            // virtual void sendConnectResponse();
            // virtual void verifyServerRequest(Request& request);
            // virtual void prepareClientResponse(Response& response);

            virtual void onSocketConnect() ;
            virtual void onClose() ;


            virtual void onHandshakeComplete();
            
            

        protected:
            HttpBase* _connection{nullptr};

            friend class WebSocketFramer;

            WebSocketFramer framer;

            Request& _request;
            Response& _response;
            
            Timer dummy_timer{ nullptr};
            std::mutex dummy_mutex;
            std::queue< std::pair< bool, std::string >> dummy_queue;
        };



    } // namespace net
} // namespace base


#endif // NET_WebSocket_H

