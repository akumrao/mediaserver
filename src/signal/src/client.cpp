/*
  The following events are reserved and should not be used as event names by your application:
 error
connect
disconnect
disconnecting
newListener
removeListener
ping
pong
 
 */

#include "socketio/client.h"
#include "http/client.h"
#include <stdexcept>
using namespace std::placeholders;
#include <functional>

static unsigned int s_global_event_id = 1;

using std::endl;
using namespace base::net;

namespace base {

    namespace sockio {

        class event_adapter {
        public:

            static void adapt_func(socket::event_listener_aux const& func, event& event) {
                func(event.get_name(), event.get_message(), event.need_ack(), event.get_ack_message_impl());
            }

            static inline socket::event_listener do_adapt(socket::event_listener_aux const& func) {
                return std::bind(&event_adapter::adapt_func, func, std::placeholders::_1);
            }

            static inline event create_event(std::string const& nsp, std::string const& name, json&& message, bool need_ack) {
                return event(nsp, name, message, need_ack);
            }
        };

        const std::string& event::get_nsp() const {
            return m_nsp;
        }

        const std::string& event::get_name() const {
            return m_name;
        }

        const json& event::get_message() const {
         /*   if (m_messages.size() > 0) {
                   return m_messages;
            } else {
                static json null_ptr;
                return null_ptr;
            }
           */ 
            return m_messages;
        }

        bool event::need_ack() const {
            return m_need_ack;
        }

        void event::put_ack_message(json const& ack_message) {
            if (m_need_ack)
                m_ack_message = std::move(ack_message);
        }

        inline
        event::event(std::string const& nsp, std::string const& name, json const& messages, bool need_ack) :
        m_nsp(nsp),
        m_name(name),
        m_messages(messages),
        m_need_ack(need_ack) {
        }

        json const& event::get_ack_message() const {
            return m_ack_message;
        }

        inline
        json& event::get_ack_message_impl() {
            return m_ack_message;
        }

        inline
        event::event(std::string const& nsp, std::string const& name, json && messages, bool need_ack) :
        m_nsp(nsp),
        m_name(name),
        m_messages(std::move(messages)),
        m_need_ack(need_ack) {
        }

        Client::Client(const std::string& host, uint16_t port) : //, uv::Loop* loop
        _host(host),
        _port(port),
        m_ping_interval(0),
        m_ping_timeout(0)
        {
            // arvind
            //_ws.addReceiver(this);
            m_packet_mgr.set_decode_callback(bind(&Client::on_decode, this, _1));
            m_packet_mgr.set_encode_callback(bind(&Client::on_encode, this, _1, _2));
        }

        void Client::on_handshake(json const& message) {

            LTrace("on_handshake")
            if (message.is_object()) {

                try {
                    m_sid = message["sid"];
                    LTrace(m_sid)
                } catch (...) {
                    LError("Handshake error")
                    m_client->Close();
                }

                try {

                    m_ping_interval = message["pingInterval"];
                    LTrace("Ping Interval ", m_ping_interval)
                } catch (...) {
                    m_ping_interval = 25000;
                }

                try {

                    m_ping_timeout = message["pingTimeout"];
                    LTrace("Ping timeout ", m_ping_timeout)
                } catch (...) {
                    m_ping_timeout = 60000;
                }

            }

            m_ping_timer.cb_timeout = std::bind(&Client::ping, this);
            m_ping_timer.Start(m_ping_interval,m_ping_interval);
            m_ping_timeout_timer.cb_timeout = std::bind(&Client::timeout_pong, this);
   
        }

        /////////////////////////////////////////////////////////////////////

        void Client::ping() {
            STrace << "ping " ;

                  packet p(packet::frame_ping);
            m_packet_mgr.encode(p, [&](bool /*isBin*/, shared_ptr<const string> payload) {
              
                m_client->send( *payload);
            });
            
             m_ping_timeout_timer.Start(m_ping_timeout);
        }

        void Client::timeout_pong() {
            STrace << "timeout pong " ;

            close_impl(1, "Pong timeout");
        }

        void Client::timeout_reconnect() {
            STrace << "timeout=reconnect" ;
            
            if (m_con_state == con_closed) {
                m_con_state = con_opening;
                m_client->Close();
                delete m_client;
                m_sid.clear();
                m_packet_mgr.reset();
  
                LTrace("Reconnecting...");
                connect();
                //if (m_reconnecting_listener) m_reconnecting_listener();
               // m_client.get_io_service().dispatch(lib::bind(&client_impl::connect_impl, this, m_base_url, m_query_string));
            }
        }
        
        ////////////////////////////////////////////////////////////////////
        
        void Client::close_impl(int const& code, string const& reason) {
            LTrace("Close by reason:", reason);

            m_reconn_timer.Stop();
            m_reconn_timer.Close();
            m_client->Close();

        }
        
        void Client::on_close()
        {
            LTrace("Client Disconnected.");

            m_con_state = con_closed;
            this->clear_timers();

        }
          socket* Client::io(string const& nsp)
        {
           socket *soc = get_socket( nsp);
           soc->send_connect();
           return soc;
        }

        socket* Client::get_socket(string const& nsp) {
            
            string aux;
            if (nsp == "") {
                aux = "/";
            } else if (nsp[0] != '/') {
                aux.append("/", 1);
                aux.append(nsp);
            } else {
                aux = nsp;
            }


            auto it = m_sockets.find(aux);
            if (it != m_sockets.end()) {
                return it->second;
            } else {
                lock_guard<mutex> guard(m_socket_mutex);
                m_sockets[aux] = new socket(this, aux);
                return m_sockets[aux];
            }
        }

        void Client::on_decode(packet const& p) {
            STrace << "on_decode" ;

            switch (p.get_frame()) {
                case packet::frame_message:
                {
                    socket *so_ptr = get_socket(p.get_nsp());
                    if (so_ptr)so_ptr->on_message_packet(p);
                    break;
                }
                case packet::frame_open:
                    this->on_handshake(p.get_message());
                    break;
                case packet::frame_close:
                    //FIXME how to deal?
                    close_impl(0, "End by server");
                    break;
                case packet::frame_pong:
                    on_pong();
                    break;

                default:
                    break;
            }
        }

        void Client::clear_timers() {
            LTrace("clear timers")

            m_ping_timeout_timer.Stop();
            m_ping_timer.Stop();
            m_ping_timeout_timer.Close();
            m_ping_timer.Close();

        }

        void Client::on_pong() {
            LTrace("on_pong")
            m_ping_timeout_timer.Stop();
        }

        void Client::on_encode(bool isBinary, shared_ptr<const string> const& payload) {
            STrace << "on_encode" << payload ;

            if (!isBinary) {
                LTrace("on_encode ", *payload)
                m_client->send(*payload);
            } else {
                LTrace("on_encode:Binary send is still pending to do ")
            }
        }

        Client::~Client() {

            //reset();
        }

        void Client::connect(const std::string& host, uint16_t port) {
            {
                _host = host;
                _port = port;
            }
            connect();
        }

        void Client::connect() {
            STrace << "SocketIO Connecting" ;
            m_con_state = con_opening;
            if (_host.empty() || !_port)
                throw std::runtime_error("The SocketIO server address is not set.");

            reset();

            sendHandshakeRequest();
        }

        std::string const& Client::get_sessionid() {
            return m_sid;
        }

        void Client::sendHandshakeRequest() {
            LTrace("sendHandshakeRequest")

            //TraceL << "Send handshake request" << endl;	
            bool ssl = false;
            std::ostringstream url;
            //url << (ssl ? "https://" : "http://")	<< _host << ":" << _port << "/socket.io/1/";

            // url << (ssl ? "wss://" : "ws://")	<< _host << ":" << _port << "/socket.io/?EIO=3&transport=websocket";
            url << "/socket.io/?EIO=4&transport=websocket";

            if (m_sid.size() > 0) {
                url << "&sid=" << m_sid;
            }
            url << "&t=" << Application::GetTime();

            m_client = new HttpClient("ws", _host, _port, url.str());

            // conn->Complete += sdelegate(&context, &CallbackContext::onClientConnectionComplete);
            m_client->fnComplete = [&](const Response & response) {
                std::string reason = response.getReason();
                StatusCode statuscode = response.getStatus();
                std::string body = m_client->readStream()->str();
                STrace << "SocketIO handshake response:" << "Reason: " << reason << " Response: " << body;
            };

            m_client->fnPayload = [&](HttpBase * con, const char* data, size_t sz) {
                STrace << "client->fnPayload" << std::string(data,sz) ;
                m_ping_timeout_timer.Reset();
                m_packet_mgr.put_payload(std::string(data,sz));
            };

            //  conn->_request.setKeepAlive(false);
            m_client->setReadStream(new std::stringstream);
            m_client->send();
            LTrace("sendHandshakeRequest over")

        }

        void Client::reset() {
            m_sid.clear();
            //Mutex::ScopedLock lock(_mutex);
            // Note: Only reset session related variables here.
            // Do not reset host and port variables.
            //_timer.Timeout -= sdelegate(this, &Client::onHeartBeatTimer);
            m_ping_timer.Stop();
            m_ping_timeout_timer.Stop();
            m_reconn_timer.Stop();
      
        }

    
        void Client::send(packet& p) {
            STrace << "send " ;
            m_packet_mgr.encode(p);
        }

        
        void Client::remove_socket(string const& nsp)
        {
            STrace << "remove_socket "  << nsp ;
            lock_guard<mutex> guard(m_socket_mutex);
            auto it = m_sockets.find(nsp);
            if(it!= m_sockets.end())
            {
                m_sockets.erase(it);
                delete it->second;
            }
        }
        
        /***************************************************************************/
        socket::socket(Client* client, std::string const& nsp) : m_client(client), m_nsp(nsp) {
        }

        socket::~socket() {

        }

        void socket::on(std::string const& event_name, event_listener const& func) {
            m_event_binding[event_name] = func;

        }

        void socket::on(std::string const& event_name, event_listener_aux const& func) {
            on(event_name, event_adapter::do_adapt(func));

        }

        void socket::off(std::string const& event_name) {
            auto it = m_event_binding.find(event_name);
            if (it != m_event_binding.end()) {
                m_event_binding.erase(it);
            }

        }

        void socket::off_all() {
            m_event_binding.clear();

        }

        void socket::close() {

            if(m_connected)
            {
                packet p(packet::type_disconnect,m_nsp);
                send_packet(p);
                m_connection_timer.cb_timeout = std::bind(&socket::on_close, this);
                m_connection_timer.Start( 3000, 0);
                
            }
              

        }

        void socket::on_error(error_listener const& l) {

            STrace << "on_error " ;
            m_error_listener = l;
        }

        void socket::off_error() {
            STrace << "off_error " ;
            m_error_listener = nullptr;
        }

        std::string const& socket::get_namespace() const {
            return m_nsp;
        }

        void socket::on_close() {
            m_connection_timer.Stop();
            m_connection_timer.Close();
            m_connected = false;
            {
                std::lock_guard<std::mutex> guard(m_packet_mutex);
                while (!m_packet_queue.empty()) {
                    m_packet_queue.pop();
                }
            }
            
            // m_client->on_socket_closed(m_nsp);
             m_client->remove_socket(m_nsp);

            //  m_client->Close();
            //delete m_client;
            //m_client = NULL;
        }

        void socket::on_open() {

        }

        void socket::on_message_packet(packet const& p) {

            if (p.get_nsp() == m_nsp) {
                switch (p.get_type()) {
                        // Connect open
                    case packet::type_connect:
                    {
                        LTrace("Received Message type (Connect)");

                        on_connected();
                        break;
                    }
                    case packet::type_disconnect:
                    {
                        LTrace("Received Message type (Disconnect)");
                        on_close();
                        break;
                    }
                    case packet::type_event:
                    case packet::type_binary_event:
                    {
                        LTrace("Received Message type (Event)");
                        const json ptr = p.get_message();
                        std::string str = cnfg::stringify(ptr);
                        json value ;
                        
                        if( ptr.is_array())
                        {
                        
                            
                            if( ptr.size() == 2)
                            {
                               value = ptr.at(1);
                            }
                            else
                            {
                                auto array = json::array();
                                for (int i=1; i< ptr.size() ; ++i)
                                {   
                                    array.push_back( ptr.at(i));
                                    //std::cout << "key: " << x.key() << ", value: " << x.value() << '\n';
                                }
                                value=array;
                            }
                                
                        }
                            
                        else
                        {
                            LTrace("Event is not array")
                            return;
                        }
                        
                        json name = ptr.at(0);
                        LTrace(str);
                        on_socketio_event(p.get_nsp(), p.get_pack_id(), name, std::move(value));

                        break;
                    }
                        // Ack
                    case packet::type_ack:
                    case packet::type_binary_ack:
                    {
                         LTrace("Received Message type (ACK)");
                         const json ptr = p.get_message();
                                                           
                         on_socketio_ack(p.get_pack_id(),ptr);
                    
                        break;
                    }
                        // Error
                    case packet::type_error:
                    {
                        LTrace("Received Message type (ERROR)");
                        this->on_socketio_error(p.get_message());
                        break;
                    }
                    default:
                        break;
                }
            }
        }

        void socket::on_disconnect() {

        }

       

        void socket::send_connect() {
            STrace << "send_connect " << m_nsp ;

            if (m_nsp == "/" || m_nsp == "") {
                return;
            }
            packet p(packet::type_connect, m_nsp);
            m_client->send(p);
            
            m_connection_timer.cb_timeout = std::bind(&socket::timeout_connection, this);
            
            m_connection_timer.Start(20000);

        }

         void socket::timeout_connection()
        {
            STrace << "timeout_connection "  ;
            m_connection_timer.Reset();
            //Should close socket if no connected message arrive.Otherwise we'll never ask for open again.
            this->on_close();
        }
         
        void socket::on_connected() {

            STrace << "on_connected " << m_nsp ;

            m_connection_timer.Stop();

            if (!m_connected) {
                m_connected = true;
                //m_client->on_socket_opened(m_nsp);

                while (true) {
                    m_packet_mutex.lock();
                    if (m_packet_queue.empty()) {
                        m_packet_mutex.unlock();
                        break;
                    }
                    packet front_pack = std::move(m_packet_queue.front());
                    m_packet_queue.pop();
                    m_packet_mutex.unlock();
                    m_client->send(front_pack);
                }
                m_client->cbConnected(this);
            }
        }

        void socket::on_socketio_error(json const& err_message) {
            STrace << "on_sokcetio_error " ;
            if (m_error_listener)m_error_listener(err_message);
        }

        /****************************************************************************/

        void socket::emit(std::string const& name, std::string const& msglist, std::function<void (json const&) > const& ack) {
            STrace << "emit " << name ;
            
            
            const char *ptr1 = m_nsp.c_str();
             int ss = m_nsp.length();
             LTrace("m_nsp: ", m_nsp )
            
            auto array = json::array();
            array.push_back(name);
            
            if(!msglist.empty())
            array.push_back(msglist);

            int pack_id;
            if (ack) {
                pack_id = s_global_event_id++;
                 std::lock_guard<std::mutex> guard(m_event_mutex);
                m_acks[pack_id] = ack;
            } else {
                pack_id = -1;
            }
            
            const char *ptr1 = m_nsp.c_str();
             int ss = m_nsp.length();
             LTrace("m_nsp: ", m_nsp )
            LTrace("emit: ", cnfg::stringify(array ))
           
            
            
            packet p(m_nsp, array, pack_id);
            send_packet(p);
        }

        void socket::send_packet(packet &p) {
            STrace << "send_packet" ;

            if (m_connected) {
                while (true) {
                    m_packet_mutex.lock();
                    if (m_packet_queue.empty()) {
                        m_packet_mutex.unlock();
                        break;
                    }
                    packet front_pack = std::move(m_packet_queue.front());
                    m_packet_queue.pop();
                    m_packet_mutex.unlock();
                    m_client->send(front_pack);
                }
                m_client->send(p);
            } else {
                std::lock_guard<std::mutex> guard(m_packet_mutex);
                m_packet_queue.push(p);
            }
        }

        void socket::on_socketio_event(const std::string& nsp, int msgId, const std::string& name, json && message) {

            STrace << "on_socketio_event " << name ;
            bool needAck = msgId >= 0;
            event ev = event_adapter::create_event(nsp, name, std::move(message), needAck);
            event_listener func = this->get_bind_listener_locked(name);
            if (func)func(ev);
            if (needAck)//arvind
            {
                this->ack(msgId, name, ev.get_ack_message());
            }
        }

        socket::event_listener socket::get_bind_listener_locked(const string &event) {
             std::lock_guard<std::mutex> guard(m_event_mutex);
            auto it = m_event_binding.find(event);
            if (it != m_event_binding.end()) {
                return it->second;
            }
            return socket::event_listener();
        }

        void socket::ack(int msgId, const string &, const json &ack_message) {
            STrace << "ack " << msgId ;

            packet p(m_nsp, ack_message, msgId, true);
            send_packet(p);
        }
        
        
        //json array is equivalent to message::list
        void socket::on_socketio_ack(int msgId, json const& message)
        {
            STrace << "on_socketio_ack "  << msgId ;
            std::function<void (json const&)> l;
            {
                std::lock_guard<std::mutex> guard(m_event_mutex);
                auto it = m_acks.find(msgId);
                if(it!=m_acks.end())
                {
                    l = it->second;
                    m_acks.erase(it);
                }
            }
            if(l)l(message);
        }

    }
} // namespace scy::sockio
