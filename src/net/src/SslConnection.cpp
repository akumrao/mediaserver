#include "net/netInterface.h"
#include "net/SslConnection.h"
#include "base/logger.h"
#include "net/sslmanager.h"


using namespace std;


namespace base {
namespace net {


SslConnection::SslConnection(Listener* listener)
    : TcpConnection(listener,65536, true)
    , _sslContext(nullptr)
    , _sslSession(nullptr)
    , _sslAdapter(this)
    ,listener(listener)
{
    LTrace("Create")
    
 
}


SslConnection::SslConnection(Listener* listener, bool server)
    : TcpConnection(listener , 65536, true)
    , _sslSession(nullptr)
    , _sslAdapter(this)
    ,listener(listener),serverMode(server)
{
    if(server)
        _sslAdapter.initServer();
    LTrace("Create")
}


SslConnection::SslConnection(Listener* listener, SSLContext::Ptr context, SSLSession::Ptr session)
    :  TcpConnection(listener)
    , _sslContext(context)
    , _sslSession(session)
    , _sslAdapter(this)
    ,listener(listener)
{
    LTrace("Create")
}


SslConnection::~SslConnection()
{
    LTrace("Destroy")
}


int SslConnection::available() const
{
    return _sslAdapter.available();
}

/*
void SslConnection::close()
{
    TCPSocket::close();
}


bool SslConnection::shutdown()
{
    LTrace("Shutdown")
    try {
        // Try to gracefully shutdown the SSL connection
        _sslAdapter.shutdown();
    } catch (...) {
    }
    return TCPSocket::shutdown();
}
*/

/*

void SslConnection::bind(const net::Address& address, unsigned flags)
{
    assert(_sslContext->isForServerUse());
    TCPSocket::bind(address, flags);
}


void SslConnection::listen(int backlog)
{
    assert(_sslContext->isForServerUse());
    TCPSocket::listen(backlog);
}

*/

/*void SslConnection::acceptConnection()
{
   // assert(_sslContext->isForServerUse());

    // Create the shared socket pointer so the if the socket handle is not
    // incremented the accepted socket will be destroyed.
   // auto socket = std::make_shared<net::SslConnection>(_sslContext, loop());

   // LTrace("Accept SSL connection: ")
    // invoke(&uv_tcp_init, loop(), socket->get()); // "Cannot initialize SSL socket"

   // if (uv_accept(get<uv_stream_t>(), socket->get<uv_stream_t>()) == 0) {
      //  socket->readStart();
      

       // AcceptConnection.emit(socket);
    //}
    //else {
       // assert(0 && "uv_accept should not fail");
   // }
}*/


void SslConnection::useSession(SSLSession::Ptr session)
{
    _sslSession = session;
}


SSLSession::Ptr SslConnection::currentSession()
{
    if (_sslAdapter._ssl) {
        SSL_SESSION* session = SSL_get1_session(_sslAdapter._ssl);
        if (session) {
            if (_sslSession && session == _sslSession->sslSession()) {
                SSL_SESSION_free(session);
                return _sslSession;
            } else
                return std::make_shared<SSLSession>(session); // new SSLSession(session);
        }
    }
    return 0;
}


void SslConnection::useContext(SSLContext::Ptr context)
{
    if (_sslAdapter._ssl)
        throw std::runtime_error(
            "Cannot change the SSL context for an active socket.");

    _sslContext = context;
}


SSLContext::Ptr SslConnection::context() const
{
    return _sslContext;
}


bool SslConnection::sessionWasReused()
{
    if (_sslAdapter._ssl)
        return SSL_session_reused(_sslAdapter._ssl) != 0;
    else
        return false;
}

/*
net::TransportType SslConnection::transport() const
{
    return net::SSLTCP;
}
*/
void SslConnection::send(const char* data, size_t len)
{
 

    LTrace("send: ", len)
    LTrace("send: ", data)
   // assert(Thread::currentID() == tid());
    // assert(len <= net::MAX_TCP_PACKET_SIZE);
/*
    if (!active()) {
        LWarn("send error")
        return -1;
    }
*/
    // send unencrypted data to the SSL context

    assert(_sslAdapter._ssl);

    _sslAdapter.addOutgoingData(data, len);
    _sslAdapter.flush();
    return ;
}

//
// Callbacks

void SslConnection::on_tls_read(const char* data, size_t len)
{
    LTrace("On SSL read: ", len)
    LTrace("On SSL read: ", data)

    // SSL encrypted data is sent to the SSL context
    _sslAdapter.addIncomingData(data, len);
    _sslAdapter.flush();
}

void SslConnection::on_read(const char* data, size_t len)
{
    LTrace("SslConnection::on_read ", len)
    LTrace("SslConnection::on_read ", data)

    listener->on_read(this,data,len );
  
}

void SslConnection::on_connect()
{
    LTrace("On connect")
   // if (readStart()) {
        _sslAdapter.initClient();
        // _sslAdapter.start();
    //emit
    listener->on_connect( this);
        
   // }
}


} // namespace net
} // namespace base

