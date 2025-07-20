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
#include "MVExtractor.h"

#include "ffmpeg_h.h"

#define tcprequest true

#include "http/websocket.h"

#define store "/mnt/clips-storage/"


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
             
            std::string uri = request.getURI();
            
            size_t pos = uri.find("?md=");
            if ( pos != std::string::npos)
            {
                uri = uri.substr(pos+4, uri.size()); //str.siz
                base::cnfg::Configuration config;
                config.load(store+uri + "/"+ uri +".js");
                msg= config.root.dump(4);
                sendResponse(msg, true);    
                return;  
            }
                    
            
           
            
            //if(authcheck( request, msg, true ))
            {
                msg =  Settings::getNode();
                sendResponse(msg, true);     
            }
//            else
//            {
//               sendResponse(msg, false);
//            }
          
     
        }
        
 
        
       void HttDeleteResponder::onPayload(const std::string&  body, net::Request& request)
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

                STrace << "Incoming connection from " << ": Request:\n" << request << std::endl;

                if( !request.has("Host"))
                {

                    SError << "Incoming connection does not have host "  << request.getMethod() << " uri: <<  " << request.getURI() << std::endl;

                    return new net::BasicResponder(conn);
                }

                SDebug << "Incoming connection from: " << request.getHost() << " method: " << request.getMethod() << " uri: <<  " << request.getURI() << std::endl;

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


        ReadMp4::ReadMp4( std::string ip, int port, StreamingResponderFactory1 *factory, bool multithreaded  ): net::HttpsServer(  ip, port,  factory, multithreaded) {

            self = this;
            
            
            
           json &rtsp =  Settings::configuration.root["rtsp"];
    
           for (json::iterator it = rtsp.begin(); it != rtsp.end(); ++it) 
           {
              std::string key;

            //  if(node.is_object())
                 key = it.key();
             /// else
               //  key = *it;
                 
                 
               std::string add;
               bool md = true;
               addCamera( key, md);

           }
            

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
        
        
        ReadMp4::stParser::stParser(ReadMp4 *mp4this,  std::string cam , std::string file)
        {
         
             
           // std::string add;

           // if( Settings::getNodeState(cam, "rtsp" , add ))
            {



                 
               fragmp4_filter = new fmp4::DummyFrameFilter("fragmp4", cam, mp4this);
               
               txt = new fmp4::TextFrameFilter("txt", cam, mp4this);
                
               fragmp4_muxer = new fmp4::FragMP4MuxFrameFilter("fragmp4muxer",  cam , fragmp4_filter , fragmp4_filter);

               info = new fmp4::InfoFrameFilter("info", nullptr);

              

                fileparser = new FFParse(nullptr, file.c_str(),  fragmp4_muxer, info, txt );

                fileparser->start();



                
            }
//            else
//            {
//                SError << "Could not find camera at Json Repository "  << cam; 
//            }


        }
        

	ReadMp4::stParser::stParser(ReadMp4 *mp4this,  std::string cam, bool &md)
        {
         
             
            std::string add;

            if( Settings::getNodeState(cam, "rtsp" , add ))
            {


               filemp4_filter = new fmp4::FileFrameFilter("filemp4", cam);
                
               fragmp4_filter = new fmp4::DummyFrameFilter("fragmp4", cam, mp4this);
               
               txt = new fmp4::TextFrameFilter("txt", cam, mp4this);
               
                
               fragmp4_muxer = new fmp4::FragMP4MuxFrameFilter("fragmp4muxer",  cam , fragmp4_filter , md?txt:nullptr);
               
               filemp4_muxer = new fmp4::FragMP4MuxFrameFilter("fragmp4muxer",  cam , filemp4_filter , nullptr);
               
               
               mvExtractor= new MVExtractor("MVExtractor", cam, txt, filemp4_muxer , filemp4_filter);

               info = new fmp4::InfoFrameFilter("info", nullptr);


               ffparser = new fmp4::LiveThread("live");
               ffparser->start();

              //ctx = new fmp4::LiveConnectionContext(fmp4::LiveConnectionType::rtsp, add, slot, cam, tcprequest, fragmp4_muxer , info, txt, nullptr); // Request livethread to write into filter info
  
               ctx = new fmp4::LiveConnectionContext(fmp4::LiveConnectionType::rtsp, add, slot, cam, tcprequest, nullptr , info, txt, mvExtractor); // Request livethread to write into filter info
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
            if(ffparser)
            {
                ffparser->stopStreamCall(*ctx);

                ffparser->deregisterStreamCall(*ctx);
                ffparser->stop();
                ffparser->join();


                delete ffparser;
                ffparser =nullptr;
            }
            
            
            if(fileparser)
            {
                fileparser->stop();
                fileparser->join();

                delete fileparser;
                fileparser =nullptr;
            }
            
            if(ctx)
            delete ctx;
            ctx = nullptr;
            
            if(fragmp4_filter)        
             delete fragmp4_filter;
            fragmp4_filter = nullptr;
            
           if(fragmp4_muxer)
             delete fragmp4_muxer;
             fragmp4_muxer = nullptr;
            
            if(info)
            delete info;
            info = nullptr;
            
            if(txt)
            delete txt;
            txt = nullptr;
            
            
            if(filemp4_filter)
            delete filemp4_filter;
            filemp4_filter = nullptr;
            
            if(filemp4_muxer)
            delete filemp4_muxer;
            filemp4_muxer = nullptr;
            
            
        }
        
        
        void ReadMp4::on_wsclose(net::Listener* connection)
        {
             
            net::WebSocketConnection* con = (net::WebSocketConnection*)connection;
              
            if(con)
            {
              SInfo << "on_close "  <<  con->key;

               //delCamera(con->key);
               deleteAsync( con->key);
            }
             
         }
         
        
         void ReadMp4::readConfig( std::string &file)
         {
            Settings::readConfig(file);
         }
         

         void ReadMp4::get_cam( std::string &cam, std::string &value)
         {
            Settings::getNodeID( cam , value);
         }


         void ReadMp4::set_cam( std::string &cam,  std::string &key, std::string &value )
         {
            Settings::setNodeValue(cam,key, value );
         }


         void ReadMp4::addCamera( std::string &cam , bool &md)
         {
            SInfo << "Start Camera : " << cam;
           
             
            mutCam.lock();
            
            if( parser.find(cam)  !=   parser.end())
            {
                 ++parser[cam ]->number;
            }
            else
            {
               parser[cam ]  = new stParser(this, cam, md);
               ++parser[cam ]->number;
            }
                
           // SInfo << "Start Camera : " << cam  << " no " << parser[cam ]->number;
            
            mutCam.unlock();
                
         }
         
         void ReadMp4::playclip( std::string &cam, std::string file)
         {
           
             
            mutCam.lock();
            
            if( parser.find(cam)  !=   parser.end())
            {
                 ++parser[cam ]->number;
            }
            else
            {
               parser[cam ]  = new stParser(this,  cam, file);
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
         

        void ReadMp4::deleteAsync( std::string cam) // asyncronous delete 
        {
              
             mutDelcamera.lock();
             vecDelcamera.push(cam);
             mutDelcamera.unlock();
             base::Thread::start();
              
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
              
              this->join();
          }
          
          
        void ReadMp4::on_wsconnect(net::Listener* connection)
        {
           // SInfo << "on_wsconnect";
        }

        void  ReadMp4::on_wsread(net::Listener* connection, const char* msg, size_t len) {

            //connection->send("arvind", 6 );
   
            
            std::string camT = std::string(msg, len);
            
            if( !strncmp(msg, "MD_",3))
            {
                const char delim = '_';
 
                std::vector<std::string> out;

                base::util::split(camT, delim, out);

                for (auto &s: out) {
                    std::cout << s << std::endl;
                }
                
                std::string clipCam =  out[0] +"_" + out[1];
                
                net::WebSocketConnection *con = (net::WebSocketConnection *)connection;

                if(con)
                {
                  con->key = clipCam;
                }
                
                base::cnfg::Configuration config;
                
                config.load( store + out[1] + "/"+ out[1] +".js");
                
                std::string h264File;
                if( config.getRaw( camT,  h264File))
                {
                    playclip( clipCam,  h264File );
                    
                    SInfo << " Play Clip " <<  h264File;
                }
                else
                {
                     SError << " Clip not found for MD trigger " <<  camT;
                }
                

                 
                return;
            }
            else //if( !strncmp(msg, "all",3))
            {
                net::WebSocketConnection *con = (net::WebSocketConnection *)connection;
                if(con)
                {
                  con->key = camT;
                }
                return ;
            }
            
            
            // Enable below code if you want to add and delete camera with rest apis
            net::WebSocketConnection *con = (net::WebSocketConnection *)connection;

            if(con)
            {
              con->key = camT;
            }
            
             
            std::string add;
            bool md = false;

            if( !Settings::getNodeStates(camT, "rtsp" , add, "md", md ))
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
             
           
            
       
            
           addCamera( camT, md);
           
           
           
           
           
             
             
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
    
        void ReadMp4::broadcast(const char * data, int size,  FRAMETYPE frameType, int fametype , std::string &cam  )
        {
           // conn->send( data, size, binary    );
 //           static int noCon =0;
            
//            if(noCon !=this->GetNumConnections())
//                
//            {
//                noCon = this->GetNumConnections();
//                SInfo << "No of Connectons " << noCon;
//            }
            

            for (auto* connection :  this->GetConnections())
            {
                net::HttpsConnection* cn = (net::HttpsConnection*)connection;
                if(cn)
                {
                    net::WebSocketConnection *con = ((net::HttpsConnection*)cn)->getWebSocketCon();
                    if(con && con->key == "all" )
                     con->push(data ,size, fametype, fametype);
                    else if(con && con->key == cam )
                    {
                        con->push(data ,size, fametype, fametype);
                    }
                }
            }
            
            if(frameType == FRAMETYPE::TEXTDEL)
            {
            
                Settings::setNodeState(cam , std::string(data, size) );
                
                deleteAsync(cam);
                
               // delCamera( cam); // nerver do that otherwise live555 will crash
            }

        }
        
        void ReadMp4::run() {
       
            std::string sCamera;
            mutDelcamera.lock();
            sCamera = vecDelcamera.top();
            vecDelcamera.pop();
            mutDelcamera.unlock();

            delCamera(sCamera);
        }
        
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////

    }
}
