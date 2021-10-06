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
//#include "ffparse.h"
#include "livethread.h"
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


namespace base {
    
    fmp4::ReadMp4 *self;

    namespace fmp4 {

        ReadMp4::ReadMp4( std::string ip, int port, net::ServerConnectionFactory *factory ): net::HttpServer(  ip, port,  factory, true){
            
         self = this;
         
         
           
//
//        fragmp4_muxer.activate(); // don't forget!
//
//        std::cout << name << "starting threads" << std::endl;
//        livethread.startCall();
//
//        std::cout << name << "registering stream" << std::endl;
//        LiveConnectionContext ctx = LiveConnectionContext(LiveConnectionType::rtsp, std::string(stream_1), 2, &info); // Request livethread to write into filter info
//        livethread.registerStreamCall(ctx);
//
//        // sleep_for(1s);
//
//        std::cout << name << "playing stream !" << std::endl;
//        
           // ffparser = new FFParse(this, AUDIOFILE, VIDEOFILE);
            ffparser = new LiveThread("live");
            
            ffparser->start();    
            
           LiveConnectionContext *ctx = new LiveConnectionContext(LiveConnectionType::rtsp, std::string("stream_1"), 2, nullptr); // Request livethread to write into filter info
           ffparser->registerStreamCall(*ctx);
           
           ffparser->playStreamCall(*ctx);
                

            

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
                
                //  m_ping_timeout_timer.Reset();
                //  m_packet_mgr.put_payload(std::string(data,sz));
//                if( got == "reset")
//                    ffparser->reset();    
//                else if( got == "mute")
//                    ffparser->restart(true);
//                else if( got == "unmute")
//                    ffparser->restart(false);   
//                else if( got  == "hd")
//		    ffparser->resHD(true);
//	        else if (got == "cif")
//		   ffparser->resHD(false);
            //con->send( msg, len );

           // sendAll(msg, len);

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
