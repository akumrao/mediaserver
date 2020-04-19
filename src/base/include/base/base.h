/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */



#ifndef base_H
#define base_H


// Include build config
#include "define.h"

// Shared library exports
#if defined(base_WIN) && defined(base_SHARED_LIBRARY)
    #if defined(Base_EXPORTS)
        #define Base_API __declspec(dllexport)
    #else
        #define Base_API __declspec(dllimport)
    #endif
#else
    #define Base_API // nothing
#endif


#endif // base_H
