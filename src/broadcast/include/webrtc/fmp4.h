/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   fmp4.h
 * Author: root
 *
 * Created on June 8, 2021, 10:48 PM
 */

#ifndef FMP4_H
#define FMP4_H



#include "base/thread.h"
#include <string>
#include <vector>
#include <set>

#include "net/netInterface.h"
#include "http/HttpsClient.h"
//#include "webrtc/peer.h"

//#include "WebSocketServer.h"


 

namespace base {
    
    namespace wrtc {
    
    class PeerfMp4;
    }
namespace fmp4 {
    
class FFParse;
 class ReadMp4: public Thread
 {
     
     
 public:
     ReadMp4( );
     
     ~ReadMp4( );
     
     void websocketConnect();
     
      //void send(const char * data, int size, bool binary);
     
     int fmp4( const char *in_filename, const char *out_filename =nullptr, bool fragmented_mp4_options=true);
          
   //virtual void start() override
   // virtual void stop() override;
    
     void run() override;
     
     std::vector<uint8_t> outputData;
     bool looping{true};
     
     void broadcast(const char * data, int size, bool binary   );
     FFParse  *ffparser;
 private:
     
    //net::ClientConnecton *conn{nullptr};
     
     std::string fileName;
     
     std::mutex g_num_mutex;
     
 public:
     
     void add(wrtc::PeerfMp4 *obj);
     void remove(wrtc::PeerfMp4 *obj);
     
     std::set< wrtc::PeerfMp4* > setfmp4Peers;
     //ChatServer( int port );
  //  ~ChatServer( );
//    virtual void onConnect(    int socketID                        );
//    virtual void onMessage(    int socketID, const string& data    );
//    virtual void onDisconnect( int socketID                        );
//    virtual void   onError(    int socketID, const string& message );
//     
     

 };
 }


namespace wrtc {
    
class Peer;
class PeerfMp4
 {
     
     
 public:
     PeerfMp4(Peer *pc);


     Peer *pc;
     
     ~PeerfMp4( );
     
     
        void onConnect();
       
        
        void ondisConnect();
        void onMessage(std::string str);
        
    
      //void send(const char * data, int size, bool binary);
     
   
   //virtual void start() override
   // virtual void stop() override;
   
     //void run() override;
     
  
     
   
 private:
     
    //net::ClientConnecton *conn{nullptr};
  
     
 public:
     
     //ChatServer( int port );
  //  ~ChatServer( );
//    virtual void onConnect(    int socketID                        );
//    virtual void onMessage(    int socketID, const string& data    );
//    virtual void onDisconnect( int socketID                        );
//    virtual void   onError(    int socketID, const string& message );
//     
     

 };

}
}

#endif /* FMP4_H */

