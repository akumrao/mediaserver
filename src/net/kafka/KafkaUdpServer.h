/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */


/*
 * KafkaUdpServer.cpp - Send GET requests to a network entity and retrieve UPS properties.
 *
 * SNMP application that uses the SNMP GET request to query for information on a network entity. 
 * One or more object identifiers (OIDs) may be given as arguments to parse SNMP Request. 
 * g++ -g KafkaUdpServer.cpp ../AppSw/CcuKafkaMessage.cpp  -lsnmp -lpthread -lcppkafka ../OsEncap/libosencap.a -I/ccu/src/OsEncap/ -o test1
 */

#ifndef CKafkaUdpServer_H_
#define CKafkaUdpServer_H_


#include <string>
#include <vector>
#include <utility>
#include <map>
#include <iostream>
#include <thread>
#include <atomic>



#include "base/base.h"
#include "base/logger.h"
#include "base/application.h"
#include "net/UdpSocket.h"
#include "base/test.h"
#include "base/time.h"


using std::endl;
using namespace base;
using namespace net;
using namespace base::test;

//typedef std::vector<std::string> typeArg;
//typedef std::map<std::string, std::string > typeParam;
//typedef std::pair< typeArg, typeParam > typeAction;
//typedef std::vector<typeAction> typeRow;

enum kafka_msg_type {
    NORMAL, ERR, ALERT
};

class KafkaUdpServer : public UdpServer::Listener {
public:

    KafkaUdpServer(std::string IP, int port);

    void start();

    void send(std::string txt, std::string ip, int port) ;
    void shutdown() ;

    void OnUdpSocketPacketReceived(UdpServer* socket, const char* data, size_t len, struct sockaddr* remoteAddr);

    void postToKafka(std::string msg, kafka_msg_type type);

    UdpServer *udpServer;

    std::string IP;
    int port;

    Producer *kfProducer;

};

#endif // _CCU_CKafkaUdpServer_H_


