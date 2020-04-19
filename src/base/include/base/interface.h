/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */



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
