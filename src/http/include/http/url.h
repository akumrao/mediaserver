/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */



#ifndef HTTP_URL_H
#define HTTP_URL_H


#include "http/http.h"
#include "base/collection.h"
#include <http_parser.h>


namespace base {
namespace net {


/// An RFC 3986 based URL which uses an external c
/// library to do the heavy lifting.
/// Constructors and assignment operators will throw
/// a SyntaxException if the URL is invalid.
class  URL
{
public:
    URL();
    URL(const char* url);
    URL(const std::string& url);
    URL(const std::string& scheme, const std::string& authority);
    URL(const std::string& scheme, const std::string& authority,
        const std::string& pathEtc);
    URL(const std::string& scheme, const std::string& authority,
        const std::string& path, const std::string& query,
        const std::string& fragment = "");
    ~URL();

    URL& operator=(const URL& uri);
    URL& operator=(const std::string& uri);
    URL& operator=(const char* uri);

    /// Parses and assigns an URI from the given std::string.
    /// Throws a SyntaxException if whiny is set and the
    /// given url is invalid.
    bool parse(const std::string& url, bool whiny = true);

    /// RFC 3986 based URL encoding based on JavaScript's
    /// encodeURIComponent()
    static std::string encode(const std::string& str);

    /// RFC 3986 based URL decoding based on JavaScript's
    /// decodeURIComponent()
    static std::string decode(const std::string& str);

public:
    std::string scheme() const;
    std::string userInfo() const;
    std::string host() const;
    uint16_t port() const;
    std::string authority() const;
    std::string path() const;
    std::string pathEtc() const;
    std::string query() const;
    std::string fragment() const;

    bool hasSchema() const;
    bool hasUserInfo() const;
    bool hasHost() const;
    bool hasPort() const;
    bool hasPath() const;
    bool hasQuery() const;
    bool hasFragment() const;

    bool valid() const;

    std::string str() const;

    friend std::ostream& operator<<(std::ostream& stream, const URL& url)
    {
        stream << url.str();
        return stream;
    }

protected:
    http_parser_url _parser;
    std::string _buf;
};


} // namespace net
} // namespace base


#endif // HTTP_URL_H

