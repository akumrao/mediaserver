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


    select options

    https://streaming.pro-vigil.info:8080

    select Headers

    enter
    key   admin@passsword
    exp   360
    perm   w

    it will generate token
    45674E7A-7936-4946-8E69-4A6C78537A2B^w^0^0^0^1652121492^25aece8f5854b8b4f1951a206332f9bbb676d9e6


    Select Get 
    add
    token 

 */

#include "restApi.h"

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
    
  

    namespace web_rtc {

        RestApi *self;
         
   
        void HttpPostResponder::onPayload(const std::string&  body , net::Request& request)
        {
            
            try
            {
                settingCam = json::parse(body.c_str());
            }

            catch(...)
            {
                settingCam = nullptr;
                return;
            }

            
            if(request.getURI() == "/api/login")
            {
                std::string id = settingCam["uname"].get<std::string>();
                std::string pass = settingCam["psw"].get<std::string>();
                
                request.set( "key", id + "@" +  pass);
                request.set( "uid", id );
                request.set( "exp", "3600" );
                request.set( "perm", "w" );
            
                    
                if(authcheck( request, msg ))
                {
                    return ;
                }
                            
       
                settingCam.clear();
                settingCam= nullptr;
                
                return ;
            }
            
            else if(request.getURI() == "/api/account")
            {
            
                std::string user = settingCam["uname"].get<std::string>();
                std::string pass = settingCam["psw"].get<std::string>();

                
                request.set( "key", user + "@" +  pass);
                request.set( "uid", user );
                request.set( "exp", "3600" );
                request.set( "perm", "w" );
            
                if(authcheck( request, msg ))
                {
                     return ;
                }
               
       
               settingCam.clear();
               settingCam= nullptr;
                
                return ;
            }
            else if(request.getURI() == "/api/post")
            {
                if(authcheck( request, msg ))
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

        }

        void HttpPostResponder::onRequest(net::Request& request, net::Response& response) 
        {
            STrace << "On complete" << std::endl;
            

            if( settingCam != nullptr)
            {
               // msg = "Success";
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
            
            if(authcheck( request, msg ))
            {
            
                std::string prvrecording;
                std::string currecording;
                std::string camid;
                
                bool found;
                 
                try
                {
                    settingCam = json::parse(body.c_str());
                     
                    camid = settingCam.items().begin().key();
                    currecording = settingCam.items().begin().value()["recording"];
                    found = Settings::getNodeState(camid, "recording" , prvrecording );
                    ret = Settings::putNode( settingCam, vec);
                }
                catch(...)
                {
                     settingCam = nullptr;
                     return;
                }
                
                SInfo << "Rest API: Put Camera " << body << std::endl;
                   
                if(found)
                {
                    if( prvrecording !=  currecording )
                    {
                       if( currecording == "on")
                       {
                           sig._capturer.startRecording(camid);
                       }
                       else
                       {
                            sig._capturer.stopRecording(camid);
                       }
                    }
                        
                }else if ( currecording == "on"  )
                {
                    sig._capturer.startRecording(camid);
                }
           
            }
            else
            {   
                settingCam.clear();
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
            
            
            if(request.getURI() == "/api/camlist")
            {     

                std::string msg;

                if(authcheck( request, msg ))
                {
                    msg =  Settings::getNode();
                    sendResponse(msg, true);     
                }
                else
                {
                   sendResponse(msg, false);
                }
            }
            else if(request.getURI() == "/api/camtree")
	    {
		
                json ret;
                
                json node =  Settings::getJsonNode();
                  
                 
                for (json::iterator it = node.begin(); it != node.end(); ++it)
                {
                    std::string key;
                   
                    json value;

                    if(node.is_object())
                    {   
                        key = it.key();
                        value = it.value();

                    
                        //if( value["state"]=="streaming")
                        {
                            if( value["audio"].is_object())
                            {
                               // if( value["audio"]["state"]=="streaming")
                                {
                                    ret[key]["video"]=key;
                                    ret[key]["audio"]=key;
                                }
                            }
                            else
                                ret[key]["video"]=key;
                        }
                    }
                }
                std::string msg = ret.dump(4);
                sendResponse(msg, true);  

	    }
            else if(request.getURI() == "/api/recordcam")
	    {
		
                json ret;
                
                json node =  Settings::getJsonNode();
                  
                 
                for (json::iterator it = node.begin(); it != node.end(); ++it)
                {
                    std::string key;
                   
                    json value;

                    if(node.is_object())
                    {   
                        key = it.key();
                        value = it.value();

                        if( value.find("recording") != value.end() )
                        {
                          ret.push_back(key);
                        }
                    }
                }
                std::string msg = ret.dump(4);
                sendResponse(msg, true);  

	    }
            else if(request.getURI() == "/api/recordtree")
	    {
		 std::string msg = "Requet without camera info";
                if(!request.has("cam"))
                {
                    sendResponse(msg, false);  
                    return ;
                }
                
                std::string basePath =  Settings::configuration.storage + "CAM"+ request.get("cam") + "/";
                json ret;
                
                listFilesRecursively(basePath.c_str(), ret, true  );
                
                
                for (json::iterator itDate = ret.begin(); itDate != ret.end(); ++itDate) {
                    
                    std::string path =  basePath + itDate.key() + "/manifest.js";
                    
                    base::cnfg::Configuration encconfig;
                    encconfig.load(path);
                    
                    if(encconfig.root.find( itDate.key()) != encconfig.root.end())
                    {
                        json &nodeHr =  encconfig.root[itDate.key()];
                         
                        for (json::iterator itHr = nodeHr.begin(); itHr != nodeHr.end(); ++itHr)
                        {
                            std::string endTime;
                            
                            json &nodeMin =  itHr.value();
                            for (json::iterator itmin = nodeMin.begin(); itmin != nodeMin.end(); ++itmin)
                            {
                                std::string time = itmin.key();
                                std::string tmpString;
                                int min = std::stoi( time.substr(3,2));
                                if( min > 0 &&  min <= 15  )
                                {
                                    tmpString = "1-15"; 
                                }
                                else  if( min > 15 &&  min <= 30  )
                                {
                                    tmpString = "15-30"; 
                                }
                                else  if( min > 30 &&  min <= 45  )
                                {
                                    tmpString = "30-45"; 
                                }
                                else  if( min > 45 &&  min <= 60  )
                                {
                                    tmpString = "45-60";
                                } 
                                
                                itDate.value()[itHr.key()][tmpString]=  request.get("cam") + "/" + itDate.key() + "/" + itHr.key() + "/" + time + "/" ;
                                
                            }
                             
                        }
                        
                    }
                    
                }
                
                msg = ret.dump(4);
                sendResponse(msg, true);  

	    }
            
     
        }
        
 
        
       void HttDeleteResponder::onPayload(const std::string&  body, net::Request& request)
       {
           
            std::string msg;

            if(!authcheck( request, msg ))
            {
                settingCam.clear();
                settingCam = nullptr;
                return; 
            }
                
                    
            try
            {
                
                settingCam = json::parse(body.c_str());
                
                std::vector<std::string>  vec;
                    
                ret = Settings::deleteNode( settingCam, vec);
                
                for( std::string  el : vec)
                {
                     sig.postcloseCamera(el, "Deleted camera with Rest API");  // arvind
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
            
            std::string msg = "Success";
            //if(authcheck( request, msg, false ))
            {
               sendResponse(msg, true);      
            }
//            else
//            {
//               sendResponse(msg, false);
//            }
                
                    
                
        }

        
        

        RestApi::RestApi( std::string ip, int port,   base::web_rtc::Signaler &sig, net::ServerConnectionFactory *factory ): sig(sig), net::HttpsServer(  ip, port,  factory, true) {

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

            base::Thread::start();         

        }

        RestApi::~RestApi() 
        {
            SInfo << "~RestApi( )";
             
            for( const auto& kv  : parser)
	    {
               delete kv.second;

            }
           
            mutDelcamera.lock();
            while (!vecDelcamera.empty()) vecDelcamera.pop(); 
            mutDelcamera.unlock();
            
            condwait.signal();
            
            base::Thread::stop();
            
            base::Thread::join();
            
       } 
        

	 RestApi::stParser::stParser(RestApi *mp4this,  std::string & cam, std::string &date, std::string &hr, std::string &time)
        {
         
             
            std::string add;

            if( Settings::getNodeState(cam, "rtsp" , add ))
            {

                mp4Thread = new Mp4Thread( mp4this , cam, date, hr, time );
                mp4Thread->start();


               // ctx = new fmp4::LiveConnectionContext(fmp4::LiveConnectionType::rtsp, add, slot, cam, tcprequest, fragmp4_muxer , info, txt); // Request livethread to write into filter info
              //  ffparser->registerStreamCall(*ctx);
              //  ffparser->playStreamCall(*ctx);

             //   Settings::configuration.rtsp[cam]["state"]="streaming";
               // Settings::setNodeState(cam , "streaming" );

                SInfo  <<   cam  << " " <<  add;
            }
            else
            {
                SError << "Could not find camera at Json Repository "  << cam; 
            }


        }
        
         
        RestApi::stParser::~stParser()
        {
            
            mp4Thread->stop();
            mp4Thread->join();
            
            delete mp4Thread;
            mp4Thread =nullptr;
             
            SInfo  <<   "~stParser()";
             
//            ffparser->stopStreamCall(*ctx);
//
//            ffparser->deregisterStreamCall(*ctx);
//            ffparser->stop();
//            ffparser->join();
//
//
//            delete ffparser;
//            ffparser =nullptr;
//            
//            if(ctx)
//            delete ctx;
//            ctx = nullptr;
//            
//            if(fragmp4_filter)        
//             delete fragmp4_filter;
//            fragmp4_filter = nullptr;
//            
//            if(fragmp4_muxer)
//             delete fragmp4_muxer;
//             fragmp4_muxer = nullptr;
//            
//            if(info)
//            delete info;
//            info = nullptr;
//            
//            if(txt)
//            delete txt;
//            txt = nullptr;
            
            
            
        }
        
        
        void RestApi::on_wsclose(net::Listener* connection)
        {
             
            net::WebSocketConnection* con = (net::WebSocketConnection*)connection;
              
            if(con)
            {
              SInfo << "on_close "  <<  con->key;

               delCamera(con->key);
            }
             
         }
         
         void RestApi::addCamera( std::string &cam, std::string &date, std::string &hr, std::string &time)
         {
             SInfo << "Start Camera : " << cam;
           
             
            mutCam.lock();
            
            if( parser.find(cam)  !=   parser.end())
            {
                 ++parser[cam ]->refCount;
            }
            else
            {
               parser[cam ]  = new stParser(this,  cam, date, hr, time );
               ++parser[cam ]->refCount;
            }
                
           // SInfo << "Start Camera : " << cam  << " no " << parser[cam ]->refCount;
            
            mutCam.unlock();
                
         }
         
         void RestApi::forceDelCamera(std::string &cam ,  std::string  reason )
         {
             SInfo << "Delete Camera : " << cam << " reason " << reason;  
              mutCam.lock();
              
              
              std::map< std::string,  stParser*>::iterator it = parser.find(cam);
             
              if( it !=   parser.end())
              {
                   parser[cam ]->refCount = 0;
                   
                  // SInfo << "Delete Camera : " << cam  << " no " << parser[cam ]->refCount;
                     
                                     
                   if( parser[cam ]->refCount == 0)
                   {
                       delete parser[cam ];
                       
                       parser.erase(it);
                   }
                       
             }
              mutCam.unlock();
         }
         
          void RestApi::delCamera( std::string &cam)
          {
              
              SInfo << "Delete Camera : " << cam;  
              mutCam.lock();
              
              
              std::map< std::string,  stParser*>::iterator it = parser.find(cam);
             
              if( it !=   parser.end())
              {
                   --parser[cam ]->refCount;
                   
                  // SInfo << "Delete Camera : " << cam  << " no " << parser[cam ]->refCount;
                     
                                     
                   if( parser[cam ]->refCount == 0)
                   {
                       delete parser[cam ];
                       
                       parser.erase(it);
                   }
                       
             }
              mutCam.unlock();
          }
          
          
        void RestApi::on_wsconnect(net::Listener* connection)
        {
           // SInfo << "on_wsconnect";
        }

        void  RestApi::on_wsread(net::Listener* connection, const char* msg, size_t len) {

   
            std::string delimiter = "/";
            
            std::string camT;
            
            std::string date;
            
            std::string hr;
            
            std::string time;
            
            std::string s = std::string(msg, len);
              
            size_t pos = 0;
            if ((pos = s.find(delimiter)) != std::string::npos) {
                camT = s.substr(0, pos);
                std::cout << camT << std::endl;
                s.erase(0, pos + delimiter.length());
            }

            if ((pos = s.find(delimiter)) != std::string::npos) {
                date = s.substr(0, pos);
                std::cout << date << std::endl;
                s.erase(0, pos + delimiter.length());
            }
            
            if ((pos = s.find(delimiter)) != std::string::npos) {
                hr = s.substr(0, pos);
                std::cout << hr << std::endl;
                s.erase(0, pos + delimiter.length());
            }
            
            if ((pos = s.find(delimiter)) != std::string::npos) {
                time = s.substr(0, pos);
                std::cout << time << std::endl;
                s.erase(0, pos + delimiter.length());
            }
         
           
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
             
           
            
       
            
           addCamera( camT, date, hr, time);



        }
    
        void RestApi::broadcast(const char * data, int size, bool binary, int fametype , std::string &cam  )
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
                    if(con && con->key == cam )
                     con->push(data ,size, binary, fametype);
                }
            }
            
            
            if(!binary)
            {
            
                Settings::setNodeState(cam , std::string(data, size) );
                //delCamera( cam); // nerver do that otherwise live555 will crash
            }

        }
        
        
        void HttpGetResponder::listFilesRecursively(const char *basePath, json & ret, bool isDir )
        {
          //  char path[1000];
            struct dirent *dp;
            DIR *dir = opendir(basePath);

            // Unable to open directory stream
            if (!dir)
                return;

            while ((dp = readdir(dir)) != NULL)
            {
                if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
                {
                    printf("%s\n", dp->d_name);

//                    // Construct new path from our base path
//                    strcpy(path, basePath);
//                    strcat(path, "/");
//                    strcat(path, dp->d_name);
                    
                    if(isDir &&  dp->d_type & DT_DIR)
                    {
                       if(ret.find(dp->d_name) == ret.end() ) 
                       ret[dp->d_name] = json::object();
                      
//                                    ret[key]["audio"]=key;
                      //listFilesRecursively(path, ret, isDir);
                    }
                }
            }

            closedir(dir);
        }
  
        void RestApi::postAppMessage(std::string &cam, FRAMETYPE frameType, std::string &reason)
        {
            
     
             
            if(frameType == FRAMETYPE::TEXTDEL)
            {
            
                Settings::setNodeState(cam , reason );
                
                deleteAsync(cam, reason);
                
               // delCamera( cam); // nerver do that otherwise live555 will crash
            }
            
           //  sig.postcloseCamera( cam, reason   );  //arvind
             
          
        }
        

        void RestApi::deleteAsync( std::string cam, std::string &reason) // asyncronous delete 
        {
             SInfo << "RestApi::deleteAsync " << cam ;
              
            CamError camError;
            camError.cam = cam;
            camError.reason= reason; 
            
            mutDelcamera.lock();
            vecDelcamera.push(camError);
            mutDelcamera.unlock();
            condwait.signal();
      
        }


       void RestApi::run()
       {
           
           while(!stopped())
           {
                condwait.wait(); 

                while(vecDelcamera.size() )
                {
                    CamError camError;
                    mutDelcamera.lock();
                    camError = vecDelcamera.front();
                    vecDelcamera.pop();
                    mutDelcamera.unlock();

                    //delCamera(sCamera);
                    sig.postcloseCamera( camError.cam ,camError.reason  );  //arvind
                }
           }    
            
        }


        
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////

    }
}
