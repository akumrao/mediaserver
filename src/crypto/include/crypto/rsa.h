


#ifndef Crypto_RSA_H
#define Crypto_RSA_H


#include "crypto/crypto.h"
#include <openssl/rsa.h>


namespace base {
namespace crypto {


/// Forward the OpenSSL type to our namespace.
/// This may become a class/wrapper in the future.
typedef ::RSA RSAKey;


} // namespace crypto
} // namespace base


#endif // Crypto_RSA_H


