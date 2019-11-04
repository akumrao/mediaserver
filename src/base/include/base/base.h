

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
