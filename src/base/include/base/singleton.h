/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */



#ifndef base_Singleton_H
#define base_Singleton_H


#include <cstdint>
#include <mutex>


namespace base {


/// This is a helper template class for managing
/// singleton objects allocated on the heap.
template <class S>
class Singleton
{
public:
    /// Creates the Singleton wrapper.
    Singleton()
        : _ptr(0)
    {
    }

    /// Destroys the Singleton wrapper and the managed
    /// singleton instance it holds.
    ~Singleton()
    {
        if (_ptr)
            delete _ptr;
    }

    /// Returns a pointer to the singleton object hold by the Singleton.
    /// The first call to get on a nullptr singleton will instantiate
    /// the singleton.
    S* get()
    {
        std::lock_guard<std::mutex> guard(_m);
        if (!_ptr)
            _ptr = new S;
        return _ptr;
    }

    /// Swaps the old pointer with the new one and returns the old instance.
    S* swap(S* newPtr)
    {
        std::lock_guard<std::mutex> guard(_m);
        S* oldPtr = _ptr;
        _ptr = newPtr;
        return oldPtr;
    }

    /// Destroys the managed singleton instance.
    void destroy()
    {
        std::lock_guard<std::mutex> guard(_m);
        if (_ptr)
            delete _ptr;
        _ptr = nullptr;
    }

private:
    S* _ptr;
    std::mutex _m;
};


} // namespace base


#endif // base_Singleton_H


