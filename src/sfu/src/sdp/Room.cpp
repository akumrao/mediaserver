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
     peers = new Peers(signaler);
}

Rooms::~Rooms() {
    delete peers;
}



void Rooms::createRoom(std::string const& name, json const& data, bool isAck, json & ack_resp) {

    
       std::string roomName = data[0].get<std::string>();
    
      if( signaler->worker->mapRouters.find(roomName) !=  signaler->worker->mapRouters.end())
       {
           SWarn << "Room already exist " << name << ":" << data[0].get<std::string>() << " - sfuID is " << data[1].get<std::string>();
           return ;
       }
       
       SInfo << name << " room :" << data[0].get<std::string>() << " - sfuID is " << data[1].get<std::string>();

       
       json param = json::array();
       param.push_back("worker_createRouter");
       param.push_back(data[1].get<std::string>());
       json &trans = Settings::configuration.worker_createRouter;
       trans["internal"]["routerId"] = roomName ;// uuid4::uuid();
       param.push_back(trans);
       signaler->request("worker_createRouter", param, true,  [&](const json & ack_resp){});
       
      // Room * room = new Room(signaler);
     //  room->routerId = trans["internal"]["routerId"];
    //   room->name = roomName;
//       mapRooms[roomName] = room;
       
       
       //create router_createAudioLevelObserver
       {
            json param = json::array();
            param.push_back("worker_createRouter");
            param.push_back(data[1].get<std::string>());
            json &trans = Settings::configuration.router_createAudioLevelObserver;
            trans["internal"]["routerId"] = roomName;
            param.push_back(trans);
            signaler->request("createAudioLevelObserver", param, true,  [&](const json & ack_resp){});
       
       }
   }


    /* HandlerId is Consumer or Producer device id*/
    void Rooms::resume(std::string &room , std::string& participantID, std::string& handlerId,  bool flag, bool producer)
    {
        SInfo << "Resume/Pause video for room" <<  room << " : " <<  " for participantID  " << participantID << " for consumerID  " << handlerId;

       if( signaler->worker->mapRouters.find(room) !=  signaler->worker->mapRouters.end())
       {
            peers->resume(participantID, handlerId, flag, producer);
       }
       else
       {
            SError << "Room does not exist: " << room  ;
       }

    }
    
    void Rooms::onSubscribe(std::string &room , std::string& participantID, const json& peerPartiID)
    {
  
        SInfo << "Consumer subscribe to join room: " <<  room << " : " <<  " from participantID  " << participantID  << " peerids" << peerPartiID.dump(4);

        
       if( signaler->worker->mapRouters.find(room) !=  signaler->worker->mapRouters.end())
       {
            peers->onSubscribe(room, participantID, peerPartiID);
       }
       else
       {
            SError << "Room does not exist: " << room  ;
       }

    }

    void Rooms::on_producer_offer(std::string &room , std::string& participantID, const json &sdp)
    {
       SInfo << "Producer Offer to join room" <<  room << " : " <<  " from participantID  " << participantID ;
       
       
       if( signaler->worker->mapRouters.find(room) !=  signaler->worker->mapRouters.end())
       {
          
            peers->on_producer_offer(room, participantID,  sdp);
       }
       else
       {
            SError << "Room does not exist: " << room  ;
       }
    }
    
    
    void Rooms::on_consumer_answer(std::string &room , std::string& participantID, std::string& to, const json &sdp)
    {
       SInfo << "Consumer got answer to join room" <<  room << " : " <<  " from participantID  " << participantID  << " to " << to;

        
       if( signaler->worker->mapRouters.find(room) !=  signaler->worker->mapRouters.end())
       {
          
            peers->on_consumer_answer(participantID, to,  sdp);
       }
       else
       {
            SError << "Room does not exist: " << room  ;
       }
    }
    
    
    void Rooms::onDisconnect( std::string& participantID)
    {
  
        SInfo << " Disconeect peer with participantID  " << participantID ;

          
        peers->onDisconnect(participantID);
  

    }

    
    void Rooms::producer_getStats(std::string &room , std::string& participantID, const std::string& producerId) 
    {
  
        SInfo << "Producer requested Stats for Room: " <<  room << " : " <<  "  participantID  " << participantID ;
   
       if( signaler->worker->mapRouters.find(room) !=  signaler->worker->mapRouters.end())
       {
            peers->producer_getStats(participantID, producerId);
       }
       else
       {
            SError << "Room does not exist: " << room  ;
       }
    }

    void Rooms::rtpObserver_addProducer(std::string &room , std::string& participantID, bool flag)
    {
  
        SInfo << "Producer requested Stats for Room: " <<  room << " : " <<  "  participantID  " << participantID ;
   
       if( signaler->worker->mapRouters.find(room) !=  signaler->worker->mapRouters.end())
       {
            peers->rtpObserver_addProducer(participantID, flag);
       }
       else
       {
            SError << "Room does not exist: " << room  ;
       }
    }



   void Rooms::consumer_getStats(std::string &room , std::string& participantID, const std::string& consumerId) 
    {
  
        SInfo << "Consumer requested Stats for Room: " <<  room << " : " <<  "  participantID  " << participantID ;
   
       if( signaler->worker->mapRouters.find(room) !=  signaler->worker->mapRouters.end())
       {
            peers->consumer_getStats(participantID, consumerId);
       }
       else
       {
            SError << "Room does not exist: " << room  ;
       }
    }



   void Rooms::setPreferredLayers(std::string &room , std::string& participantID,  json &layer) 
    {
  
        SInfo << "Consumer set PreferredLayers for Room: " <<  room << " : " <<  "  participantID  " << participantID ;
   
       if( signaler->worker->mapRouters.find(room) !=  signaler->worker->mapRouters.end())
       {
            peers->setPreferredLayers(participantID, layer);
       }
       else
       {
            SError << "Room does not exist: " << room  ;
       }
    }


}//namespace SdpParse 
