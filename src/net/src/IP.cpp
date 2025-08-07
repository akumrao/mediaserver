/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */



#include "net/IP.h"
#include "base/logger.h"
#include <cstring> // std::memcmp(), std::memcpy()
namespace base
{
    namespace net
    {
     
	

	/* Inline static methods. */
    
        
        
         bool IP::addr_is_equal(const struct sockaddr *a, const struct sockaddr *b, bool compare_ports)
         {
                if (a->sa_family != b->sa_family)
                        return false;

                switch (a->sa_family) {
                case AF_INET: {
                        const struct sockaddr_in *ain = (const struct sockaddr_in *)a;
                        const struct sockaddr_in *bin = (const struct sockaddr_in *)b;
                        if (memcmp(&ain->sin_addr, &bin->sin_addr, 4) != 0)
                                return false;
                        if (compare_ports && ain->sin_port != bin->sin_port)
                                return false;
                        break;
                }
                case AF_INET6: {
                        const struct sockaddr_in6 *ain6 = (const struct sockaddr_in6 *)a;
                        const struct sockaddr_in6 *bin6 = (const struct sockaddr_in6 *)b;
                        if (memcmp(&ain6->sin6_addr, &bin6->sin6_addr, 16) != 0)
                                return false;
                        if (compare_ports && ain6->sin6_port != bin6->sin6_port)
                                return false;
                        break;
                }
                default:
                        return false;
                }

                return true;
        }
         
        bool IP::addr_record_is_equal(const addr_record_t *a, const addr_record_t *b, bool compare_ports) 
        {
            return addr_is_equal((const struct sockaddr *)&a->addr, (const struct sockaddr *)&b->addr,
	                     compare_ports);
        } 

	
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
        
        void IP::CopyAddress(const struct sockaddr* addr, addr_record_t &mapped)
        {
            switch (addr->sa_family)
	    {
                    case AF_INET:
                            std::memcpy(&mapped.addr, addr, sizeof(struct sockaddr_in));
                            mapped.len = sizeof(struct sockaddr_in);
                            break;

                    case AF_INET6:
                            std::memcpy(&mapped.addr, addr, sizeof(struct sockaddr_in6));
                            mapped.len = sizeof(struct sockaddr_in6);
                            break;
	    }
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
        
    // dunplicate funtion need to be remvoed
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


        //          char ip[40];  uint16_t port;
        //          IP::AddressToString(mapped, ip, port) ;
        //     
        void IP::AddressToString( addr_record_t &mapped,  char *ip,  uint16_t &port)
        {

              if(mapped.addr.ss_family == AF_INET6)
              {
                  uv_ip6_name((sockaddr_in6* )&mapped.addr, ip, 39);
                  port = ntohs( ((sockaddr_in6 *)&mapped.addr)->sin6_port);

              }
              else if(mapped.addr.ss_family  == AF_INET )
              {
                   uv_ip4_name((sockaddr_in*)&mapped.addr, ip, 16);
                   port =  ntohs( ((sockaddr_in *)&mapped.addr)->sin_port); 
              }

              //STrace << " address: "<<  ip  << " port: " << port;
        }
        
        
        
        
        
        
        void IP::StringToAddress(const char *ip,  uint16_t port, addr_record_t &mapped)
        {

            if (IP::GetFamily(ip) == AF_INET6) {
          
                ASSERT(0 == uv_ip6_addr(ip, port, (struct sockaddr_in6 *)&mapped.addr));
                mapped.len = sizeof(struct sockaddr_in6);
            }
            else {
                ASSERT(0 == uv_ip4_addr(ip, port, (struct sockaddr_in *)&mapped.addr));
                  mapped.len = sizeof(struct sockaddr_in);
            }
            
        }
        
        
        int IP::GetFamily(const std::string& ip) {

            char ia[sizeof (struct in6_addr)];
            if (uv_inet_pton(AF_INET, ip.c_str(), &ia) == 0)

                return AF_INET;
            else if (uv_inet_pton(AF_INET6, ip.c_str(), &ia) == 0)
                return AF_INET6;
            else
            {
                SError << "Invalid IP address format: "<<   ip;
                return PF_UNSPEC;  // It mean hostname need be resolved . check  how to find if it ip4 ipv6 or hostname
            }
        }
        
        
        bool IP::addr_unmap_inet6_v4mapped(struct sockaddr *sa, socklen_t *len) 
        {
            if (sa->sa_family != AF_INET6)
                    return false;

            const struct sockaddr_in6 *sin6 = (const struct sockaddr_in6 *)sa;
            if (!IN6_IS_ADDR_V4MAPPED(&sin6->sin6_addr))
                    return false;

            struct sockaddr_in6 copy = *sin6;
            sin6 = &copy;

            struct sockaddr_in *sin = (struct sockaddr_in *)sa;
            memset(sin, 0, sizeof(*sin));
            sin->sin_family = AF_INET;
            sin->sin_port = sin6->sin6_port;
            memcpy(&sin->sin_addr, ((const uint8_t *)&sin6->sin6_addr) + 12, 4);
            *len = sizeof(*sin);
            return true;
        }

        


    } // namespace net
}//base


/*
 how to find if it ip4 ipv6 or hostname
 
 #include <stdio.h>
   #include <string.h>
   #include <arpa/inet.h>
   #include <stdbool.h>
   
   bool is_ip_address(const char *str) {
       struct in_addr addr;
       struct in6_addr addr6;
       if (inet_pton(AF_INET, str, &addr) == 1) {
           return true; // Valid IPv4 address
       } else if (inet_pton(AF_INET6, str, &addr6) == 1) {
           return true; // Valid IPv6 address
       }
       return false; // Not a valid IP address
   }
   
   int main() {
       const char *test_strings[] = {
           "192.168.1.1",
           "2001:0db8:85a3:0000:0000:8a2e:0370:7334",
           "example.com",
           "not an ip or hostname",
           "127.0.0.1",
           "10.0.0.256", // Invalid octet
           "192.168.1", // Missing octet
           "192.168.1.1.1" // Extra octet
       };
   
       for (int i = 0; i < sizeof(test_strings) / sizeof(test_strings[0]); i++) {
           printf("'%s' is %s IP address.\n", test_strings[i], is_ip_address(test_strings[i]) ? "a valid" : "NOT a valid");
       }
       return 0;
   }
 
 */
