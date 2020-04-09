#ifndef SFU_LOGGER_TAG
#define SFU_LOGGER_TAG

#include "base/logger.h"
#include "Settings.h"
#include <cstdio>  // std::snprintf(), std::fprintf(), stdout, stderr
#include <cstdlib> // std::abort()
#include <cstring>


#include <inttypes.h>
#include <cassert>
#define assertm(exp, msg) assert(((void)msg, exp))


#define MS_UINT16_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c"
#define MS_UINT16_TO_BINARY(value) \
    ((value & 0x8000) ? '1' : '0'), \
    ((value & 0x4000) ? '1' : '0'), \
    ((value & 0x2000) ? '1' : '0'), \
    ((value & 0x1000) ? '1' : '0'), \
    ((value & 0x800) ? '1' : '0'), \
    ((value & 0x400) ? '1' : '0'), \
    ((value & 0x200) ? '1' : '0'), \
    ((value & 0x100) ? '1' : '0'), \
    ((value & 0x80) ? '1' : '0'), \
    ((value & 0x40) ? '1' : '0'), \
    ((value & 0x20) ? '1' : '0'), \
    ((value & 0x10) ? '1' : '0'), \
    ((value & 0x08) ? '1' : '0'), \
    ((value & 0x04) ? '1' : '0'), \
    ((value & 0x02) ? '1' : '0'), \
    ((value & 0x01) ? '1' : '0')

#define _MS_TAG_ENABLED(tag) Settings::configuration.logTags.tag
#define _MS_TAG_ENABLED_2(tag1, tag2) (Settings::configuration.logTags.tag1 || Settings::configuration.logTags.tag2)



using namespace base;

#define MS_WARN_TAG(tag,  ...) if(_MS_TAG_ENABLED(tag)) { LogStream(Level::Trace, _fileName(__FILE__), __LINE__).write(__VA_ARGS__); }

#define MS_DEBUG_2TAGS(tag1,tag2, ...) if(_MS_TAG_ENABLED_2(tag1, tag2)) { LogStream(Level::Trace, _fileName(__FILE__), __LINE__).write(__VA_ARGS__); }

#define MS_WARN_2TAGS(tag1,tag2, ...) if(_MS_TAG_ENABLED_2(tag1, tag2)) { LogStream(Level::Trace, _fileName(__FILE__), __LINE__).write(__VA_ARGS__); }

#define MS_DUMP(...)

#define MS_HAS_DEBUG_TAG(tag1)  false

#define MS_ERROR(...)  { LogStream(Level::Trace, _fileName(__FILE__), __LINE__).write(__VA_ARGS__); }
#define MS_DEBUG_DEV( ...) { LogStream(Level::Trace, _fileName(__FILE__), __LINE__).write(__VA_ARGS__); }

#define MS_DEBUG_TAG(tag, ...) if(_MS_TAG_ENABLED(tag)) { LogStream(Level::Trace, _fileName(__FILE__), __LINE__).write(__VA_ARGS__); }

#define MS_WARN_DEV( ...) { LogStream(Level::Trace, _fileName(__FILE__), __LINE__).write(__VA_ARGS__); }

#define MS_ABORT(...)

//LogStream(Level::Trace, _fileName(__FILE__), __LINE__).write(__VA_ARGS__); std::abort();
  


#define MS_ERROR_STD(...) { LogStream(Level::Trace, _fileName(__FILE__), __LINE__).write(__VA_ARGS__); }



#define MS_TRACE() 
#define MS_TRACE_STD()


#endif
