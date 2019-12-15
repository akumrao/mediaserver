

#include "net/sslsession.h"


using namespace std;


namespace base {
namespace net {


SSLSession::SSLSession(SSL_SESSION* ptr)
    : _ptr(ptr)
{
}


SSLSession::~SSLSession()
{
    SSL_SESSION_free(_ptr);
}


SSL_SESSION* SSLSession::sslSession() const
{
    return _ptr;
}


} // namespace net
} // namespace base

