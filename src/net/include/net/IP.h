#ifndef MS_UTILS_HPP
#define MS_UTILS_HPP

#include <uv.h>
#include <string>
 #include <stdlib.h>

#define ASSERT(expr)                                      \
 do {                                                     \
  if (!(expr)) {                                          \
    fprintf(stderr,                                       \
            "Assertion failed in %s on line %d: %s\n",    \
            __FILE__,                                     \
            __LINE__,                                     \
            #expr);                                       \
    abort();                                              \
  }                                                       \
 } while (0)


namespace base
{

    namespace net
    {

        class IP
        {
        public:
            static void GetAddressInfo(struct sockaddr* addr, int& family, std::string& ip, uint16_t& port);
            static int GetFamily(const std::string& ip);

        };


    } // namespace net
} // base
#endif
