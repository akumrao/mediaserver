/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */


#include "http/httputil.h"
#include "base/util.h"


// using std::endl;


namespace base {
namespace net {


std::string parseURI(const std::string& request)
{
    std::string req = request;
    std::string value = "";
    std::string::size_type start, end = 0;
    util::toUpper(req);
    start = req.find(" ");
    if (start != std::string::npos) {
        start++;
        end = req.find(" HTTPS", start);
        if (end == std::string::npos)
            end = req.find(" HTTP", start);
        if (end == std::string::npos)
            end = req.find(" RTSP", start);
        if (end == std::string::npos)
            return "";
        value = request.substr(start, end - start);
    }
    return value;
}


bool matchURL(const std::string& uri, const std::string& expression)
{
    std::string::size_type index = uri.find("?");
    return util::matchNodes(uri.substr(0, index), expression, "/");
}


std::string parseCookieItem(const std::string& cookie, const std::string& item)
{
    std::string::size_type start, end = 0;
    start = cookie.find(item + "=");
    if (start != std::string::npos) {
        start += item.size() + 1;
        end = cookie.find(";", start);
        return cookie.substr(start, end - start);
    }
    return "";
}


bool splitURIParameters(const std::string& uri, NVCollection& out)
{
    size_t len = uri.length();
    size_t i = 0;

    // Parse REST parameters
    while (i < len && uri[i] != '?') {
        i++;

        std::string value = "";
        while (uri[i] != '/' && uri[i] != '?' && i < len)
            value += uri[i++];

        // REST parameters are referenced by index
        if (!value.empty())
            out.set(util::itostr(out.size()), value);
    }

    // Parse query parameters
    if (uri[i] == '?')
        i++;
    while (i < len) {
        std::string name = "";
        while (uri[i] != '=' && i < len)
            name += uri[i++];
        i++;
        std::string value = "";
        while (uri[i] != '&' && i < len)
            value += uri[i++];
        i++;

        if (!name.empty() && !value.empty())
            out.set(name, value);
    }

    return out.size() > 0;
}


void splitParameters(const std::string& s, std::string& value,
                     NVCollection& parameters)
{
    value.clear();
    parameters.clear();

    std::string::const_iterator it = s.begin();
    std::string::const_iterator end = s.end();
    while (it != end && ::isspace(*it))
        ++it;
    while (it != end && *it != ';')
        value += *it++;
    util::trimRightInPlace(value);
    if (it != end)
        ++it;

    splitParameters(it, end, parameters);
}


void splitParameters(const std::string::const_iterator& begin,
                     const std::string::const_iterator& end,
                     NVCollection& parameters)
{
    std::string pname;
    std::string pvalue;
    pname.reserve(32);
    pvalue.reserve(64);
    std::string::const_iterator it = begin;
    while (it != end) {
        pname.clear();
        pvalue.clear();
        while (it != end && ::isspace(*it))
            ++it;
        while (it != end && *it != '=' && *it != ';')
            pname += *it++;
        util::trimRightInPlace(pname);
        if (it != end && *it != ';')
            ++it;
        while (it != end && ::isspace(*it))
            ++it;
        while (it != end && *it != ';') {
            if (*it == '"') {
                ++it;
                while (it != end && *it != '"') {
                    if (*it == '\\') {
                        ++it;
                        if (it != end)
                            pvalue += *it++;
                    } else
                        pvalue += *it++;
                }
                if (it != end)
                    ++it;
            } else if (*it == '\\') {
                ++it;
                if (it != end)
                    pvalue += *it++;
            } else
                pvalue += *it++;
        }
        util::trimRightInPlace(pvalue);
        if (!pname.empty())
            parameters.add(pname, pvalue);
        if (it != end)
            ++it;
    }
}


#if 0
string parseHeader(const std::string& request, const std::string& name)
{
    std::string req = request;
    toLower(req);
    toLower(name);
    std::string value = "";
    string::size_type start, end = 0;
    start = req.find(name+": ");
    if (start != std::string::npos) {
        start += name.length() + 2;
        end = req.find("\r\n", start);
        if (end == std::string::npos) return "";
        value = request.substr(start, end-start);
        replaceInPlace(value,"\"","");
        replaceInPlace(value,"\r","");
        replaceInPlace(value,"\n","");
    }
    return value;
}
#endif


} // namespace net
} // namespace base
