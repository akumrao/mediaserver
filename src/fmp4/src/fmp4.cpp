/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "fmp4.h"

#include "ff/ff.h"
#include "ff/mediacapture.h"
#include "base/define.h"
#include "base/test.h"
#include <thread>
#include "ffparse.h"

extern "C"
{
//#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
}


#define SERVER_HOST  "127.0.0.1"               
#define SERVER_PORT 8000

#define AUDIOFILE  "/var/tmp/songs/hindi.pcm"               
#define VIDEOFILE  "/experiment/live/testProgs/test.264"
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

        ReadMp4::ReadMp4( std::string ip, int port, net::ServerConnectionFactory *factory ): net::HttpServer(  ip, port,  factory){
            
         self = this;
         
            ffparser = new FFParse(this, AUDIOFILE, VIDEOFILE);
            ffparser->start();    

        }

        ReadMp4::~ReadMp4() {
            SInfo << "~ReadMp4( )";
            ffparser->stop();
            ffparser->join();
            delete ffparser;
        }

//        void ReadMp4::websocketConnect() {
//
//
//            conn = new net::HttpClient("ws", SERVER_HOST, SERVER_PORT, "/");
//	    //conn = new net::HttpsClient("wss", SERVER_HOST, SERVER_PORT, "/"); for https
//
//
//            //            {
//            //                m_client = new HttpClient("ws", _host, _port, url.str());
//            //            }
//            //            else
//            //            {
//            //                 m_client = new HttpsClient("wss", _host, _port, url.str());
//            //            }
//
//            // conn->Complete += sdelegate(&context, &CallbackContext::onClientConnectionComplete);
//            conn->fnComplete = [&](const net::Response & response) {
//                std::string reason = response.getReason();
//                net::StatusCode statuscode = response.getStatus();
//                std::string body = conn->readStream() ? conn->readStream()->str() : "";
//                STrace << "SocketIO handshake response:" << "Reason: " << reason << " Response: " << body;
//            };
//
//            conn->fnPayload = [&](net::HttpBase * con, const char* data, size_t sz) {
//                std::string got = std::string(data, sz);
//                STrace << "client->fnPayload " << got;
//                
//                //  m_ping_timeout_timer.Reset();
//                //  m_packet_mgr.put_payload(std::string(data,sz));
//                if( got == "reset")
//                    ffparser->reset();    
//            };
//
//
//            conn->fnConnect = [&](net::HttpBase * con) {
//                STrace << "onconnect ";
//                
//                //this->start();
//                ffparser = new FFParse(this, AUDIOFILE, VIDEOFILE);
//                ffparser->start();    
//                // conn->send(x, 2, true);
//            };
//
//
//            conn->fnClose = [&](net::HttpBase * con, std::string str) {
//                STrace << "client->fnClose " << str;
//
//                //on_close();
//            };
//
//            //  conn->_request.setKeepAlive(false);
//            conn->setReadStream(new std::stringstream);
//            conn->send();
//            LTrace("sendHandshakeRequest over")
//            
//
//        }

        void  ReadMp4::on_read(net::Listener* connection, const char* msg, size_t len) {

            //connection->send("arvind", 6 );
   
            
             std::string got = std::string(msg, len);
             STrace << "on_read " << got;
                
                //  m_ping_timeout_timer.Reset();
                //  m_packet_mgr.put_payload(std::string(data,sz));
                if( got == "reset")
                    ffparser->reset();    
               
            
        

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
                 con->send(data ,size, binary );
            }

        

        }
        
         void ReadMp4::run() {
             
         }
#if 0 
         void ReadMp4::websocketConnect() {
             
             runwebsocket( 1000000 ); // millisecond
             
         }


        void ReadMp4::run() {


            std::ifstream bunnyFile;
            bunnyFile.open("/experiment/fmp4/testfrag.mp4", std::ios_base::in | std::ios_base::binary);

            char buf[ MAX_CHUNK_SIZE];

            while (bunnyFile.good() && !stopped()) {
                bunnyFile.read(buf, MAX_CHUNK_SIZE);
                int nRead = bunnyFile.gcount();
                if (nRead > 0) {
                    // dc->sendDataMsg("ravind");

                   // conn->send((const char*) buf, nRead, true);


                    std::this_thread::sleep_for(std::chrono::milliseconds(25));


                    // while( pc->data_channel_->buffered_amount()  >  12 * 1024 * 1024 )
                    // std::this_thread::sleep_for(std::chrono::milliseconds(10));

                }

                SInfo << "Sent message of size " << nRead;
            }


            SInfo << "fmp4 thread exit";

            // fileName = "/var/tmp/videos/test.mp4";
            fileName = "/var/tmp/kunal720.mp4";
            //fmp4(fileName.c_str(), "fragTmp.mp4");
            //fmp4(fileName.c_str());
        }

        int ReadMp4::fmp4(const char *in_filename, const char *out_filename, bool fragmented_mp4_options) {



        }
        
        
        
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void ReadMp4::onConnect(int socketID) {
            // Give this connection a random user ID
         //   const string& handle = "User #" + Util::toString(socketID);
           // Util::log("New connection: " + handle);

            // Associate this handle with the connection
           // this->setValue(socketID, "handle", handle);

            // Let everyone know the new user has connected
         //   this->broadcast(handle + " has connected.");
            SInfo << "Connected  " << socketID;
        }

        void ReadMp4::onMessage(int socketID, const string& data) {
            // Send the received message to all connected clients in the form of 'User XX: message...'
           // Util::log("Received: " + data);
           // const string& message = this->getValue(socketID, "handle") + ": " + data;

           // this->broadcast(message);
             SInfo << "onMessage form "  << socketID << " msg " << data; 
             
              if( data == "reset")
                    ffparser->reset();  
            
        }

        void ReadMp4::onDisconnect(int socketID) {
 //           const string& handle = this->getValue(socketID, "handle");
           // Util::log("Disconnected: " + handle);

//            // Let everyone know the user has disconnected
//            const string& message = handle + " has disconnected.";
//            for (map<int, Connection*>::const_iterator it = this->connections.begin(); it != this->connections.end(); ++it)
//                if (it->first != socketID)
//                    // The disconnected connection gets deleted after this function runs, so don't try to send to it
//                    // (It's still around in case the implementing class wants to perform any clean up actions)
//                    this->send(it->first, message);
            
             SInfo << "onDisconnect  " << socketID;
        }

        void ReadMp4::onError(int socketID, const string& message) {
            SInfo << "onError form "  << socketID << " msg " << message; 
        }

        
        
//        void ReadMp4::send(const char * data, int size, bool binary)
//        {
//            
//        }
//        
        
      
#endif  
        
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////

    }
}
