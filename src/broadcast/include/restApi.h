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
#include "http/HttpClient.h"
#include "http/HttpServer.h"
#include "muxer.h"
#include "net/netInterface.h"

#include "json/json.h"
#include <atomic>
#include <queue>
//#define AUDIOFILE "/workspace/mediaserver/src/rtsp/main/hindi.pcm"
//#define VIDEOFILE "/workspace/mediaserver/src/rtsp/main/test.264"
//
//#define AUDIOFILE1 "/var/tmp/songs/quintin.pcm"
//#define VIDEOFILE1 "/var/tmp/videos/test1.264"

#include "webrtc/signaler.h"

#include "Mp4Thread.h"

namespace base {
namespace web_rtc {

    class LiveThread;
    class DummyFrameFilter;
    class FragMP4MuxFrameFilter;
    class InfoFrameFilter;
    class TextFrameFilter;
    class RestApi;
    class LiveConnectionContext;
    class FFParse;

    class HttpPostResponder : public net::BasicResponder {
    public:
        HttpPostResponder(net::HttpBase* conn)
            :

            net::BasicResponder(conn)
        {
            STrace << "BasicResponder" << std::endl;
        }

        virtual void onClose()
        {
            LDebug("On close")
        }

        void onRequest(net::Request& request, net::Response& response);

        void onPayload(const std::string& /* body */, net::Request& request);

        json settingCam{ nullptr };

        std::string msg;
    };

    class HttpPutResponder : public net::BasicResponder
    /// Basic server responder (make echo?)
    {
    public:
        HttpPutResponder(net::HttpBase* conn, base::web_rtc::Signaler& sig)
            : net::BasicResponder(conn), sig(sig)
        {
            STrace << "BasicResponder" << std::endl;
        }

        virtual void onClose()
        {
            LDebug("On close")
        }

        void onRequest(net::Request& request, net::Response& response);

        void onPayload(const std::string& /* body */, net::Request& request);

        json settingCam{ nullptr };

        std::vector<std::string> vec;
        bool ret{ false };

        std::string msg;
         base::web_rtc::Signaler& sig;
    };

    class HttpGetResponder : public net::BasicResponder
    /// Basic server responder (make echo?)
    {
    public:
        HttpGetResponder(net::HttpBase* conn)
            : net::BasicResponder(conn)
        {
            STrace << "BasicResponder" << std::endl;
        }

        virtual void onClose()
        {
            LDebug("On close")
        }

        void onRequest(net::Request& request, net::Response& response);

        void onPayload(const std::string& /* body */, net::Request& request);
        
        void listFilesRecursively(const char *basePath, json & ret, bool isDir );


        json settingCam{ nullptr };
    };

    class HttDeleteResponder : public net::BasicResponder {
    public:
        HttDeleteResponder(net::HttpBase* conn, base::web_rtc::Signaler& sig)
            : net::BasicResponder(conn)
            , sig(sig)
        {
            STrace << "BasicResponder" << std::endl;
        }

        virtual void onClose()
        {
            LDebug("On close")
        }

        void onRequest(net::Request& request, net::Response& response);

        void onPayload(const std::string& /* body */, net::Request& request);

        json settingCam{ nullptr };

        bool ret{ false };

        base::web_rtc::Signaler& sig;
    };

    class HttOptionsResponder : public net::BasicResponder {
    public:
        HttOptionsResponder(net::HttpBase* conn)
            : net::BasicResponder(conn)
        {
            STrace << "BasicResponder" << std::endl;
        }

        virtual void onClose()
        {
            LDebug("On close")
        }

        void onRequest(net::Request& request, net::Response& response);

        json settingCam{ nullptr };

        std::vector<std::string> vec;
        bool ret{ false };
    };

    class StreamingResponderFactory1 : public net::ServerConnectionFactory {
    public:
        StreamingResponderFactory1(base::web_rtc::Signaler& sig)
            : sig(sig)
        {
        }

        net::ServerResponder* createResponder(net::HttpBase* conn)
        {

            auto& request = conn->_request;

            STrace << "Incoming connection from "
                   << ": Request:\n"
                   << request << std::endl;

            if (!request.has("Host")) {

                SError << "Incoming connection does not have host " << request.getMethod() << " uri: <<  " << request.getURI() << std::endl;

                return new net::BasicResponder(conn);
            }

            SDebug << "Incoming connection from: " << request.getHost() << " method: " << request.getMethod() << " uri: <<  " << request.getURI() << std::endl;

            // Handle websocket connections
            if (request.getMethod() == "POST") {
                return new HttpPostResponder(conn);
            } else if (request.getMethod() == "PUT") {
                return new HttpPutResponder(conn, sig);
            } else if (request.getMethod() == "GET") {
                return new HttpGetResponder(conn);
            } else if (request.getMethod() == "DELETE") {
                return new HttDeleteResponder(conn, sig);
            } else if (request.getMethod() == "OPTIONS") {
                return new HttOptionsResponder(conn);
            } else {
                return new net::BasicResponder(conn);
            }
        }

    public:
        base::web_rtc::Signaler& sig;
    };

    class RestApi : public Thread, public net::HttpsServer {

    public:
        RestApi(std::string ip, int port, base::web_rtc::Signaler& sig, net::ServerConnectionFactory* factory);

        ~RestApi();

        // void websocketConnect();

        //void send(const char * data, int size, bool binary);

        int fmp4(const char* in_filename, const char* out_filename = nullptr, bool fragmented_mp4_options = true); // this code is for mpeg-dash not used in webrtc

        //virtual void start() override
        // virtual void stop() override;

       // void run() override;
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
        base::web_rtc::Signaler& sig;
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
    //void broadcast(std::string& cam, std::string& reason);
    
        
    
     struct stParser
     {
//      #if FILEPARSER ==1
//         FFParse  *ffparser;
//      #elif FILEPARSER ==2
//        FileThread  *ffparser;
//      #else
//        LiveThread  *ffparser;
//      #endif
//         
//        DummyFrameFilter *fragmp4_filter{nullptr};
//        FrameFilter *fragmp4_muxer{nullptr};;
//        FrameFilter *info{nullptr};;
//        FrameFilter *txt{nullptr};
//        LiveConnectionContext *ctx{nullptr};;
//        int slot{1};    
         
        Mp4Thread* mp4Thread{nullptr};
         
        stParser(RestApi *mp4this,  std::string &cam, std::string &date, std::string &hr, std::string &time);
        ~stParser();
        
        int refCount{0};

    };
     
 private:
     
    std::map< std::string,  stParser* > parser ;
   

    // std::string fileName;
     
     std::atomic<int> critical_sec{0};
 public:
     //// 1 ftype, 2 moov , 3 first moof( idr frame), 4 P or B frames cane be dropped 
    void broadcast(const char * data, int size, bool binary,  int frametype , std::string &cam  );
    void postAppMessage(std::string &cam, FRAMETYPE frameType, std::string &reason );
    void on_wsread(net::Listener* connection, const char* msg, size_t len) ;
    
    void on_wsclose(net::Listener* connection);
     
    void on_wsconnect(net::Listener* connection);
     
    
    void run() override;
     
    std::mutex mutCam;
     
     
    void addCamera( std::string &cam , std::string &date, std::string &hr, std::string &time);
     
    void delCamera( std::string &cam);
     
    void forceDelCamera(std::string &cam ,  std::string  reason );
    
    void deleteAsync( std::string cam, std::string &reason);
    std::mutex mutDelcamera;
    
    struct CamError {
    
        std::string cam;
        std::string reason;
    };
    std::queue< CamError > vecDelcamera;
    CondWait condwait;

    };
}
}

#endif /* FMP4_H */
