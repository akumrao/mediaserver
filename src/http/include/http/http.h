

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

