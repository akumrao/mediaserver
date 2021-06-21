/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <thread>
#include "fmp4.h"

#include "base/logger.h"
#include "base/define.h"
#include "base/test.h"
//#include "net/netInterface.h"
//#include "http/websocket.h"
//#include "http/HttpsClient.h"


#define MAX_CHUNK_SIZE 2048
// maximum send buffer 262144  =1024 *256
#define highWaterMark  8 * 1048576
//maximum buffer = 16 *1048576 where  1024*1024 =1048576


#define IOBUFSIZE 40960
//40960*6


#define SERVER_HOST  "127.0.0.1"               
#define SERVER_PORT 8443 //443

using namespace base::net;

namespace base {
    namespace fmp4 {

        ReadMp4::ReadMp4() {
            
            
         //ffparser = new FFParse(nullptr);
        // ffparser->start(); 
            

        }

        ReadMp4::~ReadMp4() {
            SInfo << "~ReadMp4( )";
            ffparser->stop();
            ffparser->join();
            delete ffparser;
        }

        void ReadMp4::websocketConnect() {


            conn = new net::HttpsClient("wss", SERVER_HOST, SERVER_PORT, "/");


            //            {
            //                m_client = new HttpClient("ws", _host, _port, url.str());
            //            }
            //            else
            //            {
            //                 m_client = new HttpsClient("wss", _host, _port, url.str());
            //            }

            // conn->Complete += sdelegate(&context, &CallbackContext::onClientConnectionComplete);
            conn->fnComplete = [&](const Response & response) {
                std::string reason = response.getReason();
                StatusCode statuscode = response.getStatus();
                std::string body = conn->readStream() ? conn->readStream()->str() : "";
                STrace << "SocketIO handshake response:" << "Reason: " << reason << " Response: " << body;
            };

            conn->fnPayload = [&](HttpBase * con, const char* data, size_t sz) {
                std::string got = std::string(data, sz);
                STrace << "client->fnPayload " << got;
                
                //  m_ping_timeout_timer.Reset();
                //  m_packet_mgr.put_payload(std::string(data,sz));
                if( got == "reset")
                    ffparser->reset();    
            };


            conn->fnConnect = [&](HttpBase * con) {
                STrace << "onconnect ";
                
                //this->start();
                ffparser = new FFParse(conn);
                ffparser->start();    
                // conn->send(x, 2, true);
            };


            conn->fnClose = [&](HttpBase * con, std::string str) {
                STrace << "client->fnClose " << str;

                //on_close();
            };

            //  conn->_request.setKeepAlive(false);
            conn->setReadStream(new std::stringstream);
            conn->send();
            LTrace("sendHandshakeRequest over")
            

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

                    conn->send((const char*) buf, nRead, true);


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

    }
}
