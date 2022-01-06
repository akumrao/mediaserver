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
#include "filethread.h"
#include <atomic>
#include "json/json.h"

#define AUDIOFILE  "/workspace/mediaserver/src/rtsp/main/hindi.pcm"               
#define VIDEOFILE   "/workspace/mediaserver/src/rtsp/main/test.264"

#define AUDIOFILE1  "/var/tmp/songs/quintin.pcm"               
#define VIDEOFILE1  "/var/tmp/videos/test1.264"  

#define FILEPARSER 3

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
  class BasicResponder : public net::ServerResponder
        /// Basic server responder (make echo?)
{
public:

    BasicResponder(net::HttpBase* conn) :
    net::ServerResponder(conn) {
        STrace << "BasicResponder" << std::endl;
    }

    virtual void onClose() {
        LDebug("On close")

    }

    void onRequest(net::Request& request, net::Response& response) ;
    

};

class HttpPostResponder : public net::ServerResponder
/// Basic server responder (make echo?)
{
public:

    HttpPostResponder(net::HttpBase* conn) :
    net::ServerResponder(conn) {
        STrace << "BasicResponder" << std::endl;
    }

    virtual void onClose() {
        LDebug("On close")

    }

    void onRequest(net::Request& request, net::Response& response);
    
    void onPayload(const std::string& /* body */); 
    
     json settingCam{ nullptr};
             
};


class HttpPutResponder : public net::ServerResponder
/// Basic server responder (make echo?)
{
public:

    HttpPutResponder(net::HttpBase* conn) :
    net::ServerResponder(conn) {
        STrace << "BasicResponder" << std::endl;
    }

    virtual void onClose() {
        LDebug("On close")

    }

    void onRequest(net::Request& request, net::Response& response);
    
    void onPayload(const std::string& /* body */); 
    
    json settingCam{ nullptr};
    
    std::vector<std::string>  vec;
    bool ret{false};
             
};



class HttpGetResponder : public net::ServerResponder
/// Basic server responder (make echo?)
{
public:

    HttpGetResponder(net::HttpBase* conn) :
    net::ServerResponder(conn) {
        STrace << "BasicResponder" << std::endl;
    }

    virtual void onClose() {
        LDebug("On close")

    }

    void onRequest(net::Request& request, net::Response& response);
    
    void onPayload(const std::string& /* body */); 
    
     json settingCam{ nullptr};
             
};
        
class HttDeleteResponder : public net::ServerResponder
/// Basic server responder (make echo?)
{
public:

    HttDeleteResponder(net::HttpBase* conn) :
    net::ServerResponder(conn)  {
        STrace << "BasicResponder" << std::endl;
    }

    virtual void onClose() {
         LDebug("On close")

    }

    void onRequest(net::Request& request, net::Response& response);
    
    void onPayload(const std::string& /* body */); 
    
    json settingCam{ nullptr};
    
    std::vector<std::string>  vec;
    bool ret{false};
    
 
    
             
};


 class StreamingResponderFactory1 : public net::ServerConnectionFactory {
        public:
            
            StreamingResponderFactory1( )
            {
                
            }

            net::ServerResponder* createResponder(net::HttpBase* conn) ;
            
 public:        
  

 };


 class ReadMp4: public Thread, public net::HttpServer 
 {
     
     
 public:
     ReadMp4( std::string ip, int port, StreamingResponderFactory1 *factory );
     
     ~ReadMp4( );
     
    // void websocketConnect();
     
      //void send(const char * data, int size, bool binary);
     
     int fmp4( const char *in_filename, const char *out_filename =nullptr, bool fragmented_mp4_options=true);
          
   //virtual void start() override
   // virtual void stop() override;
   
     void run() override;
     
     std::vector<uint8_t> outputData;
     bool looping{true};
     
     
 private:
     
     struct stParser
     {
      #if FILEPARSER ==1
         FFParse  *ffparser;
      #elif FILEPARSER ==2
        FileThread  *ffparser;
      #else
        LiveThread  *ffparser;
      #endif
         
        DummyFrameFilter *fragmp4_filter{nullptr};
        FrameFilter *fragmp4_muxer{nullptr};;
        FrameFilter *info{nullptr};;
        FrameFilter *txt{nullptr};
        fmp4::LiveConnectionContext *ctx{nullptr};;
        int slot{1};    
        
        stParser(ReadMp4 *mp4this,  std::string cam);
        ~stParser();
        
        int number{0};

     };
     
 private:
     
     std::map< std::string,  stParser* > parser ;
   

    // std::string fileName;
     
     std::atomic<int> critical_sec{0};
 public:
     //// 1 ftype, 2 moov , 3 first moof( idr frame), 4 P or B frames cane be dropped 
     void broadcast(const char * data, int size, bool binary,  int frametype , std::string &cam  );
     void on_wsread(net::Listener* connection, const char* msg, size_t len) ;
     
     void on_wsclose(net::Listener* connection);
     
     void on_wsconnect(net::Listener* connection);
     
     
//    virtual void onConnect(    int socketID                        );
//    virtual void onMessage(    int socketID, const string& data    );
//    virtual void onDisconnect( int socketID                        );
//    virtual void   onError(    int socketID, const string& message );
//     
     
     std::mutex mutCam;
     
     
     void addCamera( std::string &cam);
     
     void delCamera( std::string &cam);
     
     void forceDelCamera(std::string &cam ,  std::string  reason );
     
   

 };
 
}
}

#endif /* FMP4_H */

