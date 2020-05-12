/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Rooms.cpp
 * Author: root
 * 
 * Created on May 4, 2020, 9:22 AM
 */


#include "sdp/Room.h"
#include "LoggerTag.h"
#include "base/uuid.h"
#include "sdp/signaler.h"

namespace SdpParse {

Rooms::Rooms( Signaler *signaler):signaler(signaler)
{
}

Rooms::~Rooms() {
    
    for(auto& room : mapRooms  )
    {
        delete room.second;
    }
}



void Rooms::createRoom(std::string const& name, json const& data, bool isAck, json & ack_resp) {

    
       std::string roomName = data[0].get<std::string>();
    
       if( mapRooms.find(roomName) != mapRooms.end())
       {
           SWarn << "Room already exist " << name << ":" << data[0].get<std::string>() << " - sfuID is " << data[1].get<std::string>();
           return ;
       }
       
       SInfo << name << " room :" << data[0].get<std::string>() << " - sfuID is " << data[1].get<std::string>();

       
       json param = json::array();
       param.push_back("worker_createRouter");
       param.push_back(data[1].get<std::string>());
       json &trans = Settings::configuration.worker_createRouter;
       trans["internal"]["routerId"] = uuid4::uuid();
       param.push_back(trans);
       signaler->request("worker_createRouter", param, true, ack_resp);
       
       Room * room = new Room(signaler);
       room->routerId = trans["internal"]["routerId"];
       room->name = roomName;
       mapRooms[roomName] = room;
       
       
       //create router_createAudioLevelObserver
       {
            json param = json::array();
            param.push_back("worker_createRouter");
            param.push_back(data[1].get<std::string>());
            json &trans = Settings::configuration.router_createAudioLevelObserver;
            trans["internal"]["routerId"] = room->routerId;
            param.push_back(trans);
            signaler->request("createAudioLevelObserver", param, true, ack_resp);
       
       }
   }



    void Rooms::resume(std::string &room , std::string& participantID, bool flag)
    {
  
//                   consumer =  new SdpParse::Consumer(peer, peerID );
//                   consumer->runit(this , producer->producer);
//                   sendSDP("offer", consumer->offer);

    }
    void Rooms::onSubscribe(std::string &room , std::string& participantID)
    {
  
        SInfo << "Consumer subscribe to join room" <<  room << " : " <<  " from participantID  " << participantID ;

        
       if( mapRooms.find(room) != mapRooms.end())
       {
          
            Peers *peers = mapRooms[room]->peers;
            peers->onSubscribe(participantID);
       }
       else
       {
            SError << "Room does not exist: " << room  ;
       }

    }

    void Rooms::on_producer_offer(std::string &room , std::string& participantID, const json &sdp)
    {
       SInfo << "Producer Offer to join room" <<  room << " : " <<  " from participantID  " << participantID ;
       
       
       if( mapRooms.find(room) != mapRooms.end())
       {
          
            Peers *peers = mapRooms[room]->peers;
            peers->on_producer_offer(participantID,  sdp);
       }
       else
       {
            SError << "Room does not exist: " << room  ;
       }
    }
    
    
    void Rooms::on_consumer_answer(std::string &room , std::string& participantID, const json &sdp)
    {
       SInfo << "Consumer got answer to join room" <<  room << " : " <<  " from participantID  " << participantID ;

        
       if( mapRooms.find(room) != mapRooms.end())
       {
          
            Peers *peers = mapRooms[room]->peers;
            peers->on_consumer_answer(participantID,  sdp);
       }
       else
       {
            SError << "Room does not exist: " << room  ;
       }
    }
    
    
    void Rooms::onDisconnect(std::string &room , std::string& participantID)
    {
  
        SInfo << " Disconeect peer from Room: " <<  room << " : " <<  " with participantID  " << participantID ;

        
       if( mapRooms.find(room) != mapRooms.end())
       {
          
            Peers *peers = mapRooms[room]->peers;
            peers->onDisconnect(participantID);
       }
       else
       {
            SError << "Room does not exist: " << room  ;
       }

    }

    
    void Rooms::producer_getStats(std::string &room , std::string& participantID)
    {
  
        SInfo << "Producer requested Stats for Room: " <<  room << " : " <<  "  participantID  " << participantID ;
   
       if( mapRooms.find(room) != mapRooms.end())
       {
            Peers *peers = mapRooms[room]->peers;
            peers->producer_getStats(participantID);
       }
       else
       {
            SError << "Room does not exist: " << room  ;
       }
    }

     void Rooms::rtpObserver_addProducer(std::string &room , std::string& participantID)
    {
  
        SInfo << "Producer requested Stats for Room: " <<  room << " : " <<  "  participantID  " << participantID ;
   
       if( mapRooms.find(room) != mapRooms.end())
       {
            Peers *peers = mapRooms[room]->peers;
            peers->rtpObserver_addProducer(participantID);
       }
       else
       {
            SError << "Room does not exist: " << room  ;
       }
    }

}//namespace SdpParse 