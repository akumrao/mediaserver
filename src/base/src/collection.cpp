/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */




#include "base/collection.h"


namespace base {


NVCollection& NVCollection::operator=(const NVCollection& nvc)
{
    if (&nvc != this) {
        _map = nvc._map;
    }
    return *this;
}


const std::string& NVCollection::operator[](const std::string& name) const
{
    ConstIterator it = _map.find(name);
    if (it != _map.end())
        return it->second;
    else
        throw std::runtime_error("Item not found: " + name);
}


void NVCollection::set(const std::string& name, const std::string& value)
{
    Iterator it = _map.find(name);
    if (it != _map.end())
        it->second = value;
    else
        _map.insert(Map::value_type(name, value));
}


void NVCollection::add(const std::string& name, const std::string& value)
{
    _map.insert(Map::value_type(name, value));
}


const std::string& NVCollection::get(const std::string& name) const
{
    ConstIterator it = _map.find(name);
    if (it != _map.end())
        return it->second;
    else
        throw std::runtime_error("Item not found: " + name);
}


const std::string& NVCollection::get(const std::string& name,
    const std::string& defaultValue) const
{
    ConstIterator it = _map.find(name);
    if (it != _map.end())
        return it->second;
    else
        return defaultValue;
}


bool NVCollection::has(const std::string& name) const
{
    return _map.find(name) != _map.end();
}


NVCollection::ConstIterator NVCollection::find(const std::string& name) const
{
    return _map.find(name);
}


NVCollection::ConstIterator NVCollection::begin() const
{
    return _map.begin();
}


NVCollection::ConstIterator NVCollection::end() const
{
    return _map.end();
}


bool NVCollection::empty() const
{
    return _map.empty();
}


int NVCollection::size() const
{
    return (int)_map.size();
}


void NVCollection::erase(const std::string& name)
{
    _map.erase(name);
}


void NVCollection::clear()
{
    _map.clear();
}


} // namespace base


