

#include "crypto/cipher.h"
#include "base/base64.h"
#include "base/error.h"
#include "base/hex.h"

#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include "base/random.h"

using std::endl;


namespace base {
namespace crypto {


Cipher::Cipher(const std::string& name, const std::string& passphrase,
               const std::string& salt, int iterationCount)
    : _initialized(false)
    , _encrypt(false)
    , _cipher(nullptr)
    , _ctx(EVP_CIPHER_CTX_new())
    , _name(name)
    , _key()
    , _iv()
{
    crypto::initializeEngine();

    _cipher = EVP_get_cipherbyname(name.c_str());
    if (!_cipher)
        throw std::invalid_argument("Not found: Cipher " + name +
                                    " is unavailable");

    _key = ByteVec(keySize());
    _iv = ByteVec(ivSize());
    generateKey(passphrase, salt, iterationCount);
}


Cipher::Cipher(const std::string& name, const ByteVec& key, const ByteVec& iv)
    : _initialized(false)
    , _encrypt(false)
    , _cipher(nullptr)
    , _ctx(EVP_CIPHER_CTX_new())
    , _name(name)
    , _key(key)
    , _iv(iv)
{
    crypto::initializeEngine();

    _cipher = EVP_get_cipherbyname(name.c_str());
    if (!_cipher)
        throw std::invalid_argument("Not found: Cipher " + name +
                                    " is unavailable");
}


Cipher::Cipher(const std::string& name)
    : _initialized(false)
    , _encrypt(false)
    , _cipher(nullptr)
    , _ctx(EVP_CIPHER_CTX_new())
    , _name(name)
    , _key()
    , _iv()
{
    crypto::initializeEngine();

    _cipher = EVP_get_cipherbyname(name.c_str());
    if (!_cipher)
        throw std::invalid_argument("Not found: Cipher " + name +
                                    " is unavailable");

    _key = ByteVec(keySize());
    _iv = ByteVec(ivSize());
    setRandomKey();
    setRandomIV();
}


Cipher::~Cipher()
{
    crypto::uninitializeEngine();

    if (_initialized)
        EVP_CIPHER_CTX_cleanup(_ctx);
    EVP_CIPHER_CTX_free(_ctx);
}


void Cipher::initEncryptor()
{
    init(true);
}


void Cipher::initDecryptor()
{
    init(false);
}


void Cipher::init(bool encrypt)
{
    if (_initialized)
        EVP_CIPHER_CTX_cleanup(_ctx);

    EVP_CipherInit(_ctx, _cipher, &_key[0], _iv.empty() ? 0 : &_iv[0],
                   encrypt ? 1 : 0);

    _encrypt = encrypt;
    _initialized = true;
}


ssize_t Cipher::update(const unsigned char* input, size_t inputLength,
                       unsigned char* output, size_t outputLength)
{
    assert(outputLength >= (inputLength + blockSize() - 1));
    int len;
    internal::api(EVP_CipherUpdate(_ctx, output, &len, input, (int)inputLength));
    return (ssize_t)len;
}


ssize_t Cipher::final(unsigned char* output, size_t length)
{
    assert(length >= (size_t)blockSize());
    int len;
    internal::api(EVP_CipherFinal_ex(_ctx, output, &len));
    return (ssize_t)len;
}


inline basic::Encoder* createEncoder(Cipher::Encoding encoding)
{
    switch (encoding) {
        case Cipher::Binary:
            return nullptr;

        case Cipher::Base64:
            return new base64::Encoder();

        case Cipher::Base64_NoLF: {
            base64::Encoder* benc = new base64::Encoder();
            benc->setLineLength(0);
            return benc;
        }
        case Cipher::BinHex:
            return new hex::Encoder();

        case Cipher::BinHex_NoLF: {
            hex::Encoder* henc = new hex::Encoder();
            henc->setLineLength(0);
            return henc;
        }
        default:
            throw std::invalid_argument("Invalid cypher encoding method");
    }
}


ssize_t Cipher::encrypt(const unsigned char* inbuf, size_t inlen,
                        unsigned char* outbuf, size_t outlen,
                        Encoding encoding)
{
    initEncryptor();

    ssize_t reslen, nwrite = 0;
    std::unique_ptr<basic::Encoder> encoder(createEncoder(encoding));
    std::unique_ptr<unsigned char[]> cryptbuf(
        encoder ? new unsigned char[outlen] : nullptr);

    // Encrypt and then encode to outbuf
    if (encoder) {
        reslen = update(inbuf, inlen, cryptbuf.get(), outlen);
        nwrite += encoder.get()->encode((const char*)cryptbuf.get(), reslen,
                                        (char*)outbuf + nwrite);
    }

    // Encrypt direct to outbuf
    else {
        reslen = update(inbuf, inlen, outbuf + nwrite, outlen);
        nwrite += reslen;
    }

    // Finalize
    if (encoder) {
        reslen = final(cryptbuf.get(), outlen);
        nwrite += encoder.get()->encode((const char*)cryptbuf.get(), reslen,
                                        (char*)outbuf + nwrite);
        nwrite += encoder.get()->finalize((char*)outbuf + nwrite);
    } else {
        reslen = update(inbuf, inlen, outbuf, outlen);
        nwrite += reslen;
    }

    return nwrite;
}


std::string Cipher::encryptString(const std::string& str, Encoding encoding)
{
#if 0 // fixme for faster encoding
    const int N = std::max<int>(str.length() + blockSize(), str.length() * 2);
    assert(N >= (str.length() + blockSize() - 1));

    std::unique_ptr<char[]> outbuf(new char[N]);
    int len = encrypt(
        reinterpret_cast<const unsigned char*>(&str[0]), str.length(),
        reinterpret_cast<unsigned char*>(outbuf.get()), N,
        encoding);
    return std::string(outbuf.get(), len);
#endif

    std::string res;
    std::istringstream source(str);
    std::ostringstream sink;
    encryptStream(source, sink, encoding);
    return sink.str();
}


std::string Cipher::decryptString(const std::string& str, Encoding encoding)
{
    std::istringstream source(str);
    std::ostringstream sink;

    decryptStream(source, sink, encoding);

    return sink.str();
}


void Cipher::encryptStream(std::istream& source, std::ostream& sink, Encoding encoding)
{
    initEncryptor();

    const int N = blockSize() * 128;
    size_t cryptsize = N * 2;
    ssize_t nread = N;
    ssize_t reslen = 0;
    ssize_t enclen = 0;

    std::unique_ptr<basic::Encoder> encoder(createEncoder(encoding));
    std::unique_ptr<unsigned char[]> readbuf(new unsigned char[N]);
    std::unique_ptr<unsigned char[]> cryptbuf(new unsigned char[cryptsize]);
    std::unique_ptr<char[]> encbuf(encoder ? new char[cryptsize * 2] : nullptr);

    do {
        source.read((char*)readbuf.get(), nread);
        nread = static_cast<int>(source.gcount());

        reslen = update(readbuf.get(), nread, cryptbuf.get(), cryptsize);
        if (encoder) {
            enclen = encoder.get()->encode((const char*)cryptbuf.get(), reslen, encbuf.get());
            sink.write((const char*)encbuf.get(), enclen);
        } else {
            sink.write((const char*)cryptbuf.get(), reslen);
        }
    } while (source.good() && nread > 0);

    reslen = final(cryptbuf.get(), cryptsize);
    if (encoder) {
        enclen = encoder.get()->encode((const char*)cryptbuf.get(), reslen, encbuf.get());
        sink.write((const char*)encbuf.get(), enclen);
        enclen = encoder.get()->finalize(encbuf.get());
        sink.write((const char*)encbuf.get(), enclen);
    } else {
        sink.write((const char*)cryptbuf.get(), reslen);
    }
}


inline basic::Decoder* createDecoder(Cipher::Encoding encoding)
{
    switch (encoding) {
        case Cipher::Binary:
            return nullptr;

        case Cipher::Base64:
        case Cipher::Base64_NoLF:
            return new base64::Decoder();

        case Cipher::BinHex:
        case Cipher::BinHex_NoLF:
            return new hex::Decoder();

        default:
            throw std::invalid_argument("Invalid cypher decoding method");
    }
}


void Cipher::decryptStream(std::istream& source, std::ostream& sink, Encoding encoding)
{
    initDecryptor();

    const int N = blockSize() * 128;
    size_t cryptsize = N * 2; // must be bigger than N, see update()
    ssize_t nread = N;
    ssize_t reslen = 0;
    ssize_t declen = 0;

    std::unique_ptr<basic::Decoder> decoder(createDecoder(encoding));
    std::unique_ptr<unsigned char[]> readbuf(new unsigned char[N]);
    std::unique_ptr<unsigned char[]> cryptbuf(new unsigned char[cryptsize]);
    std::unique_ptr<char[]> decbuf(decoder ? new char[cryptsize * 2] : nullptr);

    do {
        source.read((char*)readbuf.get(), nread);
        nread = static_cast<int>(source.gcount());

        if (decoder) {
            declen = decoder->decode((const char*)readbuf.get(), nread, decbuf.get());
            if (declen == 0)
                continue;
            reslen = update((const unsigned char*)decbuf.get(), declen,
                            cryptbuf.get(), cryptsize);
            sink.write((const char*)cryptbuf.get(), reslen);
        } else {
            reslen = update(readbuf.get(), nread, cryptbuf.get(), cryptsize);
            sink.write((const char*)cryptbuf.get(), reslen);
        }
    } while (source.good() && nread > 0);

    if (decoder) {
        declen = decoder->finalize(decbuf.get());
        if (declen) {
            reslen = update((const unsigned char*)decbuf.get(), declen,
                            cryptbuf.get(), cryptsize);
            if (reslen)
                sink.write((const char*)cryptbuf.get(), reslen);
        }
    }

    reslen = final(cryptbuf.get(), cryptsize);
    if (reslen)
        sink.write((const char*)cryptbuf.get(), reslen);
}


int Cipher::setPadding(int padding)
{
    return EVP_CIPHER_CTX_set_padding(_ctx, padding);
}


int Cipher::keySize() const
{
    return EVP_CIPHER_key_length(_cipher);
}


const ByteVec& Cipher::getKey() const
{
    return _key;
}


const ByteVec& Cipher::getIV() const
{
    return _iv;
}


int Cipher::blockSize() const
{
    return EVP_CIPHER_block_size(_cipher);
}


int Cipher::ivSize() const
{
    return EVP_CIPHER_iv_length(_cipher);
}


inline void getRandomBytes(ByteVec& vec, size_t count)
{
    Random rnd;
    rnd.seed();

    vec.clear();
    vec.reserve(count);

    for (unsigned i = 0; i < count; ++i)
        vec.push_back(rnd.nextChar());
}


void Cipher::setRandomIV()
{
    getRandomBytes(_iv, ivSize());
}


void Cipher::setRandomKey()
{
    getRandomBytes(_key, keySize());
}


void Cipher::generateKey(const std::string& password, const std::string& salt,
                         int iterationCount)
{
    unsigned char keyBytes[EVP_MAX_KEY_LENGTH];
    unsigned char ivBytes[EVP_MAX_IV_LENGTH];

    // OpenSSL documentation specifies that the salt must be an 8-byte array.
    unsigned char saltBytes[8];

    if (!salt.empty()) {
        int len = static_cast<int>(salt.size());
        // Create the salt array from the salt string
        for (int i = 0; i < 8; ++i)
            saltBytes[i] = salt.at(i % len);
        for (int i = 8; i < len; ++i)
            saltBytes[i % 8] ^= salt.at(i);
    }

    // Now create the key and IV, using the MD5 digest algorithm.
    int keySize = EVP_BytesToKey(
        _cipher, EVP_md5(), (salt.empty() ? 0 : saltBytes),
        reinterpret_cast<const unsigned char*>(password.data()),
        static_cast<int>(password.size()), iterationCount, keyBytes, ivBytes);

    // Copy the buffers to our member byte vectors.
    _key.assign(keyBytes, keyBytes + keySize);

    if (ivSize() == 0)
        _iv.clear();
    else
        _iv.assign(ivBytes, ivBytes + ivSize());
}


} // namespace crypto
} // namespace base

