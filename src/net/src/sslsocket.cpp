#include "net/netInterface.h"
#include "net/sslsocket.h"
#include "base/logger.h"
#include "net/sslmanager.h"


using namespace std;


namespace base {
namespace net {

/*
SSLSocket::SSLSocket(uv::Loop* loop)
    : TCPSocket(loop)
    , _sslContext(nullptr)
    , _sslSession(nullptr)
    , _sslAdapter(this)
{
    LTrace("Create")
}


SSLSocket::SSLSocket(SSLContext::Ptr context, uv::Loop* loop)
    : TCPSocket(loop)
    , _sslContext(context)
    , _sslSession(nullptr)
    , _sslAdapter(this)
{
    LTrace("Create")
}


SSLSocket::SSLSocket(SSLContext::Ptr context, SSLSession::Ptr session, uv::Loop* loop)
    : TCPSocket(loop)
    , _sslContext(context)
    , _sslSession(session)
    , _sslAdapter(this)
{
    LTrace("Create")
}
*/

SSLSocket::~SSLSocket()
{
    LTrace("Destroy")
}


int SSLSocket::available() const
{
    return _sslAdapter.available();
}

/*
void SSLSocket::close()
{
    TCPSocket::close();
}


bool SSLSocket::shutdown()
{
    LTrace("Shutdown")
    try {
        // Try to gracefully shutdown the SSL connection
        _sslAdapter.shutdown();
    } catch (...) {
    }
    return TCPSocket::shutdown();
}


ssize_t SSLSocket::send(const char* data, size_t len, int flags)
{
    return send(data, len, peerAddress(), flags);
}


void SSLSocket::bind(const net::Address& address, unsigned flags)
{
    assert(_sslContext->isForServerUse());
    TCPSocket::bind(address, flags);
}


void SSLSocket::listen(int backlog)
{
    assert(_sslContext->isForServerUse());
    TCPSocket::listen(backlog);
}


ssize_t SSLSocket::send(const char* data, size_t len, const net::Address& , int )
{
    LTrace("send: ", len)
    assert(Thread::currentID() == tid());
    // assert(len <= net::MAX_TCP_PACKET_SIZE);

    if (!active()) {
        LWarn("send error")
        return -1;
    }

    // send unencrypted data to the SSL context

    assert(_sslAdapter._ssl);

    _sslAdapter.addOutgoingData(data, len);
    _sslAdapter.flush();
    return len;
}


void SSLSocket::acceptConnection()
{
    assert(_sslContext->isForServerUse());

    // Create the shared socket pointer so the if the socket handle is not
    // incremented the accepted socket will be destroyed.
    auto socket = std::make_shared<net::SSLSocket>(_sslContext, loop());

    LTrace("Accept SSL connection: ")
    // invoke(&uv_tcp_init, loop(), socket->get()); // "Cannot initialize SSL socket"

    if (uv_accept(get<uv_stream_t>(), socket->get<uv_stream_t>()) == 0) {
        socket->readStart();
        socket->_sslAdapter.initServer();

        AcceptConnection.emit(socket);
    }
    else {
        assert(0 && "uv_accept should not fail");
    }
}
*/

void SSLSocket::useSession(SSLSession::Ptr session)
{
    _sslSession = session;
}


SSLSession::Ptr SSLSocket::currentSession()
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


void SSLSocket::useContext(SSLContext::Ptr context)
{
    if (_sslAdapter._ssl)
        throw std::runtime_error(
            "Cannot change the SSL context for an active socket.");

    _sslContext = context;
}


SSLContext::Ptr SSLSocket::context() const
{
    return _sslContext;
}


bool SSLSocket::sessionWasReused()
{
    if (_sslAdapter._ssl)
        return SSL_session_reused(_sslAdapter._ssl) != 0;
    else
        return false;
}

/*
net::TransportType SSLSocket::transport() const
{
    return net::SSLTCP;
}


//
// Callbacks

void SSLSocket::onRead(const char* data, size_t len)
{
    LTrace("On SSL read: ", len)

    // SSL encrypted data is sent to the SSL context
    _sslAdapter.addIncomingData(data, len);
    _sslAdapter.flush();
}


void SSLSocket::onConnect()
{
    LTrace("On connect")
    if (readStart()) {
        _sslAdapter.initClient();
        // _sslAdapter.start();
        onSocketConnect(*this);
    }
}
*/

} // namespace net
} // namespace base

