/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "fmp4.h"

//#include "ff/ff.h"
//#include "ff/mediacapture.h"
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


#define SERVER_HOST  "127.0.0.1"               
#define SERVER_PORT 8000



//#define VIDEOFILE  "/experiment/fmp4/kunal720.264"


//#define MAX_CHUNK_SIZE 10240*8
// maximum send buffer 262144  =1024 *256

//#define highWaterMark  8 * 1048576
//maximum buffer = 16 *1048576 where  1024*1024 =1048576




//#define IOBUFSIZE 40960
//40960*6



#include "http/websocket.h"
//std::string rtsp  = "rtsp://10.170.4.89:8554/testStream";
//std::string rtsp  = "rtsp://192.168.0.19:8554/testStream";

namespace base {
    
    fmp4::ReadMp4 *self;

    namespace fmp4 {

        ReadMp4::ReadMp4( std::string ip, int port, net::ServerConnectionFactory *factory ): net::HttpServer(  ip, port,  factory, true) {

            self = this;

	    fragmp4_filter = new DummyFrameFilter("fragmp4", this);
            fragmp4_muxer = new FragMP4MuxFrameFilter("fragmp4muxer", fragmp4_filter);

            info = new InfoFrameFilter("info", nullptr);

            txt = new TextFrameFilter("txt", this);
            

            #if FILEPARSER
            ffparser = new FFParse(AUDIOFILE, VIDEOFILE,  fragmp4_muxer, info, txt );

            ffparser->start();
            #else
            ffparser = new LiveThread("live");
            
            ffparser->start();
            
          
            
            ctx = new LiveConnectionContext(LiveConnectionType::rtsp, Settings::configuration.rtsp2, slot, false, fragmp4_muxer, info); // Request livethread to write into filter info
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
             STrace << "on_read " << got;
                
              #if FILEPARSER

              if( got == "reset")
              ffparser->reset();  
            
              #else

               if( got == "reset")
               {
                    SInfo  << "reset";
                    fragmp4_muxer->resetParser = true ;//  
               }
               else
               {
                   ffparser->stopStreamCall(*ctx);
                   
                   ffparser->deregisterStreamCall(*ctx);
                   delete ctx;
                    
                   fragmp4_muxer->resetParser = true ;// 
                   SInfo <<  "slot " <<  ++slot ;
                   
                   broadcast("reset" , 5, false);
                   
                   ctx = new LiveConnectionContext(LiveConnectionType::rtsp, Settings::configuration.rtsp2, 1, false, fragmp4_muxer, info); // Request livethread to write into filter info
                   ffparser->registerStreamCall(*ctx);
                   ffparser->playStreamCall(*ctx);
            
                   
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
    
        void ReadMp4::broadcast(const char * data, int size, bool binary   )
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
                 net::WebSocketConnection *con = ((net::HttpConnection*)connection)->getWebSocketCon();
                 if(con)
                 con->push(data ,size, binary );
            }

        

        }
        
        void ReadMp4::run() {
             
        }
 
        
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////

    }
}
