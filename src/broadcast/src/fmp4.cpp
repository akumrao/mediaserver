/* This file is part of mediaserver. A RTSP live server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
  for testing reset api use Postman
 * https://web.postman.co/workspace/My-Workspace~292b44c7-cae4-44d6-8253-174622f0233e/request/create?requestId=e6995876-3b8c-4b7e-b170-83a733a631db
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

#define tcprequest true

#include "http/websocket.h"

namespace base {
    
   fmp4::ReadMp4 *self;

    namespace fmp4 {


        void HttpPostResponder::onPayload(const std::string&  body , net::Request& request)
        {
            
            
            
            if(authcheck( request, msg, true ))
            {
               
                
                try
                {
                   settingCam = json::parse(body.c_str());

                   Settings::postNode( settingCam);

                   SInfo << "reconfigure Camera settings " << body << std::endl;
                }
                catch(...)
                {
                    settingCam = nullptr;
                }
                
            }
            else
            {
                 settingCam = nullptr;
            }
           
            
              
        }

        void HttpPostResponder::onRequest(net::Request& request, net::Response& response) 
        {
            STrace << "On complete" << std::endl;
            

            if( settingCam != nullptr)
            {
                msg = "Success";
                sendResponse(msg, true);
            }
            else
            {
                if(!msg.size())
                msg = "failure";
                
                sendResponse(msg, false);
            }
                
        
        }
        
        
        
        void HttpPutResponder::onPayload(const std::string&  body , net::Request& request)
        {
            
            if(authcheck( request, msg, true ))
            {
            
                try
                {
                    settingCam = json::parse(body.c_str());

                    ret = Settings::putNode( settingCam, vec);

                    SInfo << "Add single Camera " << body << std::endl;
                }
                catch(...)
                {
                     settingCam = nullptr;
                } 
            
            }
            else
            {
                 settingCam = nullptr;
            }

           
              
         }

        void HttpPutResponder::onRequest(net::Request& request, net::Response& response) {
            STrace << "On complete" << std::endl;
            
            if( settingCam != nullptr && ret)
            {   msg = "Success";
                sendResponse(msg, true);
            }
            else
            {
                if(!msg.size())
                msg = "failure";
                sendResponse(msg, false);
            }

        }
        
        
       void HttpGetResponder::onPayload(const std::string&  body ,net::Request& request)
       {
          //  SInfo << "get Camera settings " << body << std::endl;
       }

        void HttpGetResponder::onRequest(net::Request& request, net::Response& response) {
            STrace << "On complete" << std::endl;
            
            
            std::string msg;
            
            if(authcheck( request, msg, true ))
            {
                msg =  Settings::getNode();
                sendResponse(msg, true);     
            }
            else
            {
               sendResponse(msg, false);
            }
          
     
        }
        
 
        
       void HttDeleteResponder::onPayload(const std::string&  body, net::Request& request)
       {
           
            try
            {
                settingCam = json::parse(body.c_str());
                
                std::vector<std::string>  vec;
                    
                ret = Settings::deleteNode( settingCam, vec);
                
                for( std::string  el : vec)
                {
                    sig.closeCamera(el, "Deleted camera with Rest API");
                }
                
                SInfo << "reconfigure Camera settings " << body << std::endl;
             }
             catch(...)
             {
                 settingCam = nullptr;
             }
              
         }

        void HttDeleteResponder::onRequest(net::Request& request, net::Response& response) {
            STrace << "On complete" << std::endl;
            
            std::string msg;
            if( settingCam != nullptr && ret)
            {
                msg = "Success";
                sendResponse(msg, true);
            }
            else
            {
                msg = "failure";
                sendResponse(msg, false);
            }
        }
        
        
   

        void HttOptionsResponder::onRequest(net::Request& request, net::Response& response) {
            STrace << "On complete" << std::endl;
            
            std::string msg;
            if(authcheck( request, msg, false ))
            {
               sendResponse(msg, true);      
            }
            else
            {
               sendResponse(msg, false);
            }
                
                    
                
        }

        
        

        ReadMp4::ReadMp4( std::string ip, int port,   base::wrtc::Signaler &sig, net::ServerConnectionFactory *factory ): sig(sig), net::HttpsServer(  ip, port,  factory, true) {

            self = this;

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
             
      /*
            
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
       */ 
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
    
        void ReadMp4::broadcast(std::string &cam, std::string &reason )
        {
            
             Settings::setNodeState(cam , reason );
             
             sig.closeCamera( cam, reason   );
             
          
        }
        
        void ReadMp4::run() {
             
        }
 
        
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////

    }
}
