/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */


#include "fmp4.h"

#include "base/define.h"
#include "base/test.h"
#include <thread>
#include "ffparse.h"

extern "C"
{
//#include <libavutil/timestamp.h>
#include <avformat.h>
}


#include "http/websocket.h"
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

         
           ffparser = new FFParse(AUDIOFILE, VIDEOFILE,  fragmp4_muxer, info, txt );
           ffparser->start();    

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
             SInfo << "on_read " << got;
                
                //  m_ping_timeout_timer.Reset();
                //  m_packet_mgr.put_payload(std::string(data,sz));
                if( got == "reset")
                {
                  //  ffparser->reset();    
                }
                else
                {
		    
//                     broadcast("reset" , 5, false);
//
//                    int x = 0;
//                    if( got  == "hd")
//                        ffparser->resHD(true);
//                    else if (got == "sd")
//                        ffparser->resHD(false);
                    // hd or sd
                    if( len == 2  && critical_sec++ == 0  )
                   {
                        broadcast("reset" , 5, false);

                 

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
                        
                        ffparser = new FFParse(AUDIOFILE, VIDEOFILE,  fragmp4_muxer, info, txt );
                        
                        ffparser->start();
                        
                      
                        
           




                        // SInfo <<  "slot " <<  ++slot ;


                        // ctx = new LiveConnectionContext(LiveConnectionType::rtsp, Settings::configuration.rtsp2, 1, tcprequest, fragmp4_muxer, info); // Request livethread to write into filter info
                        // ffparser->registerStreamCall(*ctx);
                        // ffparser->playStreamCall(*ctx);
                        
                        critical_sec =0;
                   }

                }
//                else if( got == "mute")
//                    ffparser->restart(true);
//                else if( got == "unmute")
//                    ffparser->restart(false);   
//                else if( got  == "hd")
//		         ffparser->resHD(true);
//	            else if (got == "sd")
//		          ffparser->resHD(false);
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
#if HTTPSSL
                 net::WebSocketConnection *con = ((net::HttpsConnection*)connection)->getWebSocketCon();
#else
                 net::WebSocketConnection *con = ((net::HttpConnection*)connection)->getWebSocketCon(); 
#endif               
                 if(con)
                 con->push(data ,size, binary );
            }

        

        }
        
        void ReadMp4::run() {
             
        }
 
        
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////

    }
}
