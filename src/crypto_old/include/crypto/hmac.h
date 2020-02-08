

#ifndef Crypto_HMAC_H
#define Crypto_HMAC_H


#include "crypto/crypto.h"
#include <string>


namespace base {
namespace crypto {


/// HMAC is a MAC (message authentication code), i.e. a keyed hash function
/// used for message authentication, which is based on a hash function (SHA1).
///
/// Input is the data to be signed, and key is the private password.
std::string computeHMAC(const std::string& input, const std::string& key);


} // namespace crypto
} // namespace base


#endif // Crypto_HMAC_H


