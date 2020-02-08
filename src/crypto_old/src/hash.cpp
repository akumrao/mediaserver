

#include "crypto/hash.h"
#include "base/error.h"
#include <assert.h>
#include <iostream>

using std::endl;


namespace base {
namespace crypto {


Hash::Hash(const std::string& algorithm)
    : _algorithm(algorithm)
    , _ctx(EVP_MD_CTX_new())//for new 1.0.1  otherwise comment it
{
    crypto::initializeEngine();

    _md = EVP_get_digestbyname(algorithm.data());
    if (!_md)
        throw std::runtime_error("Algorithm not supported: " + algorithm);

    //EVP_DigestInit(&_ctx, _md);
    EVP_DigestInit(_ctx, _md);
}


Hash::~Hash()
{
    crypto::uninitializeEngine();

    //EVP_MD_CTX_cleanup(&_ctx);
    EVP_MD_CTX_free(_ctx); // open SSL 1.1.0
}


void Hash::reset()
{
     EVP_MD_CTX_free(_ctx);  
    _ctx = EVP_MD_CTX_new();
    internal::api(EVP_DigestInit(_ctx, _md)); //new open ssl

    //internal::api(EVP_MD_CTX_cleanup(&_ctx));
    //internal::api(EVP_DigestInit(&_ctx, _md));
    _digest.clear();
}


void Hash::update(const void* data, size_t length)
{

    internal::api(EVP_DigestUpdate(_ctx, data, length));
    //internal::api(EVP_DigestUpdate(&_ctx, data, length));
}


void Hash::update(const std::string& data)
{
    update(data.c_str(), data.length());
}


void Hash::update(char data)
{
    update(&data, 1);
}


const ByteVec& Hash::digest()
{
    // Compute the first time
    if (_digest.size() == 0) {
        _digest.resize(EVP_MAX_MD_SIZE); // TODO: Get actual algorithm size
        unsigned int len = 0;
        internal::api(EVP_DigestFinal(_ctx, &_digest[0], &len));
       // internal::api(EVP_DigestFinal(&_ctx, &_digest[0], &len));
        _digest.resize(len);
    }
    return _digest;
}


std::string Hash::digestStr()
{
    const ByteVec& vec = digest();
    return std::string((const char*)vec.data(), vec.size());
}


const std::string& Hash::algorithm(void) const
{
    return _algorithm;
}


} // namespace crypto
} // namespace base

