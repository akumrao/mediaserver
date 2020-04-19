/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */


#ifndef SPEEDTEST_SPEEDTESTCLIENT_H
#define SPEEDTEST_SPEEDTESTCLIENT_H


#include <ctime>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <chrono>
#include <unistd.h>
#include "SpeedTest.h"
#include "DataTypes.h"
class SpeedTestClient {
public:
    explicit SpeedTestClient(const ServerInfo& serverInfo);
    ~SpeedTestClient();

    bool connect();
    bool ping(long &millisec);
    bool upload(long size, long chunk_size, long &millisec);
    bool download(long size, long chunk_size, long &millisec);
    float version();
    const std::pair<std::string, int> hostport();
    void close();


private:
    bool mkSocket();
    ServerInfo mServerInfo;
    int mSocketFd;
    float mServerVersion;
    static bool readLine(int& fd, std::string& buffer);
    static bool writeLine(int& fd, const std::string& buffer);
};

typedef bool (SpeedTestClient::*opFn)(const long size, const long chunk_size, long &millisec);
#endif //SPEEDTEST_SPEEDTESTCLIENT_H
