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

#include "http/HTTPResponder.h"
#include "net/netInterface.h"
#include "http/HttpClient.h"
#include "http/HttpServer.h" 
#include "muxer.h"

#include <atomic>
#include "json/json.h"

#define AUDIOFILE  "/workspace/mediaserver/src/rtsp/main/hindi.pcm"               
#define VIDEOFILE   "/workspace/mediaserver/src/rtsp/main/test.264"

#define AUDIOFILE1  "/var/tmp/songs/quintin.pcm"               
#define VIDEOFILE1  "/var/tmp/videos/test1.264"  

#include "webrtc/signaler.h"

//#define FILEPARSER 1

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
 
class HttpPostResponder : public net::BasicResponder
{
public:

    HttpPostResponder(net::HttpBase* conn) :

    net::BasicResponder(conn) {
        STrace << "BasicResponder" << std::endl;
    }

    virtual void onClose() {
        LDebug("On close")

    }

    void onRequest(net::Request& request, net::Response& response);
    
    void onPayload(const std::string& /* body */, net::Request& request); 
    
    json settingCam{ nullptr};
    
    std::string msg;
             
};


class HttpPutResponder : public net::BasicResponder
/// Basic server responder (make echo?)
{
public:

    HttpPutResponder(net::HttpBase* conn) :
    net::BasicResponder(conn) {
        STrace << "BasicResponder" << std::endl;
    }

    virtual void onClose() {
        LDebug("On close")

    }

    void onRequest(net::Request& request, net::Response& response);
    
    void onPayload(const std::string& /* body */, net::Request& request); 
    
    json settingCam{ nullptr};
    
    std::vector<std::string>  vec;
    bool ret{false};
    
    std::string msg;
             
};



class HttpGetResponder : public net::BasicResponder
/// Basic server responder (make echo?)
{
public:

    HttpGetResponder(net::HttpBase* conn) :
    net::BasicResponder(conn) {
        STrace << "BasicResponder" << std::endl;
    }

    virtual void onClose() {
        LDebug("On close")

    }

    void onRequest(net::Request& request, net::Response& response);
    
    void onPayload(const std::string& /* body */, net::Request& request);  
    
     json settingCam{ nullptr};
             
};
        
class HttDeleteResponder : public net::BasicResponder
{
public:

    HttDeleteResponder(net::HttpBase* conn, base::wrtc::Signaler  &sig) :
    net::BasicResponder(conn), sig(sig)  {
        STrace << "BasicResponder" << std::endl;
    }

    virtual void onClose() {
         LDebug("On close")

    }

    void onRequest(net::Request& request, net::Response& response);
    
    void onPayload(const std::string& /* body */,net::Request& request); 
    
    json settingCam{ nullptr};

    bool ret{false};
    
    base::wrtc::Signaler  &sig;
    
             
};

class HttOptionsResponder : public net::BasicResponder
{
public:

    HttOptionsResponder(net::HttpBase* conn) :
    net::BasicResponder(conn)  {
        STrace << "BasicResponder" << std::endl;
    }

    virtual void onClose() {
         LDebug("On close")

    }

    void onRequest(net::Request& request, net::Response& response);
    
      
    json settingCam{ nullptr};
    
    std::vector<std::string>  vec;
    bool ret{false};
    
             
};




 class StreamingResponderFactory1 : public net::ServerConnectionFactory {
        public:
            
            StreamingResponderFactory1( base::wrtc::Signaler  &sig):sig(sig)
            {
                
            }

            net::ServerResponder* createResponder(net::HttpBase* conn) {

                auto& request = conn->_request;

                // Log incoming requests
              //  STrace << "Incoming connection from " << ": Request:\n" << request << std::endl;

                SDebug << "Incoming connection from: " << request.getHost() << " method: " << request.getMethod() << " uri: <<  " << request.getURI() << std::endl;

                // Handle websocket connections
                if (request.getMethod() == "POST") {
                    return new HttpPostResponder(conn);
                }
                else if (request.getMethod() == "PUT") {
                    return new HttpPutResponder(conn);
                }
                else if (request.getMethod() == "GET") {
                    return new HttpGetResponder(conn);
                }
                else if (request.getMethod() == "DELETE") {
                    return new HttDeleteResponder(conn, sig);
                }
                else if (request.getMethod() == "OPTIONS") {
                    return new HttOptionsResponder(conn);
                }
                else {
                    return new net::BasicResponder(conn);
                }


            }
            
 public:        
    base::wrtc::Signaler  &sig;

 };



 class ReadMp4: public Thread, public net::HttpsServer 
 {
     
     
 public:
     ReadMp4( std::string ip, int port,   base::wrtc::Signaler &sig,  net::ServerConnectionFactory *factory );
     
     ~ReadMp4( );
     
    // void websocketConnect();
     
      //void send(const char * data, int size, bool binary);
     
     int fmp4( const char *in_filename, const char *out_filename =nullptr, bool fragmented_mp4_options=true);
          
   //virtual void start() override
   // virtual void stop() override;
   
     void run() override;
     /*
     std::vector<uint8_t> outputData;
     bool looping{true};
     
      #if FILEPARSER
         FFParse  *ffparser;
      #else
        LiveThread  *ffparser;
      #endif
     */
 private:
     
     base::wrtc::Signaler &sig;
     /*
     DummyFrameFilter *fragmp4_filter{nullptr};
     FrameFilter *fragmp4_muxer{nullptr};;
     FrameFilter *info{nullptr};;
     FrameFilter *txt{nullptr};;
     LiveConnectionContext *ctx{nullptr};;
     int slot{1};        
     std::string fileName;
     
     std::atomic<int> critical_sec{0};
     */
 public:
     
     void broadcast(std::string &cam, std::string &reason );
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

