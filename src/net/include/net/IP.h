#ifndef MS_UTILS_HPP
#define MS_UTILS_HPP

#include <uv.h>
#include <string>

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
