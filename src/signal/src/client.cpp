

#include "socketio/client.h"
#include "net/tcpsocket.h"
#include "net/sslsocket.h"
#include "http/client.h"
#include <stdexcept>


using std::endl;


namespace base {
namespace sockio {


//
// TCP Client
//


Client* createTCPClient(uv::Loop* loop)
{
	return new Client(std::make_shared<net::TCPSocket>(loop)); //, loop
}


TCPClient::TCPClient(uv::Loop* loop) :
	Client(std::make_shared<net::TCPSocket>(loop)) //, loop
{
}


//
// SSL Client
//
	

Client* createSSLClient(uv::Loop* loop)
{
	return new Client(std::make_shared<net::SSLSocket>(loop)); //, loop);
}


SSLClient::SSLClient(uv::Loop* loop) :
	Client(std::make_shared<net::SSLSocket>(loop)) //, loop)
{
}


Client::Client(const net::Socket::Ptr& socket) : //, uv::Loop* loop
	//net::SocketAdapter(nullptr, this),
	_timer(socket->loop()),
	_ws(socket),
	//_loop(loop),
	_wasOnline(false)
{
	_ws.addReceiver(this);
}


Client::Client(const net::Socket::Ptr& socket, const std::string& host, UInt16 port) : //, uv::Loop* loop
	//net::SocketAdapter(nullptr, this),
	_timer(socket->loop()),
	_host(host),
	_port(port),
	_ws(socket),
	//_loop(loop),
	_wasOnline(false)
{
	_ws.addReceiver(this);
}


Client::~Client() 
{
	_ws.removeReceiver(this);
	//_ws.remove(this);
	//_ws.adapter = nullptr;
	close();
	//reset();
}


void Client::connect(const std::string& host, UInt16 port)
{	
	{
		//Mutex::ScopedLock lock(_mutex);
		_host = host;
		_port = port;
	}
	connect();
}


void Client::connect()
{
	TraceL << "SocketIO Connecting" << endl;

	if (_host.empty() || !_port)
		throw std::runtime_error("The SocketIO server address is not set.");

	reset();
		
	setState(this, ClientState::Connecting);
	
	//_ws.Connect += sdelegate(this, &Client::onSocketConnect);
	//_ws.Recv += sdelegate(this, &Client::onSocketRecv);
	//_ws.Error += sdelegate(this, &Client::onSocketError);
	//_ws.Close += sdelegate(this, &Client::onSocketClose);
	
	sendHandshakeRequest();
}


void Client::close()
{			
	TraceL << "Closing" << endl;
	if (_sessionID.empty())
		return;

	reset();
	onClose();
	TraceL << "Closing: OK" << endl;	
}


void Client::sendHandshakeRequest()
{
	//Mutex::ScopedLock lock(_mutex);
		
	TraceL << "Send handshake request" << endl;	
	
	std::ostringstream url;
	url << (_ws.socket->transport() == net::SSLTCP ? "https://" : "http://")
		<< _host << ":" << _port
		<< "/socket.io/1/";

	//http::URL url(
	//	_ws.socket->transport() == net::SSLTCP ? "https" : "http", 
	//	_endpoint, "/socket.io/1/websocket/");
	//assert(url.valid());
	
	auto conn = http::Client::instance().createConnection(url.str());
	conn->Complete += sdelegate(this, &Client::onHandshakeResponse);
	conn->setReadStream(new std::stringstream);
	conn->request().setMethod("POST");
	conn->request().setKeepAlive(false);
	conn->request().setContentLength(0);
	conn->request().setURI("/socket.io/1/");
	conn->send();
}


void Client::onHandshakeResponse(void* sender, const http::Response& response)
{
	auto conn = reinterpret_cast<http::ClientConnection*>(sender);

	std::string body = conn->readStream<std::stringstream>()->str();		
	//TraceL << "SocketIO handshake response:" 
	//	<< "\n\tStatus: " << response.getStatus()
	//	<< "\n\tReason: " << response.getReason()
	//	<< "\n\tResponse: " << body << endl;
		
	// The server can respond in three different ways:
	// 401 NotAuthorized: If the server refuses to authorize the client to connect, 
	//		based on the supplied information (eg: Cookie header or custom query components).
	// 503 Service Unavailable: If the server refuses the connection for any reason (eg: overload).
	// 200 OK: The handshake was successful.
	if (response.getStatus() != http::StatusCode::OK) {
		setError(util::format("SocketIO handshake failed: HTTP error: %d %s", 
			static_cast<int>(response.getStatus()), response.getReason().c_str()));
		return;
	}

	// Parse the response
	std::vector<std::string> respData = util::split(body, ':', 4);
	if (respData.size() < 4) {
		setError(body.empty() ? 
			"Invalid SocketIO handshake response." : util::format(
			"Invalid SocketIO handshake response: %s", body.c_str()));
		return;
	}
	
	_sessionID = respData[0];
	_heartBeatTimeout = util::strtoi<UInt32>(respData[1]);
	_connectionClosingTimeout = util::strtoi<UInt32>(respData[2]);
	_protocols = util::split(respData[3], ',');

	// Check websockets are supported
	bool wsSupported = false;
	for (unsigned i = 0; i < _protocols.size(); i++) {
		TraceL << "Supports protocol: " << _protocols[i] << endl;
		if (_protocols[i] == "websocket") {
			wsSupported = true;
			break;
		}
	}

	if (!wsSupported) {
		setError("The SocketIO server does not support WebSockets.");
		return;
	}
	
	//Mutex::ScopedLock lock(_mutex);
	
	// Initialize the WebSocket
	TraceL << "Websocket connecting: " << _sessionID << endl;	
	
	/*
	ostringstream url;
	url << (_ws.socket->transport() == net::SSLTCP ? "wss://" : "ws://")
		<< _host << ":" << _port << "/socket.io/1/websocket/" 
		<< _sessionID;
	
	_ws.socket->request().setURI(url.str());
	*/
	//_ws.setRecvAdapter(this);
	//_ws.adapter = this;
	_ws.request().setURI("/socket.io/1/websocket/" + _sessionID);
	_ws.request().setHost(_host, _port);
	_ws.socket->connect(_host, _port);
}
	
	//TraceL << "Connecting: " << uri.str() << endl;	
	//_ws.socket->request().setHost("localhost");


int Client::sendConnect(const std::string& endpoint, const std::string& query)
{
	// (1) Connect
	// Only used for multiple sockets. Signals a connection to the endpoint. 
	// Once the server receives it, it's echoed back to the client.
	// 
	// Example, if the client is trying to connect to the endpoint /test, a message like this will be delivered:
	// 
	// '1::' [path] [query]
	// Example:
	// 
	// 1::/test?my=param
	std::string out = "1::";
	if (!endpoint.empty())
		out += "/" + endpoint;
	if (!query.empty())
		out += "?" + query;
	return _ws.send(out.c_str(), out.size());
}


int Client::send(sockio::Packet::Type type, const std::string& data, bool ack)
{
	Packet packet(type, data, ack);
	return send(packet);
}


int Client::send(const std::string& data, bool ack)
{
	Packet packet(data, ack);
	return send(packet);
}


int Client::send(const json::Value& data, bool ack)
{
	Packet packet(data, ack);

	//TraceL << "Sending message: " << packet.toString() << endl;
	return send(packet);
}


int Client::send(const sockio::Packet& packet)
{
	return _ws.sendPacket(packet);
}


int Client::emit(const std::string& event, const json::Value& args, bool ack)
{
	Packet packet(event, args, ack);
	return send(packet);
}


Transaction* Client::createTransaction(const sockio::Packet& request, long timeout)
{
	return new Transaction(*this, request, timeout);
}


int Client::sendHeartbeat()
{
	//TraceL << "Sending heartbeat" << endl;
	return _ws.send("2::", 3);
}


void Client::reset()
{
	//Mutex::ScopedLock lock(_mutex);

	// Note: Only reset session related variables here.
	// Do not reset host and port variables.

	_timer.Timeout -= sdelegate(this, &Client::onHeartBeatTimer);
	_timer.stop();	

	//_ws.socket->Connect -= sdelegate(this, &Client::onSocketConnect);
	//_ws.socket->Recv -= sdelegate(this, &Client::onSocketRecv);
	//_ws.socket->Error -= sdelegate(this, &Client::onSocketError);
	//_ws.socket->Close -= sdelegate(this, &Client::onSocketClose);
	_ws.socket->close();	
		
	_sessionID = "";	
	_heartBeatTimeout = 0;
	_connectionClosingTimeout = 0;
	_protocols.clear();
	_error.reset();
		
	//_wasOnline = false; // Reset via onClose()
}


void Client::setError(const scy::Error& error)
{
	ErrorL << "Set error: " << error.message << std::endl;
	
	// Set the wasOnline flag if previously online before error
	if (stateEquals(ClientState::Online))
		_wasOnline = true;

	_error = error;	
	setState(this, ClientState::Error, error.message);

	// Note: Do not call close() here, since we will be trying to reconnect...
}


void Client::onConnect()
{
	TraceL << "On connect" << endl;
			
	setState(this, ClientState::Connected);

	//Mutex::ScopedLock lock(_mutex);

	// Start the heartbeat timer
	assert(_heartBeatTimeout);
	int interval = static_cast<int>(_heartBeatTimeout * .75) * 1000;
	_timer.Timeout += sdelegate(this, &Client::onHeartBeatTimer);
	_timer.start(interval, interval);
}


void Client::onOnline()
{
	TraceL << "On online" << endl;	
	setState(this, ClientState::Online);
}


void Client::onClose()
{
	TraceL << "On close" << endl;

	// Back to initial state
	setState(this, ClientState::None);
	_wasOnline = false;
}


//
// Socket Callbacks

void Client::onSocketConnect()
{
	// Start 
	onConnect();

	// Transition to online state
	onOnline();
}


void Client::onSocketError(const scy::Error& error)
{
	TraceL << "On socket error: " << error.message << endl;
		
	setError(error);
}


void Client::onSocketClose()
{
	TraceL << "On socket close" << endl;

	// Nothing to do since the error is set via onSocketError

	// If no socket error was set we have an EOF
	//if (!error().any())
	//	setError("Disconnected from the server");
}


void Client::onSocketRecv(const MutableBuffer& buffer, const net::Address& peerAddress)
{	
	TraceL << "On socket recv: " << buffer.size() << endl;
		
	sockio::Packet pkt;
	char* buf = bufferCast<char*>(buffer);
	std::size_t len = buffer.size();
	std::size_t nread = 0;
	while (len > 0 && (nread = pkt.read(constBuffer(buf, len))) > 0) {
		onPacket(pkt);
		buf += nread;
		len -= nread;
	}
	if (len == buffer.size())
		WarnL << "Failed to parse incoming SocketIO packet." << endl;	

#if 0
	sockio::Packet pkt;
	if (pkt.read(constBuffer(packet.data(), packet.size())))
		onPacket(pkt);
	else
		WarnL << "Failed to parse incoming SocketIO packet." << endl;	
#endif
}


void Client::onPacket(sockio::Packet& packet)
{
	TraceL << "On packet: " << packet.toString() << endl;		
	PacketSignal::emit(this, packet);	
}

	
void Client::onHeartBeatTimer(void*)
{
	TraceL << "On heartbeat" << endl;
	
	if (isOnline())
		sendHeartbeat();

	// Try to reconnect if disconnected in error
	else if (error().any()) {	
		TraceL << "Attempting to reconnect" << endl;	
		try {
			connect();
		} 
		catch (std::exception& exc) {			
			ErrorL << "Reconnection attempt failed: " << exc.what() << endl;
		}	
	}
}


http::ws::WebSocket& Client::ws()
{
	//Mutex::ScopedLock lock(_mutex);
	return _ws;
}


std::string Client::sessionID() const 
{
	//Mutex::ScopedLock lock(_mutex);
	return _sessionID;
}


Error Client::error() const 
{
	//Mutex::ScopedLock lock(_mutex);
	return _error;
	//return _ws.socket->error();
}


bool Client::isOnline() const
{
	return stateEquals(ClientState::Online);
}

bool Client::wasOnline() const
{
	//Mutex::ScopedLock lock(_mutex);
	return _wasOnline; 
}



} } // namespace scy::sockio





/*


uv::Loop* Client::loop()
{
	//Mutex::ScopedLock lock(_mutex);
	return _loop;
}
std::string& Client::endpoint()
{
	//Mutex::ScopedLock lock(_mutex);
	return _endpoint;
}
*/

	// Set the socket error resulting in closure via callbacks
	//socket().setError(error);
	//close();

	
	/*
	TraceL << "%%%%%%%%%%%%%% Sending Handshake" << endl;	

	conn->Headers += sdelegate(this, &Client::onStandaloneHTTPClientConnectionHeaders);
	conn->Complete += sdelegate(this, &Client::onStandaloneHTTPClientConnectionComplete);
	http::Request* request = new http::Request("POST", uri.str());	
	http::Response response;
	assert(0);
	http::Transaction transaction(request);
	http::Response& response = transaction.response();
	transaction.send();

	TraceL << "SocketIO Handshake Response:" 
		<< "\n\tStatus: " << response.getStatus()
		<< "\n\tReason: " << response.getReason()
		<< "\n\tResponse: " << response.body.str()
		<< endl;
		
	// The server can respond in three different ways:
	// 401 NotAuthorized: If the server refuses to authorize the client to connect, 
	//		based on the supplied information (eg: Cookie header or custom query components).
	// 503 Service Unavailable: If the server refuses the connection for any reason (eg: overload).
	// 200 OK: The handshake was successful.
	if (response.getStatus() != 200)
		throw std::runtime_error(Poco::format("SocketIO handshake failed: HTTP Error: %d %s", 
			static_cast<int>(response.getStatus()), response.getReason()));

	// Parse the response response
	std::vector<std::string> respData = util::split(response.body.str(), ':', 4);
	if (respData.size() < 4)
		throw std::runtime_error(response.empty() ? 
			"Invalid SocketIO handshake response." : Poco::format(
			"Invalid SocketIO handshake response: %s", response.body.str()));
	
	_sessionID = respData[0];
	_heartBeatTimeout = util::strtoi<UInt32>(respData[1]);
	_connectionClosingTimeout = util::strtoi<UInt32>(respData[2]);
	_protocols = util::split(respData[3], ',');

	// Check websockets are supported
	bool wsSupported = false;
	for (int i = 0; i < _protocols.size(); i++) {
		TraceL << "Supports Protocol: " << _protocols[i] << endl;
		if (_protocols[i] == "websocket") {
			wsSupported = true;
			break;
		}
	}

	if (!wsSupported)
		throw std::runtime_error("The SocketIO server does not support WebSockets.");
	*/



/*
	//if (socket().isConnected())
	//else if (socket().isError()) {	
	//if (!socket().error()) //.empty()
	//	setError(socket().error());
	//else
net::Address Client::serverAddr() const 
{
	//Mutex::ScopedLock lock(_mutex);
	return _serverAddr;
}
*/

		/*
		Timer::getDefault().stop(TimerCallback<Client>(this, &Client::onHeartBeatTimer));
		if (_timer) {
			_timer.Timeout -= sdelegate(this, &Client::onHeartBeatTimer);
			_timer.destroy();	
			_timer = NULL;
		}
		*/
	
	
	
	/*
void Client::onHeartBeatTimer(TimerCallback<Socket>&) 
	Timer::getDefault().start(TimerCallback<Client>(this, &Client::onHeartBeatTimer, 
		(_heartBeatTimeout * .75) * 1000, 
		(_heartBeatTimeout * .75) * 1000));

	_timer = new TimerTask(
		(_heartBeatTimeout * .75) * 1000, 
		(_heartBeatTimeout * .75) * 1000);
	//if (!_timer) {
	//}
	//if (_timer.cancelled())
		*/
/*


	//_ws.socket->registerPacketType<sockio::Packet>(10);

void Client::onError() 
{
	WarnL << "On error" << endl;
}
*/

	
	//if (!_ws) {
	//	_ws = createSocket();
	//}

	//if (!isError()) {
	// If the socket was closed in error we keep trying to reconnect.
		//Timer::getDefault().stop(TimerCallback<SocketBase>(this, &SocketBase::onHeartBeatTimer));
	//}	
	
	//if (_ws) {
		//delete _ws;
		//_ws = NULL;
	//}

	//if (!_ws)
	//	throw std::runtime_error("The SocketIO WebSocket pointer is NULL.");

	//std::string uri("http://" + _serverAddr.toString() + "/socket.io/1/");	
	//if (_ws.socket->transport() == Net::SSLTCP)
	//	uri.setScheme("https");	
	//std::string uri("ws://" + _serverAddr.toString() + "/socket.io/1/");	
	//if (socket.transport() == Net::SSLTCP)
	//	uri.setScheme("wss");
	//uri.setPath("/socket.io/1/websocket/" + _sessionID);



	//Timer::getDefault().start(TimerCallback<SocketBase>(this, &SocketBase::onHeartBeatTimer, 
	//	(_heartBeatTimeout * .75) * 1000, 
	//	(_heartBeatTimeout * .75) * 1000));		


//SocketBase::SocketBase(Net::Reactor& reactor) :
//	WebSocket(reactor),
//	_secure(false)
//{
//}
//
//
//SocketBase::SocketBase(const net::Address& serverAddr) :
//	WebSocket(reactor),
//	_serverAddr(serverAddr),
//	_secure(false)
//{
//}
//
//
//SocketBase::~SocketBase() 
//{
//	close();
//}
//
//
//void SocketBase::connect(const net::Address& serverAddr)
//{	
//	{
//		Mutex::ScopedLock lock(_mutex);
//		_serverAddr = serverAddr;
//	}
//	connect();
//}
//
//
//void SocketBase::connect()
//{
//	Mutex::ScopedLock lock(_mutex);
//
//	assert(_serverAddr.valid());
//
//	if (isActive())
//		throw std::runtime_error("The SocketIO Socket is already active.");
//
//	_uri = "ws://" + _serverAddr.toString() + "/socket.io/1/";	
//	if (_secure)
//		_uri.setScheme("wss");
//
//	DebugL << "[sockio::Socket]	Connecting to " << _uri.toString() << endl;
//
//	if (sendHandshakeRequest()) {
//				
//		Timer::getDefault().start(TimerCallback<Socket>(this, &SocketBase::onHeartBeatTimer, 
//			(_heartBeatTimeout * .75) * 1000, 
//			(_heartBeatTimeout * .75) * 1000));
//	
//		// Receive SocketIO packets
//		registerPacketType<sockio::Packet>(10);
//		
//		// Initialize the websocket
//		_uri.setPath("/socket.io/1/websocket/" + _sessionID);
//		WebSocketBase::connect(_uri);
//	}
//}
//
//
//bool SocketBase::sendHandshakeRequest()
//{
//	// NOTE: No need for mutex lock because this method is called from connect()
//	
//	TraceL << "[sockio::Socket] Sending Handshake" << endl;
//	
//	
//	URI uri("http://" + _serverAddr.toString() + "/socket.io/1/");	
//	if (_secure)
//		uri.setScheme("https");
//
//	http::Request* request = new http::Request("POST", uri.toString());	
//	http::Transaction transaction(request);
//	http::Response& response = transaction.response();
//
//	// The server can respond in three different ways:
//    // 401 NotAuthorized: If the server refuses to authorize the client to connect, 
//	//		based on the supplied information (eg: Cookie header or custom query components).
//    // 503 Service Unavailable: If the server refuses the connection for any reason (eg: overload).
//	// 200 OK: The handshake was successful.
//	if (!transaction.send())
//		throw std::runtime_error(format("SocketIO handshake failed: HTTP Error: %d %s", 
//			static_cast<int>(response.getStatus()), response.getReason()));		
//
//	TraceL << "[sockio::Socket] Handshake Response:" 
//		<< "\n\tStatus: " << response.getStatus()
//		<< "\n\tReason: " << response.getReason()
//		<< "\n\tResponse: " << response.body.str()
//		<< endl;
//
//	//if (status != 200)
//	//	throw std::runtime_error(format("SocketIO handshake failed with %d", status));	
//
//	// Parse the response response
//	std::vector<std::string> respData = util::split(response.body.str(), ':', 4);
//	if (respData.size() < 4)
//		throw std::runtime_error(response.empty() ? 
//			"Invalid SocketIO handshake response." : format(
//			"Invalid SocketIO handshake response: %s", response.body.str()));
//	
//	_sessionID = respData[0];
//	_heartBeatTimeout = util::strtoi<UInt32>(respData[1]);
//	_connectionClosingTimeout = util::strtoi<UInt32>(respData[2]);
//	_protocols = util::split(respData[3], ',');
//
//	// Check websockets are supported
//	bool wsSupported = false;
//	for (int i = 0; i < _protocols.size(); i++) {
//		DebugL << "[sockio::Socket] Supports Protocol: " << _protocols[i] << endl;	
//		if (_protocols[i] == "websocket") {
//			wsSupported = true;
//			break;
//		}
//	}
//
//	if (!wsSupported)
//		throw std::runtime_error("The SocketIO server does not support WebSockets");
//
//	return true;
//}
//
//
//void SocketBase::close()
//{			
//	TraceL << "[sockio::Socket] Closing" << endl;	
//
//	if (!isError())
//		Timer::getDefault().stop(TimerCallback<Socket>(this, &SocketBase::onHeartBeatTimer));
//	
//	WebSocketBase::close();
//
//	TraceL << "[sockio::Socket] Closing: OK" << endl;	
//}
//
//
//int SocketBase::sendConnect(const std::string& endpoint, const std::string& query)
//{
//	Mutex::ScopedLock lock(_mutex);
//	// (1) Connect
//	// Only used for multiple sockets. Signals a connection to the endpoint. Once the server receives it, it's echoed back to the client.
//	// 
//	// Example, if the client is trying to connect to the endpoint /test, a message like this will be delivered:
//	// 
//	// '1::' [path] [query]
//	// Example:
//	// 
//	// 1::/test?my=param
//	std::string out = "1::";
//	if (!endpoint.empty())
//		out += "/" + endpoint;
//	if (!query.empty())
//		out += "?" + query;
//	return WebSocketBase::send(out.c_str(), out.size());
//}
//
//
//int SocketBase::send(sockio::Packet::Type type, const std::string& data, bool ack)
//{
//	Packet packet(type, data, ack);
//	return send(packet);
//}
//
//
//int SocketBase::send(const std::string& data, bool ack)
//{
//	Packet packet(data, ack);
//	return send(packet);
//}
//
//
//int SocketBase::send(const json::Value& data, bool ack)
//{
//	Packet packet(data, ack);
//	return send(packet);
//}
//
//
//int SocketBase::send(const sockio::Packet& packet)
//{
//	return WebSocketBase::send(packet);
//}
//
//
//int SocketBase::emit(const std::string& event, const json::Value& args, bool ack)
//{
//	Packet packet(event, args, ack);
//	return send(packet);
//}
//
//
//void SocketBase::onHeartBeatTimer(TimerCallback<Socket>&) 
//{
//	TraceL << "[sockio::Socket] Heart Beat Timer" << endl;
//	
//	if (isConnected())
//		sendHeartbeat();
//
//	// Try to reconnect if the connection was closed in error
//	else if (isError()) {	
//		TraceL << "[sockio::Socket] Attempting to reconnect" << endl;	
//		try {
//			connect();
//		} 
//		catch (std::exception& exc) {			
//			ErrorL << "[sockio::Socket] Reconnection attempt failed: " << exc.what() << endl;
//		}	
//	}
//}
//
//
//int SocketBase::sendHeartbeat()
//{
//	TraceL << "[sockio::Socket] Heart Beat" << endl;
//	return WebSocketBase::send("2::", 3);
//}
//
//
//void SocketBase::setSecure(bool flag)
//{
//	Mutex::ScopedLock lock(_mutex);
//	_secure = flag;
//}
//
//
//http::ws::WebSocket* SocketBase::socket()
//{
//	Mutex::ScopedLock lock(_mutex);
//	return _ws;
//}
//
//
//KVCollection& SocketBase::httpHeaders()
//{
//	Mutex::ScopedLock lock(_mutex);
//	return _httpHeaders;
//}
//
//
//string SocketBase::sessionID() const 
//{
//	Mutex::ScopedLock lock(_mutex);
//	return _sessionID;
//}




	/*
	//BitReader reader(body.c_str(), body.length());
	//reader.skipToNextLine(); // skip to chunk payload
	//string body(reader.current(), reader.available()); // = writer.toString();
	int status = 503;
	std::string response;
	
	// HTTPS
	if (_secure) {
		HTTPSClientSession s(_serverAddr.host().toString(), _serverAddr.port());
		HTTPRequest req(HTTPRequest::HTTP_POST, "/socket.io/1/");	
		s.sendRequest(req);
		HTTPResponse res;
		istream& rstr = s.receiveResponse(res);
		ostringstream rdata;
		StreamCopier::copyStream(rstr, rdata);
		status = res.getStatus();
		response = rdata.str();
	}

	// HTTP
	else {
		HTTPClientSession s(_serverAddr.host().toString(), _serverAddr.port());
		HTTPRequest req(HTTPRequest::HTTP_POST, "/socket.io/1/");	
		s.sendRequest(req);
		HTTPResponse res;
		istream& rstr = s.receiveResponse(res);
		ostringstream rdata;
		StreamCopier::copyStream(rstr, rdata);
		status = res.getStatus();
		response = rdata.str();
	}
	*/


	/*

	HTTPClientSession s(_serverAddr);
	HTTPRequest request(HTTPRequest::HTTP_POST, "/socket.io/1/");	
	s.sendRequest(request);
	HTTPResponse response;
	istream& rs = s.receiveResponse(response);
	StreamCopier::copyStream(rs, body);
	int status = response.getStatus();
	
	// Seems to be a strange bug where data copying from receiveBytes
	// into a iostream is invalid. This is causing Poco to throw
	// a NoMesageException when using the HTTPRequest object because 
	// the first byte == eof.
	// Copying into a string yields no such problem, so using 
	// string for now.

	StreamSocket socket;	
	socket.connect(_serverAddr);
	SocketStream ss(socket);
	
	// Request
	HTTPRequest request("POST", "/socket.io/1/");	
	for (KVCollection::ConstIterator it = _httpHeaders.begin(); it != _httpHeaders.end(); it++)
		request.set((*it).first, (*it).second);
	//request.setContentLength(0);
	request.write(ss);
	ss.flush();
	assert(ss.good());	
	
	// Response
	char buffer[1024];
	int size = socket.receiveBytes(buffer, sizeof(buffer));	
	std::string response(buffer, size);
	size_t pos = response.find(" "); pos++;
	int status = util::strtoi<UInt32>(response.substr(pos, response.find(" ", pos) + 1));
	pos = response.find("\r\n\r\n", pos); pos += 4;
	std::string body(response.substr(pos, response.length()));
	socket.shutdownSend();

	DebugL << "[sockio::Socket] Handshake:" 
		//<< "\n\tRequest: " << ss.str()
		<< "\n\tStatus: " << status
		<< "\n\tResponse: " << response
		<< "\n\tResponse Len: " << response.size()
		<< "\n\tBody: " << body
		<< endl;

	std::string response = transaction.response().body.str();
		*/