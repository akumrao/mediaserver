
#ifndef Net_SSLSession_H
#define Net_SSLSession_H


#include "crypto/crypto.h"
//#include "base/memory.h"
//#include "net/net.h"

#include <openssl/ssl.h>


namespace base {
namespace net {


/// This class encapsulates a SSL session object
/// used with session caching on the client side.
///
/// For session caching to work, a client must
/// save the session object from an existing connection,
/// if it wants to reuse it with a future connection.
class  SSLSession
{
public:
    typedef std::shared_ptr<SSLSession> Ptr;

    /// Returns the stored OpenSSL SSL_SESSION object.
    SSL_SESSION* sslSession() const;

    /// Creates a new Session object, using the given
    /// SSL_SESSION object.
    ///
    /// The SSL_SESSION's reference count is not changed.
    SSLSession(SSL_SESSION* ptr);

    /// Destroys the Session.
    ///
    /// Calls SSL_SESSION_free() on the stored
    /// SSL_SESSION object.
    ~SSLSession();

    SSLSession();

protected:
    SSL_SESSION* _ptr;
};


} // namespace net
} // namespace base


#endif // Net_SSLSession_H

