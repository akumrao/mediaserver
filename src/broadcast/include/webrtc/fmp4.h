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


#include "net/netInterface.h"
#include "http/HttpsClient.h"
//#include "webrtc/peer.h"

//#include "WebSocketServer.h"


 

namespace base {
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
     
     void broadcast(const char * data, int size, bool binary   ){}
     FFParse  *ffparser;
 private:
     
    //net::ClientConnecton *conn{nullptr};
     
     std::string fileName;
     
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


namespace wrtc {
    
    class Peer;
class ReadMp4: public Thread
 {
     
     
 public:
     ReadMp4(Peer *pc);


     Peer *pc;
     
     ~ReadMp4( );
     
    
      //void send(const char * data, int size, bool binary);
     
     int fmp4( const char *in_filename, const char *out_filename =nullptr, bool fragmented_mp4_options=true);
          
   //virtual void start() override
   // virtual void stop() override;
   
     void run() override;
     
     std::vector<uint8_t> outputData;
     bool looping{true};
     
   
 private:
     
    //net::ClientConnecton *conn{nullptr};
     
     std::string fileName;
     
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

