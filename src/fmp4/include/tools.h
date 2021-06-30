#ifndef TOOLS_HEADER_GUARD 
#define TOOLS_HEADER_GUARD


#define BIG_ENDIAN 1
//
#include <chrono>
#include <cstdint>

inline uint64_t CurrentTime_milliseconds() 
{
    return std::chrono::duration_cast<std::chrono::milliseconds>
              (std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

inline uint64_t CurrentTime_microseconds() 
{
    return std::chrono::duration_cast<std::chrono::microseconds>
              (std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

inline uint64_t CurrentTime_nanoseconds()
{
    return std::chrono::duration_cast<std::chrono::nanoseconds>
              (std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}  



#ifdef BIG_ENDIAN
inline uint32_t deserialize_uint32_big_endian(unsigned char *buffer)
{
    uint32_t value = 0;

    value |= buffer[0] << 24;
    value |= buffer[1] << 16;
    value |= buffer[2] << 8;
    value |= buffer[3];
    return value;
}
#else // either not defined or little endian
// deserialize value from big endian in little endian system
// byte1 byte2 byte3 byte4 => byte4 byte3 ..
inline uint32_t deserialize_uint32_big_endian(unsigned char *buffer)
{
    uint32_t value = 0;

    value |= buffer[3] << 24;
    value |= buffer[2] << 16;
    value |= buffer[1] << 8;
    value |= buffer[0];
    return value;
}
#endif

/*
// this was removed from live555 at some point
unsigned our_inet_addr(const char* cp)
{
        return inet_addr(cp);
}
*/

#endif
