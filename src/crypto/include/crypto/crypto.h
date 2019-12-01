

#ifndef Crypto_Crypto_H
#define Crypto_Crypto_H


#include "base/base.h"
#include <string>
#include <vector>

#ifdef base_WIN 
#include <winsock2.h>
#include <windows.h>

// Undefine the following definitions defined in wincrypt.h
// as they conflict with BoringSSL
#undef X509_NAME 
#undef X509_CERT_PAIR 
#undef X509_EXTENSIONS 
#endif 

/*
// Shared library exports
#if defined(base_WIN) && defined(base_SHARED_LIBRARY)
    #if defined(Crypto_EXPORTS)
        #define Crypto_API __declspec(dllexport)
    #else
        #define Crypto_API __declspec(dllimport)
    #endif
#else
    #define Crypto_API // nothing
#endif
*/
#define Crypto_API 

namespace base {
namespace crypto {


/// Initialize the Crypto library, as well as the underlying OpenSSL
/// libraries.
///
/// OpenSSL must be initialized before using any classes from the
/// Crypto library. OpenSSL will be initialized automatically
/// through OpenSSL instances held by various Crypto classes
/// (Cipher, Hash, X509Certificate), however it is recommended to
/// call initializeEngine() in any case at application startup.
///
/// The Crypto library can be called multiple times; however,
/// for every call to initializeEngine(), a matching call to
/// uninitializeEngine() must be performed.
Crypto_API void initializeEngine();

/// Uninitializes the Crypto library.
Crypto_API void uninitializeEngine();

/// Generic storage container for storing cryptographic binary data.
typedef std::vector<unsigned char> ByteVec;

namespace internal {

/// Check return values from OpenSSL and throw an exception if it failed.
void api(int ret, const char* error = nullptr);

/// Throws the last OpenSSL error.
void throwError();

/// Allows template functions to accept a wide range of buffer types.
/// See constructor definitions below for all compatible types.
/// The class uses const_cast for maximum flexibility, so use with care.
/// Also ensure that std::string is contiguous on your platform
/// before using the std::string constructors (C++11 guarantees it).
template <typename T> struct Raw
{
    T ptr;
    size_t len;

    Raw(T ptr, size_t len)
    {
        ptr = ptr;
        len = len;
    }

    Raw(const char* ptr, size_t len)
    {
        ptr = reinterpret_cast<T>(const_cast<char*>(ptr));
        len = len;
    }

    Raw(std::string& str)
    {
        ptr = reinterpret_cast<T>(&str[0]);
        len = str.length();
    }

    Raw(const std::string& str)
    {
        ptr = reinterpret_cast<T>(const_cast<char*>(&str[0]));
        len = str.length();
    }

    Raw(const std::vector<char>& vec)
    {
        ptr = reinterpret_cast<T>(const_cast<char*>(&vec[0]));
        len = vec.size();
    }

    Raw(const ByteVec& vec)
    {
        ptr = reinterpret_cast<T>(const_cast<unsigned char*>(&vec[0]));
        len = vec.size();
    }
};

} // namespace internal


} // namespace crypto
} // namespace base


#endif 



