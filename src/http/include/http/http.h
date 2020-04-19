/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */



#ifndef DEF_HTTP_h
#define DEF_HTTP_h


#include "base/base.h"


// Shared library exports
#if defined(base_WIN) && defined(base_SHARED_LIBRARY)
    #if defined(HTTP_EXPORTS)
        #define HTTP_API __declspec(dllexport)
    #else
        #define HTTP_API __declspec(dllimport)
    #endif
#else
    #define HTTP_API // nothing
#endif


#endif // DEF_HTTP_h

