/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */


#ifndef base_Error_H
#define base_Error_H


#include "base/base.h"

#include "uv.h"

#include <exception>
#include <stdexcept>
#include <string>


namespace base {


/// Basic error type.
///
/// Errors contain an error code, message, and exception pointer.
struct Error
{
    int err;
    std::string message;
    std::exception_ptr exception;

    Error() { reset(); }

    Error(const std::string& msg)
    {
        reset();
        message = msg;
    }

    Error(const char* msg)
    {
        reset();
        message = msg;
    }

    bool any() const
    {
        return err != 0 || !message.empty() || exception != nullptr;
    }

    void reset()
    {
        err = 0;
        message.clear();
        exception = nullptr;
    }

    void rethrow()
    {
        if (exception)
            std::rethrow_exception(exception);
    }

    friend std::ostream& operator<<(std::ostream& stream, const Error& err)
    {
        stream << err.message;
        return stream;
    }
};


//
// UV Error Helpers
//


namespace uv {


inline std::string formatError(std::string message, int err = UV_UNKNOWN)
{
    if (err != UV_UNKNOWN) {
        if (!message.empty())
            message.append(": ");
        message.append(uv_strerror(err));
    }
    return message;
}


inline void throwError(std::string message, int err = UV_UNKNOWN)
{
    throw std::runtime_error(formatError(std::move(message), err));
}


} // namespace uv
} // namespace base


#endif // base_Error_H


