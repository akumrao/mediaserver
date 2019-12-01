

#ifndef Crypto_Hash_H
#define Crypto_Hash_H


#include "crypto/crypto.h"
#include "base/hex.h"
#include <cstdint>
#include <fstream>      // std::ifstream
#include <openssl/evp.h>


namespace base {
namespace crypto {


class  Hash
{
public:
    Hash(const std::string& algorithm);
    ~Hash();

    void update(char data);
    void update(const std::string& data);

    // Hash the given data.
    /// This function may (and normally will) be called
    /// many times for large blocks of data.
    void update(const void* data, size_t length);

    /// Finish up the digest operation and return the result.
    const ByteVec& digest();

    /// Finish up the digest operation and return the result as a string.
    std::string digestStr();

    /// Resets the engine and digest state ready for the next computation.
    void reset();

    /// Returns the hash algorithm being used.
    const std::string& algorithm(void) const;

protected:
    Hash& operator=(Hash const&);

    //EVP_MD_CTX _ctx;
    EVP_MD_CTX* _ctx;	// for openssl 1.0.1
    const EVP_MD* _md;
    crypto::ByteVec _digest;
    std::string _algorithm;
};


inline std::string hash(const std::string& algorithm, const std::string& data)
{
    Hash engine(algorithm);
    engine.update(data);
    return hex::encode(engine.digest());
}


inline std::string hash(const std::string& algorithm, const void* data,
                        unsigned length)
{
    Hash engine(algorithm);
    engine.update(data, length);
    return hex::encode(engine.digest());
}


/// Computes the MD5/SHA checksum for the given file.
inline std::string checksum(const std::string& algorithm,
                            const std::string& path)
{
    std::ifstream fstr(path, std::ios::in | std::ios::binary);
    if (!fstr.is_open())
        throw std::runtime_error("Cannot open file: " + path);

    Hash engine(algorithm);
    char buffer[4096];
    while (fstr.read(buffer, 4096) || fstr.gcount() > 0) {
        engine.update(buffer, (size_t)fstr.gcount());
    }

    return hex::encode(engine.digest());
}


} // namespace crypto
} // namespace base


#endif // Crypto_Hash_H



