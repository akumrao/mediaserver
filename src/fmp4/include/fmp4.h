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
#include "http/HttpClient.h"
#include "http/HttpServer.h" 
#include "muxer.h"

#include <atomic>

#define AUDIOFILE  "./hindi.pcm"               
#define VIDEOFILE  "/var/tmp/videos/test1.264"

//data/videos/test.a264"

#define AUDIOFILE1  "/var/tmp/songs/quintin.pcm"               
#define VIDEOFILE1  "/var/tmp/videos/test1.264"

namespace base {
namespace fmp4 {
    
class LiveThread;
class DummyFrameFilter;
class FragMP4MuxFrameFilter;
class InfoFrameFilter;
class TextFrameFilter;
class ReadMp4;  
class LiveConnectionContext;
class FFParse;

 class ReadMp4: public Thread, public net::HttpServer 
 {
     
     
 public:
     ReadMp4( std::string ip, int port, net::ServerConnectionFactory *factory );
     
     ~ReadMp4( );
     
    // void websocketConnect();
     
      //void send(const char * data, int size, bool binary);
     
     int fmp4( const char *in_filename, const char *out_filename =nullptr, bool fragmented_mp4_options=true);
          
   //virtual void start() override
   // virtual void stop() override;
   
     void run() override;
     
     std::vector<uint8_t> outputData;
     bool looping{true};
     
     FFParse  *ffparser;
 private:
     
     DummyFrameFilter *fragmp4_filter{nullptr};
     FrameFilter *fragmp4_muxer{nullptr};;
     FrameFilter *info{nullptr};;
     FrameFilter *txt{nullptr};;
     LiveConnectionContext *ctx{nullptr};;
     int slot{1};        
     std::string fileName;
     
     std::atomic<int> critical_sec{0};
 public:
     
     void broadcast(const char * data, int size, bool binary  );
     void on_read(net::Listener* connection, const char* msg, size_t len) ;
     
//    virtual void onConnect(    int socketID                        );
//    virtual void onMessage(    int socketID, const string& data    );
//    virtual void onDisconnect( int socketID                        );
//    virtual void   onError(    int socketID, const string& message );
//     
     

 };
 
}
}

#endif /* FMP4_H */

