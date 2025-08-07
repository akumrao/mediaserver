/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

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
        typedef struct addr_record {
            struct sockaddr_storage addr;
            socklen_t len;
        } addr_record_t;

        class IP
        {
        public:
            static void GetAddressInfo(struct sockaddr* addr, int& family, std::string& ip, uint16_t& port);
            static int GetFamily(const std::string& ip);
	    static void NormalizeIp(std::string& ip);
	    static bool CompareAddresses(const struct sockaddr* addr1, const struct sockaddr* addr2);
	    static struct sockaddr_storage CopyAddress(const struct sockaddr* addr);
            static void CopyAddress(const struct sockaddr* addr, addr_record_t &mapped);
            static void AddressToString( addr_record_t &mapped,  char *buf,  uint16_t &port);
            static void StringToAddress( const char *ip,  uint16_t port, addr_record_t &mapped);
            static bool addr_is_equal(const struct sockaddr *a, const struct sockaddr *b, bool compare_ports);
            static bool addr_record_is_equal(const addr_record_t *a, const addr_record_t *b, bool compare_ports);
            static bool addr_unmap_inet6_v4mapped(struct sockaddr *sa, socklen_t *len); 
        };


    } // namespace net
} // base
#endif
