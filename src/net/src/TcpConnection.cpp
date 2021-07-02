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
#include "net/TcpConnection.h"
#include "base/logger.h"
#include <inttypes.h>
#include "net/IP.h"
#include "base/application.h"
#include <cstdlib> // std::malloc(), std::free()
#include <cstring> // std::memcpy()

// TcpConnection class is for RFC 4571 for RTP transport. Please do not use it other than SFU/MCU.

namespace base {
    namespace net {

        /* Static methods for UV callbacks. */
        inline static void onAlloc(uv_handle_t* handle, size_t suggestedSize, uv_buf_t* buf) {
            auto* connection = static_cast<TcpConnectionBase*> (handle->data);

            if (connection == nullptr)
                return;
            connection->OnUvReadAlloc(suggestedSize, buf);
        }

        inline static void onRead(uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf) {
            auto* connection = static_cast<TcpConnectionBase*> (handle->data);

            if (connection)
            	connection->OnUvRead(nread, buf);
        }

        inline static void onWrite(uv_write_t* req, int status) {
            auto* writeData = static_cast<TcpConnectionBase::UvWriteData*> (req->data);
            auto* handle = req->handle;
            auto* connection = static_cast<TcpConnectionBase*> (handle->data);
            auto cb         = writeData->cb;

            // Delete the UvWriteData struct (which includes the uv_req_t and the store char[]).
            if (connection)
                connection->OnUvWrite(status,cb);
            delete writeData;

        }

        inline static void onClose(uv_handle_t* handle) {

            LInfo("onClose");
            TcpConnectionBase *obj = (TcpConnectionBase *) handle->data;

            if (obj)
                obj->on_close();
            delete handle;
            handle = nullptr;
        }

        inline static void onShutdown(uv_shutdown_t* req, int /*status*/) {

            LTrace("onShutdown");

            auto* handle = req->handle;
            handle->data = req->data;
            //  delete handle;
            delete req;

            // Now do close the handle.
            uv_close(reinterpret_cast<uv_handle_t*> (handle), static_cast<uv_close_cb> (onClose));
        }

        /* Instance methods. */

        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)

        TcpConnectionBase::TcpConnectionBase(bool tls) : tls(tls) {

            this->uvHandle = new uv_tcp_t;
            this->uvHandle->data = (void*) this;

            // NOTE: Don't allocate the buffer here. Instead wait for the first uv_alloc_cb().
        }

        TcpConnectionBase::~TcpConnectionBase() {


            if (!this->closed)
                Close();

            delete[] this->buffer;
        }

        void TcpConnectionBase::Close() {

            LTrace("Close ", this->uvHandle)
            if (this->closed)
                return;

           // int err;

            this->closed = true;

            this->uvHandle->data = this;

            // Tell the UV handle that the TcpConnectionBase has been closed.
            /*this->uvHandle->data = nullptr;

            
            // Don't read more.
            err = uv_read_stop(reinterpret_cast<uv_stream_t*> (this->uvHandle));
            if (err != 0)
              
             LError("uv_read_stop() failed: %s", uv_strerror(err));
           
            this->uvHandle->data = this;

             // If there is no error and the peer didn't close its connection side then close gracefully.
            if (!this->hasError && !this->isClosedByPeer) {
                // Use uv_shutdown() so pending data to be written will be sent to the peer
                // before closing.
                LTrace( "uv_shutdown_t");
                auto req = new uv_shutdown_t;
               // uvHandle->type = UV_TCP;
                req->data = (void*) this;
                err = uv_shutdown(
                        req, reinterpret_cast<uv_stream_t*> (this->uvHandle), static_cast<uv_shutdown_cb> (onShutdown));

                if (err != 0)
                    LError("uv_shutdown() failed: %s", uv_strerror(err));
                //on_close();
            }// Otherwise directly close the socket.
            else*/
            {
                uv_close(reinterpret_cast<uv_handle_t*> (this->uvHandle), static_cast<uv_close_cb> (onClose));
            }
        }

        void TcpConnectionBase::Dump() const {
            LDebug("<TcpConnectionBase>");
            LDebug(
                    "  [TCP, local:%s :%" PRIu16 ", remote:%s :%" PRIu16 ", status:%s]",
                    this->localIp.c_str(),
                    static_cast<uint16_t> (this->localPort),
                    this->peerIp.c_str(),
                    static_cast<uint16_t> (this->peerPort),
                    (!this->closed) ? "open" : "closed");
            LDebug("</TcpConnectionBase>");
        }

        void TcpConnectionBase::Setup(
                struct sockaddr_storage* localAddr, const std::string& localIp, uint16_t localPort) {


            // Set the UV handle.
            int err = uv_tcp_init(Application::uvGetLoop(), this->uvHandle);

            if (err != 0) {
                delete this->uvHandle;
                this->uvHandle = nullptr;

                LError("uv_tcp_init() failed: %s", uv_strerror(err));
            }


            // Set the local address.
            this->localAddr = localAddr;
            this->localIp = localIp;
            this->localPort = localPort;
        }

        inline void onconnect(uv_connect_t* req, int /*status*/) {
            TcpConnectionBase *obj = (TcpConnectionBase *) req->data;
            obj->Start();
            obj->on_connect();

            delete req;
        }

        void TcpConnectionBase::Connect(std::string ip, int port, addrinfo *addrs) { //for client


            struct sockaddr_in6 addr6;
            struct sockaddr_in addr;


            /////////////////////////////////////////////////////////////
            int err = uv_tcp_init(Application::uvGetLoop(), this->uvHandle);

            if (err != 0) {
                delete this->uvHandle;
                this->uvHandle = nullptr;

                LError("uv_tcp_init() failed: %s", uv_strerror(err));
            }

            // Set the listener.
            // this->listener = listener;

            // Set the local address.

            this->localIp = ip;
            this->localPort = port;


            //int r;

            auto req = new uv_connect_t();
            req->data = this;

            if (!addrs) {
                if (IP::GetFamily(ip) == AF_INET6) {
                    ASSERT(0 == uv_ip6_addr(ip.c_str(), port, &addr6));

                    // this->localAddr = (sockaddr_storage *) addr6;
                    err = uv_tcp_connect(req, this->uvHandle, reinterpret_cast<struct sockaddr*> (&addr6), static_cast<uv_connect_cb> (onconnect));


                } else {
                    ASSERT(0 == uv_ip4_addr(ip.c_str(), port, &addr));

                    err = uv_tcp_connect(req, this->uvHandle, reinterpret_cast<struct sockaddr*> (&addr), static_cast<uv_connect_cb> (onconnect));

                }
            } else {

                for (addrinfo* ai = addrs; ai != NULL; ai = ai->ai_next) {
                    if (ai->ai_family != AF_INET && ai->ai_family != AF_INET6) {
                        continue;
                    }
                    if (ai->ai_family == AF_INET6) {
                        addr6 = *(const struct sockaddr_in6 *) ai->ai_addr;
                        addr6.sin6_port = htons(port);
                        err = uv_tcp_connect(req, this->uvHandle, reinterpret_cast<struct sockaddr*> (&addr6), static_cast<uv_connect_cb> (onconnect));
                        if (!err) break;
                        //addrv = &s.addr4.sin_addr;
                    } else if (ai->ai_family == AF_INET) {
                        addr = *(const struct sockaddr_in *) ai->ai_addr;
                        addr.sin_port = htons(port);
                        err = uv_tcp_connect(req, this->uvHandle, reinterpret_cast<struct sockaddr*> (&addr), static_cast<uv_connect_cb> (onconnect));
                        if (!err) break;
                        // addrv = &s.addr6.sin6_addr;
                    }
                }

            }


            if (err != 0) {
                uv_close(reinterpret_cast<uv_handle_t*> (this->uvHandle), static_cast<uv_close_cb> (onClose));

                LError("uv_tcp_connect() failed: %s", uv_strerror(err));

                LError("uv_tcp_connect() failed for ip:port ", ip, ":", port);
            }
            ////////////////////////////////////////////



        }

        void TcpConnectionBase::Start() {


            if (this->closed)
                return;

            int err = uv_read_start(
                    reinterpret_cast<uv_stream_t*> (this->uvHandle),
                    static_cast<uv_alloc_cb> (onAlloc),
                    static_cast<uv_read_cb> (onRead));

            if (err != 0)
                LError("uv_read_start() failed: %s", uv_strerror(err));

            // Get the peer address.
            if (!SetPeerAddress())
                LError("error setting peer IP and port");
        }

        int TcpConnectionBase::Write(const char* data, size_t len, TcpConnectionBase::onSendCallback cb) {

            if (this->closed)
            {
                if (cb)
                    (cb)(false);
                return -1;
            }

            if (len == 0)
            {
                if (cb)
                    (cb)(false);
                return 0;
            }

            // LTrace("TcpConnectionBase::Write " , len);

            // First try uv_try_write(). In case it can not directly write all the given
            // data then build a uv_req_t and use uv_write().

            uv_buf_t buffer = uv_buf_init(reinterpret_cast<char*> (const_cast<char*> (data)), len);
            int written = uv_try_write(reinterpret_cast<uv_stream_t*> (this->uvHandle), &buffer, 1);

            if (written > 0)
                sentBytes += written;

            // All the data was written. Done.
            if (written == static_cast<int> (len)) {
                if (cb)
                    (cb)(true);
                return written;
            }// Cannot write any data at first time. Use uv_write().
            else if (written == UV_EAGAIN || written == UV_ENOSYS) {
                // Set written to 0 so pendingLen can be properly calculated.
                written = 0;
            }// Error. Should not happen.
            else if (written < 0) {
                //LDebug("uv_try_write() failed, closing the connection: %s", uv_strerror(written));
                LDebug("uv_try_write() failed, trying uv_write(): %s", uv_strerror(written));

                //Close(); // arvind TBD .. I am not sure if I should close connection here
                //return -1;
                written = 0;
            }

            // LDebug(
            // 	"could just write %zu bytes (%zu given) at first time, using uv_write() now",
            // 	static_cast<size_t>(written), len);

            size_t pendingLen = len - written;
            // Allocate a special UvWriteData struct pointer.
            auto* writeData = new UvWriteData(pendingLen);

            writeData->req.data = static_cast<void*>(writeData);
            std::memcpy(writeData->store, data + written, pendingLen);
            writeData->cb = cb;

            buffer = uv_buf_init(reinterpret_cast<char*>(writeData->store), pendingLen);

            int err = uv_write(
              &writeData->req,
              reinterpret_cast<uv_stream_t*>(this->uvHandle),
              &buffer,
              1,
              static_cast<uv_write_cb>(onWrite));

            if (err != 0)
            {
                LError("uv_write() failed: %s", uv_strerror(err));

                if (cb)
                    (cb)(false);

                // Delete the UvWriteData struct (it will delete the store and cb too).
                delete writeData;
                return -1;
            }
            else
            {
                // Update sent bytes.
                this->sentBytes += pendingLen;
            }


            return pendingLen;
        }

        int TcpConnectionBase::Write(const char* data1, size_t len1, const char* data2, size_t len2, TcpConnection::onSendCallback cb) {

            if (this->closed)
            {
                if (cb)
                    (cb)(false);
                return -1;
            }

            if (len1 == 0 && len2 == 0)
            {
                if (cb)
                    (cb)(false);
                return 0;
            }

            size_t totalLen = len1 + len2;
            uv_buf_t buffers[2];
            int written;
            int err;

            // First try uv_try_write(). In case it can not directly write all the given
            // data then build a uv_req_t and use uv_write().

            buffers[0] = uv_buf_init(reinterpret_cast<char*> (const_cast<char*> (data1)), len1);
            buffers[1] = uv_buf_init(reinterpret_cast<char*> (const_cast<char*> (data2)), len2);
            written = uv_try_write(reinterpret_cast<uv_stream_t*> (this->uvHandle), buffers, 2);

            if (written > 0)
                sentBytes += written;

            // All the data was written. Done.
            if (written == static_cast<int> (totalLen)) {
                 if (cb)
                    (cb)(true);
                return written;
            }// Cannot write any data at first time. Use uv_write().
            else if (written == UV_EAGAIN || written == UV_ENOSYS) {
                // Set written to 0 so pendingLen can be properly calculated.
                written = 0;
            }// Error. Should not happen.
            else if (written < 0) {
                 //LDebug("uv_try_write() failed, closing the connection: %s", uv_strerror(written));
                LDebug("uv_try_write() failed, trying uv_write(): %s", uv_strerror(written));

                //Close(); // arvind TBD .. I am not sure if I should close connection here
                //return -1;
                written = 0;
            }

            size_t pendingLen = totalLen - written;
            auto* writeData   = new UvWriteData(pendingLen);

            writeData->req.data = static_cast<void*>(writeData);

            // If the first buffer was not entirely written then splice it.
            if (static_cast<size_t>(written) < len1)
            {
                std::memcpy(
                  writeData->store, data1 + static_cast<size_t>(written), len1 - static_cast<size_t>(written));
                std::memcpy(writeData->store + (len1 - static_cast<size_t>(written)), data2, len2);
            }
            // Otherwise just take the pending data in the second buffer.
            else
            {
                std::memcpy(
                  writeData->store,
                  data2 + (static_cast<size_t>(written) - len1),
                  len2 - (static_cast<size_t>(written) - len1));
            }

            writeData->cb = cb;

            uv_buf_t buffer = uv_buf_init(reinterpret_cast<char*>(writeData->store), pendingLen);

            err = uv_write(
              &writeData->req,
              reinterpret_cast<uv_stream_t*>(this->uvHandle),
              &buffer,
              1,
              static_cast<uv_write_cb>(onWrite));

            if (err != 0)
            {
                LError("uv_write() failed: %s", uv_strerror(err));

                if (cb)
                    (cb)(false);

                // Delete the UvWriteData struct (it will delete the store and cb too).
                delete writeData;
                return -1;
            }
            else
            {
                // Update sent bytes.
                this->sentBytes += pendingLen;
            }

             return pendingLen;

        }

        void TcpConnectionBase::ErrorReceiving() {
            Close();
        }

        bool TcpConnectionBase::SetPeerAddress() {


            int err;
            int len = sizeof (this->peerAddr);

            err = uv_tcp_getpeername(this->uvHandle, reinterpret_cast<struct sockaddr*> (&this->peerAddr), &len);

            if (err != 0) {
                LError("uv_tcp_getpeername() failed: %s", uv_strerror(err));

                return false;
            }

            int family;

            IP::GetAddressInfo(
                    reinterpret_cast<struct sockaddr*> (&this->peerAddr), family, this->peerIp, this->peerPort);

            LTrace("PeerIP ", this->peerIp, ":", this->peerPort)
            return true;
        }

        inline void TcpConnectionBase::OnUvReadAlloc(size_t suggested_size, uv_buf_t* buf) {


            if (this->closed)
                return;
            //             static char slab[65536];
            //            assert(suggested_size <= sizeof(slab));
            //            buf->base = slab;
            //            buf->len = sizeof(slab);

            // If this is the first call to onUvReadAlloc() then allocate the receiving buffer now.
            if (this->buffer == nullptr)
                this->buffer = new char[this->bufferSize];

            // Tell UV to write after the last data byte in the buffer.
            buf->base = reinterpret_cast<char*> (this->buffer + this->bufferDataLen);

            // Give UV all the remaining space in the buffer.
            if (this->bufferSize > this->bufferDataLen) {
                buf->len = this->bufferSize - this->bufferDataLen;
            } else {
                buf->len = 0;

                LDebug("no available space in the buffer");
            }

        }

        inline void TcpConnectionBase::OnUvRead(ssize_t nread, const uv_buf_t* buf) {
            // LTrace("OnUvRead" )

            if (this->closed)
                return;

            if (nread == 0)
                return;

            // Data received.
            if (nread > 0) {
                // Update the buffer data length.
                // this->bufferDataLen += static_cast<size_t> (nread);
                recvBytes += nread;

                // Notify the subclass.
                if (tls)
                    on_tls_read((const char*) buf->base, nread);
                else
                    on_read((const char*) buf->base, nread);

            }// Client disconneted.
            else if (nread == UV_EOF || nread == UV_ECONNRESET) {
                LDebug("connection closed by peer, closing server side");

                this->isClosedByPeer = true;

                // Close server side of the connection.
                Close();

            }// Some error.
            else {
                LDebug("read error, closing the connection: %s", uv_strerror(nread));

                this->hasError = true;

                // Close server side of the connection.
                Close();

            }
        }

        inline void TcpConnectionBase::OnUvWrite(int status,onSendCallback cb) {

            if (status == 0) {
                if (cb)
                 (cb)(true);
            } else {
                if (status != UV_EPIPE && status != UV_ENOTCONN)
                    this->hasError = true;
                    
                if (cb)
                  (cb)(false);

                LDebug("write error, closing the connection: %s", uv_strerror(status));
                Close();
            }
        }
        
        
        void TcpConnectionBase::send(const char* data, size_t len) {

           Write(data, len, nullptr);
        }

        /*************************************************************************************************************/
        // TcpConnection class is for RFC 4571 for RTP transport. Please do not use it other than SFU/MCU.
        
        static constexpr size_t ReadBufferSize{ 65536 };
	    static uint8_t ReadBuffer[ReadBufferSize];

        TcpConnection::TcpConnection(Listener* listener, bool tls)
        : TcpConnectionBase(tls), listener(listener) {

        }

        TcpConnection::~TcpConnection() {

        }

        void TcpConnection::on_read(const char* data, size_t len) {

            /*
             * Framing RFC 4571
             *
             *     0                   1                   2                   3
             *     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
             *     ---------------------------------------------------------------
             *     |             LENGTH            |  packet  |
             *     ---------------------------------------------------------------
             *
             */

            bufferDataLen += len;
            // Be ready to parse more than a single frame in a single TCP chunk.
            while (true) {
                // We may receive multiple packets in the same TCP chunk. If one of them is
                // a DTLS Close Alert this would be closed (Close() called) so we cannot call
                // our listeners anymore.
                if (IsClosed())
                    return;

                size_t dataLen = bufferDataLen - frameStart;
                size_t packetLen;

                if (dataLen >= 2)
                    packetLen = size_t{base::util::Byte::Get2Bytes((const uint8_t*) buffer + frameStart, 0)};

                // We have packetLen bytes.
                if (dataLen >= 2 && dataLen >= 2 + packetLen) {
                    const char* packet = buffer + frameStart + 2;

                    // Update received bytes and notify the listener.
                    if (packetLen != 0) {
                        // Copy the received packet into the static buffer so it can be expanded
                        // later.
                        std::memcpy(ReadBuffer, packet, packetLen);

                        listener->on_read(this, (const char*) ReadBuffer, packetLen); //arvind
                      //   listener->on_read(this, data, len);
                    }

                    // If there is no more space available in the buffer and that is because
                    // the latest parsed frame filled it, then empty the full buffer.
                    if ((frameStart + 2 + packetLen) == bufferSize) {
                        LTrace("no more space in the buffer, emptying the buffer data");

                        frameStart = 0;
                        bufferDataLen = 0;
                    }                        // If there is still space in the buffer, set the beginning of the next
                        // frame to the next position after the parsed frame.
                    else {
                        frameStart += 2 + packetLen;
                    }

                    // If there is more data in the buffer after the parsed frame then
                    // parse again. Otherwise break here and wait for more data.
                    if (bufferDataLen > frameStart) {
                        LTrace("there is more data after the parsed frame, continue parsing");

                        continue;
                    }

                    break;
                }

                // Incomplete packet.

                // Check if the buffer is full.
                if (bufferDataLen == bufferSize) {
                    // First case: the incomplete frame does not begin at position 0 of
                    // the buffer, so move the frame to the position 0.
                    if (frameStart != 0) {
                        LTrace(
                                "no more space in the buffer, moving parsed bytes to the beginning of "
                                "the buffer and wait for more data");

                        std::memmove(
                                buffer, buffer + frameStart, bufferSize - frameStart);
                        bufferDataLen = bufferSize - frameStart;
                        frameStart = 0;
                    }                        // Second case: the incomplete frame begins at position 0 of the buffer.
                        // The frame is too big, so close the connection.
                    else {
                        LWarn(
                                "no more space in the buffer for the unfinished frame being parsed, closing the "
                                "connection");

                        // Close the socket.
                        ErrorReceiving();

                        // And exit fast since we are supposed to be deallocated.
                        return;
                    }
                }                    // The buffer is not full.
                else {
                    LTrace("frame not finished yet, waiting for more data");
                }

                // Exit the parsing loop.
                break;
            }

           
        }

        void TcpConnection::on_close() {

            listener->on_close(this);
        }

        void TcpConnection::send(const char* data, size_t len) {


            // Update sent bytes.

            // Write according to Framing RFC 4571.

            uint8_t frameLen[2];

            base::util::Byte::Set2Bytes(frameLen, 0, len);
            TcpConnectionBase::Write((const char*)frameLen, 2, data, len,nullptr);
           //TcpConnectionBase::Write(data, len);
        }





        /**************************************************************************************************************/

    } // namespace net
} // namespace base
