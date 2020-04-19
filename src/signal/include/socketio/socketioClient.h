/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */


#ifndef SocketIO_Client_H
#define SocketIO_Client_H


#include "socketio/packet.h"

#include "net/netInterface.h"
#include "http/websocket.h"
#include "json/json.h"

#include "base/logger.h"

#include "base/Timer.h"
#include "base/collection.h"
#include <exception>



#include "http/client.h"
#include "base/logger.h"
#include "base/application.h"
#include "base/platform.h"
#include "http/HttpClient.h"
//#include "http/HttpsClient.h"

namespace base {
    namespace sockio {

           /****************************************************************************/

        class event {
        public:
            const std::string& get_nsp() const;
            const std::string& get_name() const;
            const json& get_message() const;

            bool need_ack() const;

            void put_ack_message(json const& ack_message);

            json const& get_ack_message() const;
            json& get_ack_message_impl();

        protected:
            event(std::string const& nsp, std::string const& name, json const& messages, bool need_ack);
            event(std::string const& nsp,std::string const& name, json&& messages,bool need_ack);

       
        private:
            const std::string m_nsp;
            const std::string m_name;
            const json m_messages;
            const bool m_need_ack;
            json m_ack_message;

            friend class event_adapter;
        };


        class Socket;

        class SocketioClient  {
        public:

            enum con_state
            {
                con_opening,
                con_opened,
                con_closing,
                con_closed
            };
            SocketioClient(const std::string& host, uint16_t port, bool ssl=false);
            virtual ~SocketioClient();

            virtual void connect(const std::string& host, uint16_t port);
            virtual void connect();
            // virtual void close();


            std::map<const std::string, Socket*> m_sockets;
           
            void remove_socket(string const& nsp);
         
            std::mutex m_socket_mutex;

            void close(int const& code, string const& reason);
            
             void ping();
             void timeout_pong();
             void timeout_reconnect();
             con_state m_con_state;
              void on_close();
         
            //virtual const char* className() const { return "SocketIOClient"; }
            std::string const& get_sessionid();
            // void on_message_packet(packet const& p);
            std::function<void(Socket* sock) > cbConnected;

            void send(packet& p);

            Socket* io(string const& nsp="");

        protected:
            // virtual void setError(const Errors& error);

            virtual void reset();

            virtual void sendHandshakeRequest();

            void on_decode(packet const& pack);
            void on_encode(bool isBinary, shared_ptr<const string> const& payload);

            void on_handshake(json const& message);

            void on_pong();

            void clear_timers();
            
            bool ssl{false};    

        protected:
            Socket* get_socket(string const& nsp);
            std::string _host;
            uint16_t _port;
    
            Timer m_ping_timer{ nullptr};
            Timer m_ping_timeout_timer{ nullptr};
            Timer m_reconn_timer{ nullptr};


            std::string m_sid;
            net::ClientConnecton *m_client;

            unsigned int m_ping_interval;
            unsigned int m_ping_timeout;
            packet_manager m_packet_mgr;
        };


      


        /********************************************************/
        class packet;

        //The name 'Socket' is taken from concept of official Socket.io.

        class Socket {
        public:
            typedef std::function<void(const std::string& name, json const& message, bool need_ack, json& ack_message) > event_listener_aux;

           // typedef std::function<void(event& event) > event_listener;

            typedef std::function<void(json const& message) > error_listener;

            //typedef std::shared_ptr<Socket> ptr;

            ~Socket();

           // void on(std::string const& event_name, event_listener const& func);

            void on(std::string const& event_name, event_listener_aux const& func);

            void off(std::string const& event_name);

            void off_all();

            void close();

            void on_error(error_listener const& l);

            void off_error();

            std::map<unsigned int, std::function<void (json const&) > > m_acks;
            //void emit(std::string const& name, std::string const& msglist = "", std::function<void (json const&)> const& ack = nullptr);
            
            void emit(std::string const& name, json const& msglist = nullptr, std::function<void (json const&)> const& ack = nullptr);

            void on_socketio_event(const std::string& nsp, int msgId, const std::string& name, json && message);
            
            void ack(int msgId,string const& name,json const& ack_message);
            void on_socketio_ack(int msgId, json const& message);

            event_listener_aux get_bind_listener_locked(const string &event);

            std::string const& get_namespace() const;

            Socket(SocketioClient*, std::string const&);

            std::map<std::string, event_listener_aux> m_event_binding;
        protected:

            void on_connected();
           
            void timeout_connection();

            error_listener m_error_listener;

            void on_close();

            void on_open();

            void on_disconnect();

            void on_message_packet(packet const& p);
            void on_socketio_error(json const& err_message);
            void send_connect(const std::string & nsp);
            void send_packet(packet &p);


            friend class SocketioClient;

        private:
            //disable copy constructor and assign operator.

            Socket(Socket const&) {
            }

            void operator=(Socket const&) {
            }

            const std::string m_nsp;

            SocketioClient *m_client;

            Timer m_connection_timer{ nullptr};
            bool m_connected{ false};
            std::queue<packet> m_packet_queue;

            std::mutex m_packet_mutex;
            std::mutex m_event_mutex;
        };

    }
} // namespace scy::sockio


#endif //  SocketIO_Client_H



