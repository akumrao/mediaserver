/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */




#ifndef HTTP_Util_h
#define HTTP_Util_h


#include "base/base.h"
#include "base/collection.h"


namespace base {
    namespace net {


        /// Parses the URI part from a HTTP request.
        std::string parseURI(const std::string& request);

        /// Parses the URI against the expression.
        bool matchURL(const std::string& uri, const std::string& expression);

        /// Parses the URI against the expression.
        bool matchURL(const std::string& uri, const std::string& expression);

        /// Parses an item from a HTTP cookie.
        std::string parseCookieItem(const std::string& cookie, const std::string& item);

        /// Splits and adds the URI query parameters into the given collection.
        bool splitURIParameters(const std::string& uri, NVCollection& out);

        /// Splits the given std::string into a value and a collection of parameters.
        /// Parameters are expected to be separated by semicolons.
        ///
        /// Enclosing quotes of parameter values are removed.
        ///
        /// For example, the std::string
        ///   multipart/mixed; boundary="boundary-01234567"
        /// is split into the value
        ///   multipart/mixed
        /// and the parameter
        ///   boundary -> boundary-01234567
        void splitParameters(const std::string& s, std::string& value,
                NVCollection& parameters);

        /// Splits the given std::string into a collection of parameters.
        /// Parameters are expected to be separated by semicolons.
        ///
        /// Enclosing quotes of parameter values are removed.
        void splitParameters(const std::string::const_iterator& begin,
                const std::string::const_iterator& end,
                NVCollection& parameters);


    } // namespace net
} // namespace base


#endif // HTTP_Util_h



