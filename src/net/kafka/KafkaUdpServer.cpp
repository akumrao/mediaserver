//============================================================================
// Name        : KafkaUdpServer.cpp
// Author      : Arvind Umrao <arvindumrao@sunmobility.co.in>
// Version     :
// Copyright   : Copyright SUNMobility, Bangalore India
// Description : Monitor UPS
//
//============================================================================




#include <iostream>
#include <algorithm>
#include <chrono>





#define _KAFKA_ 1

#ifdef _KAFKA_

#include <cppkafka/producer.h>
//nclude "CcuCanWheeler.h"
#include "CcuKafkaMessage.h"
using namespace cppkafka;

#endif //_KAFKA_

#include "KafkaUdpServer.h"

void KafkaUdpServer::postToKafka(std::string msg, kafka_msg_type type) {
    //std::cout << "err " << type << " msg " << msg << std::endl;


    /*
        {


            Configuration config = {
                { "metadata.broker.list", "127.0.0.1:9092"},
                { "group.id", "bp_summary"},
            };

            // Create the producer
            Producer  *producer = new Producer(config);

            // Produce a message!
            string message = "{\"media-time\": 1576340478602, \"media-source\": {\"type\": \"BP\"}, \"media-recordType\": \"BP-SUMMARY-DATA\", \"media-txIdKey\": \"1576340478-601738\", \"@version\": \"1\", \"media-data\": {\"BPSummary\": {\"downloadspeed\": 1, \"latency\": 10}}}";

            std::string kfKey = CcuKafkaMessage::createKfMsgKey();

            MessageBuilder kfMessageBuilder = MessageBuilder("battorch-to-metrics");
            kfMessageBuilder.partition(0).key(kfKey).payload(message);

            std::string test = kfMessageBuilder.payload();

            producer->produce(kfMessageBuilder);
        
   

            std::cout << message << std::endl << std::flush;

            std::chrono::milliseconds flushTmo(60000);
            producer->flush(flushTmo);


            return;



        }*/

    //LTrace(msg)
    string parsed, input = msg;
    stringstream input_stringstream(input);
    


    while (getline(input_stringstream, parsed, '\n'))
    {
        if(parsed.length() < 5)
                continue;
        
        std::string kfKey = CcuKafkaMessage::createKfMsgKey();
        std::string txIdKey = CcuKafkaMessage::createTxIdKey();
    
        if (  parsed.at(0) == '{' && parsed.at(parsed.length() - 1) == '}')
        {

            LTrace(parsed);
            try {
                // Create the kafka producer

                              
                    std::string jsonOutput;

                    jsonOutput = "{\"media-time\":" + kfKey + ",";
                    jsonOutput += "\"media-txIdKey\":\"" + txIdKey + "\",";
                    jsonOutput += "\"media-source\":{\"type\":\"media\"},";
                    jsonOutput += "\"media-recordType\":\"Down-MONITOR-DATA\",\"media-data\":";
                    jsonOutput += parsed;
                    jsonOutput += "}";

   
                        MessageBuilder kfMessageBuilder = MessageBuilder("battorch-to-metrics");
                        kfMessageBuilder.partition(0).key(kfKey).payload(jsonOutput);


                        kfProducer->produce(kfMessageBuilder);
                        kfProducer->flush();
                   

                //std::chrono::milliseconds flushTmo(60000);
                // kfProducer->flush(flushTmo);

            } catch (...) {

            }
        }
        else
        {
            CcuKafkaMessage::sendLogMessage(kfProducer, txIdKey, "fix", "media", "error", parsed, "{}");
        }
        
        
    }//while

}

/**************************************************************************************************************/


KafkaUdpServer::KafkaUdpServer(std::string IP, int port) : IP(IP), port(port), kfProducer(nullptr) {

    Configuration kfProducerConfig = {
        { "metadata.broker.list", "127.0.0.1:9092"},
        { "queue.buffering.max.ms", 1},
        { "group.id", "bp_summary"},
        { "batch.num.messages", 1}
    };

    kfProducer = new Producer(kfProducerConfig);

}

void KafkaUdpServer::start() {
    // socket.send("Arvind", "127.0.0.1", 7331);
    udpServer = new UdpServer(this, IP, port);
    udpServer->bind();
}

void KafkaUdpServer::send(std::string txt, std::string ip, int port) {
    udpServer->send((char*) txt.c_str(), txt.length(), ip, port);
}

void KafkaUdpServer::shutdown() {

    delete udpServer;
    udpServer = nullptr;

    delete kfProducer;
    kfProducer = nullptr;

}

void KafkaUdpServer::OnUdpSocketPacketReceived(UdpServer* socket, const char* data, size_t len, struct sockaddr* remoteAddr) {

    int family;

    std::string peerIp;
    uint16_t peerPort;

    IP::GetAddressInfo(
            remoteAddr, family, peerIp, peerPort);

   /// std::cout << "OnUdpSocketPacketReceived " << data << " ip " << peerIp << ":" << peerPort << std::endl << std::flush;

    postToKafka(data, NORMAL);

}

int main(int argc, char** argv) {


    //   objSnmpGet->postToKafka("arvind", NORMAL);
    // objSnmpGet->postToKafka("ravind",ERR);



    Logger::instance().add(new ConsoleChannel("debug", Level::Trace));


    Application app;

    KafkaUdpServer socket("0.0.0.0", 6000);
    socket.start();

   // socket.postToKafka("{\"time\":1576365159633,\"dowloadspeed_kbps\":6772.05,\"latency_ms\":1.666831}\n{\"time\":1576365159633,\"dowloadspeed_kbps\":5672.05,\"latency_ms\":0.666831}\n", NORMAL);
    
    socket.postToKafka("error\n", NORMAL);

    
    socket.send("arvind", "127.0.0.1", 6000);

    app.waitForShutdown([&](void*) {

    socket.shutdown();

    });

}

