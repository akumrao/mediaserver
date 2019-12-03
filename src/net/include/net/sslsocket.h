#ifndef Net_SSLSocket_H
#define Net_SSLSocket_H


//#include "net/socket.h"
#include "net/ssladapter.h"
#include "net/sslcontext.h"
#include "net/sslsession.h"
#include "net/TcpConnection.h"
//#include "scy/handle.h"


namespace base {
namespace net {


/// SSL socket implementation.
class  SSLSocket : public TcpConnection
{
public:
    typedef std::shared_ptr<SSLSocket> Ptr;
    typedef std::vector<Ptr> Vec;

//    SSLSocket(uv::Loop* loop = uv::defaultLoop()); //, SocketMode mode = ClientSide
   // SSLSocket(SSLContext::Ptr sslContext, uv::Loop* loop = uv::defaultLoop());
   // SSLSocket(SSLContext::Ptr sslContext, SSLSession::Ptr session,
   //           uv::Loop* loop = uv::defaultLoop());

    virtual ~SSLSocket();

    /// Initialize the SSLSocket with the given SSLContext.
    // virtual void init(SSLContext::Ptr sslContext, SocketMode mode = ClientSide);

    /// Initializes the socket and establishes a secure connection to
    /// the TCP server at the given address.
    ///
    /// The SSL handshake is performed when the socket is connected.
    // virtual void connect(const Address& peerAddress);

    //virtual void bind(const net::Address& address, unsigned flags = 0) override;
   // virtual void listen(int backlog = 64) override;

    /// Shuts down the connection by attempting
    /// an orderly SSL shutdown, then actually
    /// shutting down the TCP connection.
//    virtual bool shutdown() override;

    /// Closes the socket forcefully.
//    virtual void close() override;

//    virtual ssize_t send(const char* data, size_t len, int flags = 0) override;
 //   virtual ssize_t send(const char* data, size_t len,
 //                        const net::Address& peerAddress, int flags = 0) override;

    /// Use the given SSL context for this socket.
    void useContext(SSLContext::Ptr context);

    /// Returns the SSL context used for this socket.
    SSLContext::Ptr context() const;

    /// Sets the SSL session to use for the next
    /// connection. Setting a previously saved Session
    /// object is necessary to enable session caching.
    ///
    /// To remove the currently set session, a nullptr pointer
    /// can be given.
    ///
    /// Must be called before connect() to be effective.
    void useSession(SSLSession::Ptr session);

    /// Returns the SSL session of the current connection,
    /// for reuse in a future connection (if session caching
    /// is enabled).
    ///
    /// If no connection is established, returns nullptr.
    SSLSession::Ptr currentSession();

    /// Returns true if a reused session was negotiated during
    /// the handshake.
    bool sessionWasReused();

    /// Returns the number of bytes available from the
    /// SSL buffer for immediate reading.
    int available() const;

    /// Returns the peer's certificate.
    X509* peerCertificate() const;

//    net::TransportType transport() const override;

   // virtual void acceptConnection() override;

   // virtual void onConnect() override;

    /// Reads raw encrypted SSL data
  //  virtual void onRead(const char* data, size_t len) override;

protected:
    net::SSLContext::Ptr _sslContext;
    net::SSLSession::Ptr _sslSession;
    net::SSLAdapter _sslAdapter;

    friend class net::SSLAdapter;
};


} // namespace net
} // namespace base


#endif // Net_SSLSocket_H


