

#ifndef Interfaces_H
#define Interfaces_H


#include "base/base.h"

#include "uv.h" // ssize_t

#include <atomic>
#include <functional>
#include <memory>
#include <stdexcept>


namespace base {


/// Interface classes
namespace basic {


class Base_API Decoder
{
public:
    Decoder() = default;
    virtual ~Decoder() = default;

    virtual ssize_t decode(const char* inbuf, size_t nread, char* outbuf) = 0;
    virtual ssize_t finalize(char* /* outbuf */) { return 0; }
};


class Base_API Encoder
{
public:
    Encoder() = default;
    virtual ~Encoder() = default;
    virtual ssize_t encode(const char* inbuf, size_t nread, char* outbuf) = 0;
    virtual ssize_t finalize(char* /* outbuf */) { return 0; }
};



} // namespace basic
} // namespace base


#endif // Interfaces_H
