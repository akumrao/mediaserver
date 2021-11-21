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

extern "C"
{
//#include <libavutil/timestamp.h>
#include <avformat.h>
}

#define tcprequest true

#include "http/websocket.h"

namespace base {
    
    fmp4::ReadMp4 *self;

    namespace fmp4 {

        ReadMp4::ReadMp4( std::string ip, int port, net::ServerConnectionFactory *factory ): net::HttpServer(  ip, port,  factory, true) {

            self = this;

	    fragmp4_filter = new DummyFrameFilter("fragmp4", this);
            fragmp4_muxer = new FragMP4MuxFrameFilter("fragmp4muxer", fragmp4_filter);

            info = new InfoFrameFilter("info", nullptr);

            txt = new TextFrameFilter("txt", this);
            
            
             #if FILEPARSER ==1
                 ffparser = new FFParse(AUDIOFILE, VIDEOFILE,  fragmp4_muxer, info, txt );

                ffparser->start();
             #elif FILEPARSER ==2
               
                ffparser = new FileThread("file");
                          
                ctx = new LiveConnectionContext(LiveConnectionType::rtsp, Settings::configuration.rtsp1, slot, tcprequest, fragmp4_muxer, info, txt); // Request livethread to write into filter info
                ffparser->registerStreamCall(*ctx);
                ffparser->playStreamCall(*ctx);
                
                 ffparser->start();
           
            #else
            ffparser = new LiveThread("live");
            
            ffparser->start();
            
          
            
            ctx = new LiveConnectionContext(LiveConnectionType::rtsp, Settings::configuration.rtsp1, slot, tcprequest, fragmp4_muxer, info, txt); // Request livethread to write into filter info
            ffparser->registerStreamCall(*ctx);
            ffparser->playStreamCall(*ctx);
          

           #endif


            

            

        }

        ReadMp4::~ReadMp4() {
            SInfo << "~ReadMp4( )";
             
            ffparser->stop();
            ffparser->join();
            delete ffparser;
        }


        void  ReadMp4::on_read(net::Listener* connection, const char* msg, size_t len) {

            //connection->send("arvind", 6 );
   
            
             std::string got = std::string(msg, len);
             SInfo << "restart  " << got;
                
               #if FILEPARSER ==1

              if( got == "reset")
              ffparser->reset();  
             
              #elif FILEPARSER ==2
            
              #else

               if( got == "reset")
               {
                    //SInfo  << "reset";
                    fragmp4_muxer->resetParser = true ;//  
               }
               else
               {
                   if( !strncmp(msg,"rtsp",4 ) && critical_sec++ == 0  )
                   {
                        broadcast("reset" , 5, false, false);

                        ffparser->stopStreamCall(*ctx);

                        ffparser->deregisterStreamCall(*ctx);
                        

                        fragmp4_muxer->resetParser = true ;

                        Settings::configuration.rtsp2 = got;

                        ffparser->stop();
                        ffparser->join();
                        


                        delete ffparser;
                        delete ctx;
                        delete fragmp4_filter;
                        delete fragmp4_muxer;
                        delete info;
                        delete txt;
                        
                        fragmp4_filter = new DummyFrameFilter("fragmp4", this);
                        fragmp4_muxer = new FragMP4MuxFrameFilter("fragmp4muxer", fragmp4_filter);

                        info = new InfoFrameFilter("info", nullptr);

                        txt = new TextFrameFilter("txt", this);
                        
                        ffparser = new LiveThread("live");
                        
                        ffparser->start();
                        
                      
                        
                        ctx = new LiveConnectionContext(LiveConnectionType::rtsp, Settings::configuration.rtsp2, slot, tcprequest, fragmp4_muxer, info, txt); // Request livethread to write into filter info
                        ffparser->registerStreamCall(*ctx);
                        ffparser->playStreamCall(*ctx);




                        // SInfo <<  "slot " <<  ++slot ;


                        // ctx = new LiveConnectionContext(LiveConnectionType::rtsp, Settings::configuration.rtsp2, 1, tcprequest, fragmp4_muxer, info); // Request livethread to write into filter info
                        // ffparser->registerStreamCall(*ctx);
                        // ffparser->playStreamCall(*ctx);
                        
                        critical_sec =0;
                   }
                 
                   
               }
             
             
              #endif

                
//                else if( got == "mute")
//                    ffparser->restart(true);
//                else if( got == "unmute")
//                    ffparser->restart(false);   
//                else if( got  == "hd")
//		    ffparser->resHD(true);
//	        else if (got == "cif")
//		   ffparser->resHD(false);


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
