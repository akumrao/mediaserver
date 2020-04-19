/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */


#ifndef Request_H
#define Request_H


#include "base/base.h"
#include "base/collection.h"
#include "http/message.h"

#include <sstream>


namespace base {
namespace net {


/// HTTP request methods
struct Method
{
    static const std::string Get;
    static const std::string Head;
    static const std::string Put;
    static const std::string Post;
    static const std::string Options;
    static const std::string Delete;
    static const std::string Trace;
    static const std::string Connect;
};


/// This class encapsulates an HTTP request message.
///
/// In addition to the properties common to all HTTP messages,
/// a HTTP request has a method (e.g. GET, HEAD, POST, etc.) and
/// a request URI.
class  Request : public Message
{
public:
    /// Creates a GET / HTTP/1.1 HTTP request.
    Request();

    /// Creates a GET / HTTP/1.x request with
    /// the given version (HTTP/1.0 or HTTP/1.1).
    Request(const std::string& version);

    /// Creates a HTTP/1.0 request with the given method and URI.
    Request(const std::string& method, const std::string& uri);

    /// Creates a HTTP request with the given method, URI and version.
    Request(const std::string& method, const std::string& uri, const std::string& version);

    /// Destroys the Request.
    virtual ~Request();

    /// Sets the method.
    void setMethod(const std::string& method);

    /// Returns the method.
    const std::string& getMethod() const;

    /// Sets the request URI.
    void setURI(const std::string& uri);

    /// Returns the request URI.
    const std::string& getURI() const;

    /// Sets the value of the Host header field.
    void setHost(const std::string& host);

    /// Sets the value of the Host header field.
    ///
    /// If the given port number is a non-standard
    /// port number (other than 80 or 443), it is
    /// included in the Host header field.
    void setHost(const std::string& host, uint16_t port);

    /// Returns the value of the Host header field.
    ///
    /// Throws a NotFoundException if the request
    /// does not have a Host header field.
    const std::string& getHost() const;

    /// Adds a Cookie header with the names and
    /// values from cookies.
    void setCookies(const NVCollection& cookies);

    /// Fills cookies with the cookies extracted
    /// from the Cookie headers in the request.
    void getCookies(NVCollection& cookies) const;

    /// Returns the request URI parameters.
    void getURIParameters(NVCollection& params) const;

    /// Returns true if the request contains authentication
    /// information in the form of an Authorization header.
    bool hasCredentials() const;

    /// Returns the authentication scheme and additional authentication
    /// information contained in this request.
    ///
    /// Throws a std::exception if no authentication information
    /// is contained in the request.
    void getCredentials(std::string& scheme, std::string& authInfo) const;

    /// Sets the authentication scheme and information for
    /// this request.
    void setCredentials(const std::string& scheme, const std::string& authInfo);

    /// Returns true if the request contains proxy authentication
    /// information in the form of an Proxy-Authorization header.
    bool hasProxyCredentials() const;

    /// Returns the proxy authentication scheme and additional proxy
    /// authentication
    /// information contained in this request.
    ///
    /// Throws a std::exception if no proxy authentication information
    /// is contained in the request.
    void getProxyCredentials(std::string& scheme, std::string& authInfo) const;

    /// Sets the proxy authentication scheme and information for this request.
    void setProxyCredentials(const std::string& scheme,
                             const std::string& authInfo);

    /// Writes the HTTP request to the given output stream.
    void write(std::ostream& ostr) const override;

    /// Writes the HTTP request to the given output string.
    void write(std::string& str) const override;

    friend std::ostream& operator<<(std::ostream& stream, const Request& req)
    {
        req.write(stream);
        return stream;
    }

protected:
    /// Returns the authentication scheme and additional authentication
    /// information contained in the given header of request.
    ///
    /// Throws a NotAuthenticatedException if no authentication information
    /// is contained in the request.
    void getCredentials(const std::string& header, std::string& scheme, std::string& authInfo) const;

    /// Writes the authentication scheme and information for
    /// this request to the given header.
    void setCredentials(const std::string& header, const std::string& scheme, const std::string& authInfo);

private:
    std::string _method;
    std::string _uri;
};


} // namespace net
} // namespace base


#endif //Request_H

