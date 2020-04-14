

#include "net/IP.h"
#include "base/logger.h"
namespace base
{
    namespace net
    {
     
	void IP::NormalizeIp(std::string& ip)
	{
		

		static sockaddr_storage addrStorage;
		char ipBuffer[INET6_ADDRSTRLEN] = { 0 };
		int err;

		switch (IP::GetFamily(ip))
		{
			case AF_INET:
			{
				err = uv_ip4_addr(
				  ip.c_str(),
				  0,
				  reinterpret_cast<struct sockaddr_in*>(&addrStorage));

				if (err != 0)
					MS_ABORT("uv_ip4_addr() failed: %s", uv_strerror(err));

				err = uv_ip4_name(
					reinterpret_cast<const struct sockaddr_in*>(std::addressof(addrStorage)),
					ipBuffer,
					sizeof(ipBuffer));

				if (err != 0)
					MS_ABORT("uv_ipv4_name() failed: %s", uv_strerror(err));

				ip.assign(ipBuffer);

				break;
			}

			case AF_INET6:
			{
				err = uv_ip6_addr(
					ip.c_str(),
					0,
				  reinterpret_cast<struct sockaddr_in6*>(&addrStorage));

				if (err != 0)
					MS_ABORT("uv_ip6_addr() failed: %s", uv_strerror(err));

				err = uv_ip6_name(
					reinterpret_cast<const struct sockaddr_in6*>(std::addressof(addrStorage)),
					ipBuffer,
					sizeof(ipBuffer));

				if (err != 0)
					MS_ABORT("uv_ip6_name() failed: %s", uv_strerror(err));

				ip.assign(ipBuffer);

				break;
			}

			default:
			{
				base::uv::throwError("invalid ip " +  ip );
			}
		}
	}
        

        void IP::GetAddressInfo(struct sockaddr* addr, int& family, std::string& ip, uint16_t& port) {


            char ipBuffer[INET6_ADDRSTRLEN + 1];
            int err;

           
            
            switch (addr->sa_family)
            {
                case AF_INET:
                {
                    err = uv_inet_ntop(
                            AF_INET, std::addressof(reinterpret_cast<const struct sockaddr_in*> (addr)->sin_addr), ipBuffer, INET_ADDRSTRLEN);

                    if (err)
                        LError("uv_inet_ntop() failed: %s", uv_strerror(err));

                    port = static_cast<uint16_t> (ntohs(reinterpret_cast<const struct sockaddr_in*> (addr)->sin_port));

                    break;
                }

                case AF_INET6:
                {
                    err = uv_inet_ntop(
                            AF_INET, std::addressof(reinterpret_cast<const struct sockaddr_in6*> (addr)->sin6_addr), ipBuffer, INET_ADDRSTRLEN);

                    if (err)
                        LError("uv_inet_ntop() failed: %s", uv_strerror(err));

                    port = static_cast<uint16_t> (ntohs(reinterpret_cast<const struct sockaddr_in6*> (addr)->sin6_port));

                    break;
                }

                default:
                {
                    LError("unknown network family: %d", static_cast<int> (addr->sa_family));
                }
            }

            family = addr->sa_family;
            ip.assign(ipBuffer);
        }

        int IP::GetFamily(const std::string& ip) {

            char ia[sizeof (struct in6_addr)];
            if (uv_inet_pton(AF_INET, ip.c_str(), &ia) == 0)

                return AF_INET;
            else if (uv_inet_pton(AF_INET6, ip.c_str(), &ia) == 0)
                return AF_INET6;
            else
                throw std::runtime_error("Invalid IP address format: " + ip);
        }


    } // namespace net
}//base
