/* This file is part of mediaserver. A RTSP live server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */


#include "fmp4.h"

#include "base/define.h"
#include "base/test.h"
#include <thread>
#include "livethread.h"
 #include "Settings.h"

//#include <libavutil/timestamp.h>
//#include <avformat.h>
extern "C"
{
//#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
//#include <libavcodec/avcodec.h>
}

#define tcprequest false

#include "http/websocket.h"

namespace base {
    
    //fmp4::ReadMp4 *self;

    namespace fmp4 {


         void BasicResponder::onRequest(net::Request& request, net::Response& response) {
            STrace << "On complete" << std::endl;

            const char *msg = "Post with Json is allowed";
            response.setContentLength( strlen(msg)); // headers will be auto flushed
            connection()->send((const char *)msg , strlen(msg));
            connection()->Close();  // wrong we should close close after write is successful. Check the callback onSendCallback function
        }
        
        
         void HttpResponder::onPayload(const std::string&  body )
         {
            
              
              
             try
             {
                settingCam = json::parse(body.c_str());
                
                Settings::configuration.rtsp = settingCam["rtsp"];
                
                SInfo << "reconfigure Camera settings " << body << std::endl;
             }
             catch(...)
             {
                 settingCam = nullptr;
             }
                  
              
              
         }
        void HttpResponder::onRequest(net::Request& request, net::Response& response) {
            STrace << "On complete" << std::endl;
            
            std::string msg;
            if( settingCam != nullptr)
                msg = "Success";
            else
                msg = "failure";
                
            response.setContentLength( msg.length()); // headers will be auto flushed
            connection()->send((const char *)msg.c_str() , msg.length());
            connection()->Close();  // wrong we should close close after write is successful. Check the callback onSendCallback function
        }


        ReadMp4::ReadMp4( std::string ip, int port, net::ServerConnectionFactory *factory ): net::HttpServer(  ip, port,  factory, true) {

           // self = this;

//	    fragmp4_filter = new DummyFrameFilter("fragmp4", this);
//            fragmp4_muxer = new FragMP4MuxFrameFilter("fragmp4muxer", fragmp4_filter);
//
//            info = new InfoFrameFilter("info", nullptr);
//
//            txt = new TextFrameFilter("txt", this);
//            
//
//            #if FILEPARSER
//            ffparser = new FFParse(AUDIOFILE, VIDEOFILE,  fragmp4_muxer, info, txt );
//
//            ffparser->start();
//            #else
//            ffparser = new LiveThread("live");
//            
//            ffparser->start();
//            
//          
//            
//            ctx = new LiveConnectionContext(LiveConnectionType::rtsp, Settings::configuration.rtsp1, slot, tcprequest, fragmp4_muxer, info, txt); // Request livethread to write into filter info
//            ffparser->registerStreamCall(*ctx);
//            ffparser->playStreamCall(*ctx);
          

  //         #endif


            

            

        }

        ReadMp4::~ReadMp4() {
            SInfo << "~ReadMp4( )";
             
      
            
            ffparser->stopStreamCall(*ctx);

            ffparser->deregisterStreamCall(*ctx);
            ffparser->stop();
            ffparser->join();



            delete ffparser;
            delete ctx;
            delete fragmp4_filter;
            delete fragmp4_muxer;
            delete info;
            delete txt;
        }


        void  ReadMp4::on_read(net::Listener* connection, const char* msg, size_t len) {

//            //connection->send("arvind", 6 );
//   
//            
//             std::string got = std::string(msg, len);
//             SInfo << "restart  " << got;
//                
//              #if FILEPARSER
//
//              if( got == "reset")
//              ffparser->reset();  
//            
//              #else
//
//               if( got == "reset")
//               {
//                    //SInfo  << "reset";
//                   // fragmp4_muxer->resetParser = true ;//  
//               }
//               else
//               {
//                   if( !strncmp(msg,"rtsp",4 ) && critical_sec++ == 0  )
//                   {
//                        broadcast("reset" , 5, false);
//
//                        ffparser->stopStreamCall(*ctx);
//
//                        ffparser->deregisterStreamCall(*ctx);
//                        
//
//                      //  fragmp4_muxer->resetParser = true ;
//
//                        Settings::configuration.rtsp2 = got;
//
//                        ffparser->stop();
//                        ffparser->join();
//                        
//
//
//                        delete ffparser;
//                        delete ctx;
//                        delete fragmp4_filter;
//                        delete fragmp4_muxer;
//                        delete info;
//                        delete txt;
//                        
//                        fragmp4_filter = new DummyFrameFilter("fragmp4", this);
//                        fragmp4_muxer = new FragMP4MuxFrameFilter("fragmp4muxer", fragmp4_filter);
//
//                        info = new InfoFrameFilter("info", nullptr);
//
//                        txt = new TextFrameFilter("txt", this);
//                        
//                        ffparser = new LiveThread("live");
//                        
//                        ffparser->start();
//                        
//                      
//                        
//                        ctx = new LiveConnectionContext(LiveConnectionType::rtsp, Settings::configuration.rtsp2, slot, tcprequest, fragmp4_muxer, info, txt); // Request livethread to write into filter info
//                        ffparser->registerStreamCall(*ctx);
//                        ffparser->playStreamCall(*ctx);
//
//
//
//
//                        // SInfo <<  "slot " <<  ++slot ;
//
//
//                        // ctx = new LiveConnectionContext(LiveConnectionType::rtsp, Settings::configuration.rtsp2, 1, tcprequest, fragmp4_muxer, info); // Request livethread to write into filter info
//                        // ffparser->registerStreamCall(*ctx);
//                        // ffparser->playStreamCall(*ctx);
//                        
//                        critical_sec =0;
//                   }
//                 
//                   
//               }
//             
//             
//              #endif
//
//                
////                else if( got == "mute")
////                    ffparser->restart(true);
////                else if( got == "unmute")
////                    ffparser->restart(false);   
////                else if( got  == "hd")
////		    ffparser->resHD(true);
////	        else if (got == "cif")
////		   ffparser->resHD(false);


        }
    
        void ReadMp4::broadcast(const char * data, int size, bool binary, bool is_first  )
        {
           // conn->send( data, size, binary    );
            static int noCon =0;
            
            if(noCon !=this->GetNumConnections())
                
            {
                noCon = this->GetNumConnections();
                SInfo << "No of Connectons " << noCon;
            }

            for (auto* connection :  this->GetConnections())
            {
                net::HttpConnection* cn = (net::HttpConnection*)connection;
                

                
                 net::WebSocketConnection *con = ((net::HttpConnection*)connection)->getWebSocketCon();
                 if(con)
                 con->push(data ,size, binary, is_first );
            }

        

        }
        
        void ReadMp4::run() {
             
        }
 
        
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////

    }
}
