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
#include "ffparse.h"
#include "livethread.h"
#include "Settings.h"
#include "http/HttpServer.h"


//extern "C"
//{
//#include <libavutil/timestamp.h>
#include "avformat.h"
//#include <libavcodec/avcodec.h>
//}

#define tcprequest true

#include "http/websocket.h"

namespace base {
    
    fmp4::ReadMp4 *self;

    namespace fmp4 {


         void HttpPostResponder::onPayload(const std::string&  body )
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

        void HttpPostResponder::onRequest(net::Request& request, net::Response& response) {
            STrace << "On complete" << std::endl;
            
            std::string msg;
            if( settingCam != nullptr)
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
        
        
        
        void HttpPutResponder::onPayload(const std::string&  body )
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

        void HttpPutResponder::onRequest(net::Request& request, net::Response& response) {
            STrace << "On complete" << std::endl;
            
            std::string msg;
            if( settingCam != nullptr && ret)
            {   msg = "Success";
                sendResponse(msg, true);
            }
            else
            {
                msg = "failure";
                sendResponse(msg, false);
            }

        }
        
        
       void HttpGetResponder::onPayload(const std::string&  body )
       {
            SInfo << "get Camera settings " << body << std::endl;
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
        
 
        
       void HttDeleteResponder::onPayload(const std::string&  body )
       {
           
            try
            {
                settingCam = json::parse(body.c_str());
                
                ret = Settings::deleteNode( settingCam, vec);
                
                for( std::string  el : vec)
                {
                   //sig.closeCamera(el, "Deleted camera with Rest API");
                    
                    self->forceDelCamera( el, "Deleted camera with Rest API"  );
                    
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

        
        

         net::ServerResponder* StreamingResponderFactory1::createResponder(net::HttpBase* conn)
         {

                auto& request = conn->_request;

                // Log incoming requests
              //  STrace << "Incoming connection from " << ": Request:\n" << request << std::endl;

                SInfo << "Incoming connection from: " << request.getHost() << " method: " << request.getMethod() << " uri: <<  " << request.getURI() << std::endl;

                // Handle websocket connections
                if (request.getMethod() == "POST") {
                    return new HttpPostResponder(conn);
                }
                else if (request.getMethod() == "PUT") {
                    return new HttpPutResponder(conn);
                }
                else if (request.getMethod() == "GET") 
                {
                    
                            
                    std::string uri =   request.getURI();
                    
                    size_t pos = uri.find("?cam=");
                    if ( pos != std::string::npos)
                    {
                        uri.erase(pos); //str.siz
                        request.setURI(uri);
                        return new base::net::HttpResponder(conn);
                    }
                    else if(  uri.find("." ) != std::string::npos)
                    {
                       return new base::net::HttpResponder(conn);
                    }
                    else 
                        return new HttpGetResponder(conn);
                   
                }
                else if (request.getMethod() == "DELETE") {
                    return new HttDeleteResponder(conn);
                }
                
                else if (request.getMethod() == "OPTIONS") {
                    return new HttOptionsResponder(conn);
                }
                else {
                    return new net::BasicResponder(conn);
                }
                
                


        }


        ReadMp4::ReadMp4( std::string ip, int port, StreamingResponderFactory1 *factory ): net::HttpsServer(  ip, port,  factory, true) {

            self = this;

//	    fragmp4_filter = new DummyFrameFilter("fragmp4", this);
//            fragmp4_muxer = new FragMP4MuxFrameFilter("fragmp4muxer", fragmp4_filter);
//
//            info = new InfoFrameFilter("info", nullptr);
//
//            txt = new TextFrameFilter("txt", this);
//            
//            
//             #if FILEPARSER ==1
//                 ffparser = new FFParse(AUDIOFILE, VIDEOFILE,  fragmp4_muxer, info, txt );
//
//                ffparser->start();
//             #elif FILEPARSER ==2
//               
//                ffparser = new FileThread("file");
//                          
//                ctx = new LiveConnectionContext(LiveConnectionType::rtsp, Settings::configuration.rtsp1, slot, tcprequest, fragmp4_muxer, info, txt); // Request livethread to write into filter info
//                ffparser->registerStreamCall(*ctx);
//                ffparser->playStreamCall(*ctx);
//                
//                 ffparser->start();
//           
//            #else
//            ffparser = new LiveThread("live");
//            
//            ffparser->start();
//            
//          
//            
//            ctx = new LiveConnectionContext(LiveConnectionType::rtsp, Settings::configuration.rtsp, slot, tcprequest, fragmp4_muxer, info, txt); // Request livethread to write into filter info
//            ffparser->registerStreamCall(*ctx);
//            ffparser->playStreamCall(*ctx);
//          
//
//           #endif
         

        }

        ReadMp4::~ReadMp4() {
            SInfo << "~ReadMp4( )";
             
            for( const auto& kv  : parser)
	    {
               delete kv.second;

           }
       } 
        

	 ReadMp4::stParser::stParser(ReadMp4 *mp4this,  std::string cam)
        {
         
             
            std::string add;

            if( Settings::getNodeState(cam, "rtsp" , add ))
            {

               fragmp4_filter = new fmp4::DummyFrameFilter("fragmp4", cam, mp4this);
               fragmp4_muxer = new fmp4::FragMP4MuxFrameFilter("fragmp4muxer", fragmp4_filter);

                info = new fmp4::InfoFrameFilter("info", nullptr);

                txt = new fmp4::TextFrameFilter("txt", cam, mp4this);

                ffparser = new fmp4::LiveThread("live");
                ffparser->start();


                ctx = new fmp4::LiveConnectionContext(fmp4::LiveConnectionType::rtsp, add, slot, cam, tcprequest, fragmp4_muxer , info, txt); // Request livethread to write into filter info
                ffparser->registerStreamCall(*ctx);
                ffparser->playStreamCall(*ctx);

             //   Settings::configuration.rtsp[cam]["state"]="streaming";
                Settings::setNodeState(cam , "streaming" );

                SInfo  <<   cam  << " " <<  add;
            }
            else
            {
                SError << "Could not find camera at Json Repository "  << cam; 
            }


        }
        
        ReadMp4::stParser::~stParser()
        {
           ffparser->stop();
            ffparser->join();
            delete ffparser;
        }
        
        
        void ReadMp4::on_wsclose(net::Listener* connection)
        {
             
            net::WebSocketConnection* con = (net::WebSocketConnection*)connection;
              
            if(con)
            {
              SInfo << "on_close "  <<  con->key;

               delCamera(con->key);
            }
             
         }
         
         void ReadMp4::addCamera( std::string &cam)
         {
             SInfo << "Start Camera : " << cam;
           
             
            mutCam.lock();
            
            if( parser.find(cam)  !=   parser.end())
            {
                 ++parser[cam ]->number;
            }
            else
            {
               parser[cam ]  = new stParser(this,  cam);
               ++parser[cam ]->number;
            }
                
           // SInfo << "Start Camera : " << cam  << " no " << parser[cam ]->number;
            
            mutCam.unlock();
                
         }
         
         void ReadMp4::forceDelCamera(std::string &cam ,  std::string  reason )
         {
             SInfo << "Delete Camera : " << cam << " reason " << reason;  
              mutCam.lock();
              
              
              std::map< std::string,  stParser*>::iterator it = parser.find(cam);
             
              if( it !=   parser.end())
              {
                   parser[cam ]->number = 0;
                   
                  // SInfo << "Delete Camera : " << cam  << " no " << parser[cam ]->number;
                     
                                     
                   if( parser[cam ]->number == 0)
                   {
                       delete parser[cam ];
                       
                       parser.erase(it);
                   }
                       
             }
              mutCam.unlock();
         }
         
          void ReadMp4::delCamera( std::string &cam)
          {
              
              SInfo << "Delete Camera : " << cam;  
              mutCam.lock();
              
              
              std::map< std::string,  stParser*>::iterator it = parser.find(cam);
             
              if( it !=   parser.end())
              {
                   --parser[cam ]->number;
                   
                  // SInfo << "Delete Camera : " << cam  << " no " << parser[cam ]->number;
                     
                                     
                   if( parser[cam ]->number == 0)
                   {
                       delete parser[cam ];
                       
                       parser.erase(it);
                   }
                       
             }
              mutCam.unlock();
          }
          
          
        void ReadMp4::on_wsconnect(net::Listener* connection)
        {
           // SInfo << "on_wsconnect";
        }

        void  ReadMp4::on_wsread(net::Listener* connection, const char* msg, size_t len) {

            //connection->send("arvind", 6 );
   
            
            std::string camT = std::string(msg, len);
            
            net::WebSocketConnection *con = (net::WebSocketConnection *)connection;

            if(con)
            {
              con->key = camT;
            }
            
             
            std::string add;

            if( !Settings::getNodeState(camT, "rtsp" , add ))
            {
                    {
                       // postAppMessage("Camera not available, check with Json API Cam: " + camT, from , room  );\
                     
                        std::string msg = "Camera not available, check with Json API Cam: " + camT;
                        
                        if(con)
                        {
                          con->push(msg.c_str() , msg.length(), false, 0);
                        }
                        return;
                    }
            }
             
           
            
       
            
           addCamera( camT);
           
           
           
           
           
             
             
//            #if FILEPARSER ==1
//
//              if( got == "reset")
//              ffparser->reset();  
//             
//            #elif FILEPARSER ==2
//            
//            #else
//
//               if( camT == "reset")
//               {
//                    //SInfo  << "reset";
//                    //fragmp4_muxer->resetParser = true ;//  
//               }
//               else
//               {
//                   if( !strncmp(msg,"rtsp",4 ) && critical_sec++ == 0  )
//                   {
////                        broadcast("reset" , 5, false, false);
////
////                        ffparser->stopStreamCall(*ctx);
////
////                        ffparser->deregisterStreamCall(*ctx);
////                        
////
////                        //fragmp4_muxer->resetParser = true ;
////
////                        Settings::configuration.rtsp = camT;
////
////                        ffparser->stop();
////                        ffparser->join();
////                        
////
////
////                        delete ffparser;
////                        delete ctx;
////                        delete fragmp4_filter;
////                        delete fragmp4_muxer;
////                        delete info;
////                        delete txt;
////                        
////                        fragmp4_filter = new DummyFrameFilter("fragmp4", this);
////                        fragmp4_muxer = new FragMP4MuxFrameFilter("fragmp4muxer", fragmp4_filter);
////
////                        info = new InfoFrameFilter("info", nullptr);
////
////                        txt = new TextFrameFilter("txt", this);
////                        
////                        ffparser = new LiveThread("live");
////                        
////                        ffparser->start();
////                        
////                      
////                        
////                        ctx = new LiveConnectionContext(LiveConnectionType::rtsp, Settings::configuration.rtsp, slot, tcprequest, fragmp4_muxer, info, txt); // Request livethread to write into filter info
////                        ffparser->registerStreamCall(*ctx);
////                        ffparser->playStreamCall(*ctx);
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

                
//                else if( got == "mute")
//                    ffparser->restart(true);
//                else if( got == "unmute")
//                    ffparser->restart(false);   
//                else if( got  == "hd")
//		    ffparser->resHD(true);
//	        else if (got == "cif")
//		   ffparser->resHD(false);


        }
    
        void ReadMp4::broadcast(const char * data, int size, bool binary, int fametype , std::string &cam  )
        {
           // conn->send( data, size, binary    );
 //           static int noCon =0;
            
//            if(noCon !=this->GetNumConnections())
//                
//            {
//                noCon = this->GetNumConnections();
//                SInfo << "No of Connectons " << noCon;
//            }
            
            if(!binary)
            Settings::setNodeState(cam , std::string(data, size) );

            for (auto* connection :  this->GetConnections())
            {
                net::HttpsConnection* cn = (net::HttpsConnection*)connection;
                if(cn)
                {
                    net::WebSocketConnection *con = ((net::HttpsConnection*)cn)->getWebSocketCon();
                    if(con && con->key == cam )
                     con->push(data ,size, binary, fametype);
                }
            }

        }
        
        void ReadMp4::run() {
             
        }
 
        
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////

    }
}
