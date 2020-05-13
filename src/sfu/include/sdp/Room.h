/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Rooms.h
 * Author: root
 *
 * Created on May 4, 2020, 9:22 AM
 */

#ifndef ROOM_H
#define ROOM_H

#include "sdp/Peer.h"

namespace SdpParse {
    class Signaler;
    
    struct Room
    {
        std::string name;
        std::string routerId;
        Peers *peers;
        
        Room(Signaler *signaler)
        {
            peers = new Peers(signaler, this);
        }
        ~Room()
        {
            delete peers;
        }        
        
    };

class Rooms {
public:
    Rooms( Signaler *signaler);

     ~Rooms();
    
    std::map < std::string , Room*> mapRooms;
    
    void createRoom(std::string const& name, nlohmann::json const& data, bool isAck, nlohmann::json & ack_resp);
    
    void on_producer_offer(std::string &room , std::string& participantID, const nlohmann::json &sdp);
    void on_consumer_answer(std::string &room , std::string& participantID, const nlohmann::json &sdp);;
    
    void onSubscribe(std::string &room , std::string& participantID);
    void resume(std::string &room , std::string& participantID, bool flag);
    void onDisconnect(std::string &room , std::string& participantID);

    void producer_getStats( std::string &room , std::string& participantID, nlohmann::json &stats); 
    void consumer_getStats( std::string &room , std::string& participantID, nlohmann::json &stats); 
    void rtpObserver_addProducer(std::string &room , std::string& participantID, bool flag);
    void setPreferredLayers( std::string &room ,std::string& participantID,  nlohmann::json &layer);
private:
    
    Signaler *signaler;

};

}//namespace SdpParse 
#endif /* ROOM_H */

