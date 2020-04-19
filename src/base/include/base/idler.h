/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */



#ifndef base_IDLER_H
#define base_IDLER_H



#include "base/base.h"
#include <functional>
#include "uv.h"

namespace base {

///
class Idler 
{
public:
    /// Create the idler with the given event loop.
    Idler();

    /// Create and start the idler with the given callback.
    Idler(std::function<void()> cbfun);

    std::function<void()> cbfun;
   
    /// Start the idler with the given callback function.
    virtual void start();

    virtual ~Idler();

    virtual void stop();
    virtual void run();

protected:

    uv_idle_t idler;
};


} // namespace scy


#endif // SCY_Idler_H

