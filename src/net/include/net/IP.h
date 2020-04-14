#ifndef NET_UTILS_IP
#define NET_UTILS_IP

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
	    static void NormalizeIp(std::string& ip);
	    static bool CompareAddresses(const struct sockaddr* addr1, const struct sockaddr* addr2);
	    static struct sockaddr_storage CopyAddress(const struct sockaddr* addr);
	  
        };


    } // namespace net
} // base
#endif
