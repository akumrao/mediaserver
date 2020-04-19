/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */


#ifndef SPEEDTEST_DATATYPES_H
#define SPEEDTEST_DATATYPES_H
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
static const float EARTH_RADIUS_KM = 6371.0;

typedef struct ip_info_t {
    std::string ip_address;
    std::string isp;
    float lat;
    float lon;
} IPInfo;

    
    struct ServerInfo {
    std::string url;
    float lat;
    float lon;
    std::string name;
    std::string country;
    std::string country_code;
    std::string sponsor;
    int   id;
    std::string host;
  
    float distance;
    
    ServerInfo(std::string url,float lat, float lon, std::string name,  std::string country, std::string country_code,  std::string sponsor, int   id,  std::string host, float distance):
    url(url),lat(lat), lon(lon), name(name),country(country), country_code(country_code), sponsor(sponsor),  id(id),  host(host),  distance(distance)
    {
        
    }
    
    ServerInfo(){}

} ;

typedef struct test_config_t {
    long start_size;
    long max_size;
    long incr_size;
    long buff_size;
    long min_test_time_ms;
    int  concurrency;
    std::string label;
} TestConfig;

#endif //SPEEDTEST_DATATYPES_H
