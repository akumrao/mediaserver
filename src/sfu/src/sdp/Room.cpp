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

    
      // std::string roomName = data[0].get<std::string>();
    
       if( mapRooms.find(name) != mapRooms.end())
       {
           SWarn << "Room already exist " << name << ":" << data[0].get<std::string>() << " - my client ID is " << data[1].get<std::string>();
           return ;
       }
       
       SInfo << name << ":" << data[0].get<std::string>() << " - my client ID is " << data[1].get<std::string>();

       
       json param = json::array();
       param.push_back("worker_createRouter");
       param.push_back(data[1].get<std::string>());
       json &trans = Settings::configuration.worker_createRouter;
       trans["id"] = 1;
       trans["internal"]["routerId"] = uuid4::uuid();
       param.push_back(trans);
       signaler->request("signal", param, true, ack_resp);
       
       Room * room = new Room(signaler);
       room->routerId = trans["internal"]["routerId"];
       room->name = name;
       mapRooms[name] = room;
       
   }


    void Rooms::onffer(std::string &name , std::string& participantID, const json &sdp)
    {
        
       if( mapRooms.find(name) != mapRooms.end())
       {
           SWarn << "Room already exist " << name ;
           return ;
       }
       
        Peers *peers = mapRooms[name]->peers;
        peers->onffer(participantID,  sdp);
  
    }


}//namespace SdpParse 