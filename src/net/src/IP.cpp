

#include "net/IP.h"
#include "base/logger.h"
#include <cstring> // std::memcmp(), std::memcpy()
namespace base
{
    namespace net
    {
     
	

	/* Inline static methods. */

	
	bool IP::CompareAddresses(const struct sockaddr* addr1, const struct sockaddr* addr2)
	{
		// Compare family.
		if (addr1->sa_family != addr2->sa_family || (addr1->sa_family != AF_INET && addr1->sa_family != AF_INET6))
		{
			return false;
		}

		// Compare port.
		if (
		  reinterpret_cast<const struct sockaddr_in*>(addr1)->sin_port !=
		  reinterpret_cast<const struct sockaddr_in*>(addr2)->sin_port)
		{
			return false;
		}

		// Compare IP.
		switch (addr1->sa_family)
		{
			case AF_INET:
			{
				return (
				  reinterpret_cast<const struct sockaddr_in*>(addr1)->sin_addr.s_addr ==
				  reinterpret_cast<const struct sockaddr_in*>(addr2)->sin_addr.s_addr);
			}

			case AF_INET6:
			{
				return (
				  std::memcmp(
				    std::addressof(reinterpret_cast<const struct sockaddr_in6*>(addr1)->sin6_addr),
				    std::addressof(reinterpret_cast<const struct sockaddr_in6*>(addr2)->sin6_addr),
				    16) == 0
				    ? true
				    : false);
			}

			default:
			{
				return false;
			}
		}
	}

	struct sockaddr_storage IP::CopyAddress(const struct sockaddr* addr)
	{
		struct sockaddr_storage copiedAddr;

		switch (addr->sa_family)
		{
			case AF_INET:
				std::memcpy(std::addressof(copiedAddr), addr, sizeof(struct sockaddr_in));
				break;

			case AF_INET6:
				std::memcpy(std::addressof(copiedAddr), addr, sizeof(struct sockaddr_in6));
				break;
		}

		return copiedAddr;
	}

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
					 uv::throwError("uv_ip4_addr() failed: " , err);

				err = uv_ip4_name(
					reinterpret_cast<const struct sockaddr_in*>(std::addressof(addrStorage)),
					ipBuffer,
					sizeof(ipBuffer));

				if (err != 0)
					 uv::throwError("uv_ipv4_name() failed: ", err);

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
					 uv::throwError("uv_ip6_addr() failed: ", err);

				err = uv_ip6_name(
					reinterpret_cast<const struct sockaddr_in6*>(std::addressof(addrStorage)),
					ipBuffer,
					sizeof(ipBuffer));

				if (err != 0)
					 uv::throwError("uv_ip6_name() failed: ", err);

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
