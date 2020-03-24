#include "base/base.h"
#include "base/logger.h"
#include "base/application.h"

#include "hmUdpClient.h"
#include "base/time.h"
#include "tcpUpload.h"
#include "base/platform.h"
#include "hmTcpClient.h"
#include <future>

#define UPLOADTIMEOUT 10000

using std::endl;
using namespace base;
using namespace net;

static void async_cb_upload(uv_async_t* handle) {

    LTrace(" Upload::async_cb_upload")



    hmTcpClient *p = ( hmTcpClient *) handle->data;

    uv_close((uv_handle_t*)&p->async, nullptr);

    p->m_ping_timeout_timer->Stop();
    p->m_ping_timeout_timer->Close();

    delete p->m_ping_timeout_timer;


    p->Close();
    p->stop();
  

    SInfo << "Upload::async_cb_upload over" ;

}


 hmTcpClient::~hmTcpClient() {
     
//   join();  
    if(udpsocket)
    {
        delete udpsocket;
        udpsocket = nullptr;
    }
   
    SInfo << " Upload::async_cb_upload over";
}
 
void hmTcpClient::run() {

    SInfo << "run";
  
    Connect(m_IP, m_port);
    
    async.data = this;
    int r = uv_async_init(app.uvGetLoop(), &async, async_cb_upload);
    assert(r == 0);

    m_ping_timeout_timer = new Timer(nullptr);
    m_ping_timeout_timer->cb_timeout = std::bind(&hmTcpClient::timeout_pong, this);
    
    app.run();
    SInfo << "run over";
    
     
}

void hmTcpClient::timeout_pong()
{
    STrace << "TCP Received type " <<  "time Expired";
    m_ping_timeout_timer->Reset();
    udpsocket->restartUPload( udpsocket->uploadedPacketNO +1  );
}

void hmTcpClient::upload(std::string fileName, std::string driverId, std::string metaData) {
    m_fileName = fileName;
    m_driverId = driverId;
    m_metaData = metaData;
}

void hmTcpClient::sendPacket(uint8_t type, uint32_t payload) {

    STrace << "Send Type" << (int) type << " payload " << payload;

    TcpPacket tcpPacket;
    int size_of_packet = sizeof (struct TcpPacket);
    tcpPacket.type = type;
    tcpPacket.sequence_number = payload;

    char *send_buffer = (char*) malloc(size_of_packet);
    memset(send_buffer, 0, size_of_packet);
    memcpy(send_buffer, (char*) &tcpPacket, size_of_packet);
    send(send_buffer, size_of_packet);
    free(send_buffer);
}

void hmTcpClient::on_connect() {
    STrace << "on_connect Send Init: ";
    sendPacket(0, 0);
    en_state = Connected;
}

void hmTcpClient::shutdown() {
    
    std::lock_guard<std::mutex> guard(g_shutdown_mutex);
    
    if(!shuttingDown )
    {
        shuttingDown =true;

        if(udpsocket)
        {
            udpsocket->shutdown();
        }
        int  r = uv_async_send(&async);
        assert(r == 0);

//       join();
       STrace << "shutdown over";
    }

     
}

void hmTcpClient::on_close() {
    //std::cout << " Close Con LocalIP" << connection->GetLocalIp() << " PeerIP" << connection->GetPeerIp() << std::endl << std::flush;
    if( en_state < Progess) {

        if (fnFailure)
            fnFailure(m_fileName, "Network Issue or Media Service not running", -1);
    }
    
    app.stop();
    app.uvDestroy();
    

    SInfo << "hmTcpClient::on_close";
}





void hmTcpClient::on_read(Listener* connection, const char* data, size_t len) {
   // STrace << "TCP on_read " << "len: " << len;
    // connection->send((const char*) send.c_str(), 5);

    if( shuttingDown )
    {
        return;
    }
    
    if (len != sizeof (struct TcpPacket)) {
       // LTrace(data)
        LError("Fatal error: Some part of packet lost. ")
        return;
    }

    TcpPacket packet;
    memcpy(&packet, (void*) data, len);

    switch (packet.type) {
        case 1:
        {
            
            udpsocket = new hmUdpClient(m_IP, packet.sequence_number, this);
            if(!udpsocket->upload(m_fileName, m_driverId, m_metaData))
            {
               // udpsocket->shutdown();
                delete udpsocket;
                udpsocket = nullptr;

                if (fnFailure  ) {
                    fnFailure(m_fileName, "Cannot open file", -2);
                    fnFailure = nullptr;
                    en_state = Done;
                }

                shutdown();
                return;
            }
            SInfo << "UDP Client connect at: " <<  packet.sequence_number; 
            udpsocket->start();
            
            m_ping_timeout_timer->Start(UPLOADTIMEOUT, UPLOADTIMEOUT);

            if (fnUpdateProgess)
                fnUpdateProgess(m_fileName, 0);

            break;
        }
        case 2:
        {

            SInfo << "TCP Received type " << (int) packet.type << " Retransmission: " << packet.sequence_number;

//            udpsocket->stop();
//            udpsocket->join();
//            udpsocket->stop(false);
//            udpsocket->rem = packet.sequence_number;
//            udpsocket->start();
              udpsocket->restartUPload(packet.sequence_number );

            break;
        }

        case 3:
        {
            en_state = Progess;
            STrace << "TCP Received type " << (int) packet.type << " percentage uploaded:" << packet.sequence_number;

            udpsocket->uploadedPacketNO = packet.sequence_number;
            
            if (fnUpdateProgess)
            {
                int per = 10 * (packet.sequence_number / ((udpsocket->lastPacketNo) / 10));
                if (per != 100) {
                    
                }
                fnUpdateProgess(m_fileName, per);
            }
           
            if(   packet.sequence_number == udpsocket->lastPacketNo -1 )
            {
                m_ping_timeout_timer->Stop();
                en_state = Progess;
                if (fnSuccess)
                    fnSuccess(m_fileName, "Upload Completed");

                shutdown();
            }else
            {
                 m_ping_timeout_timer->Reset();
            }

            break;
        }
        
        case 4:
        {

            SInfo << "TCP Received type " << (int) packet.type << " Retransmission of header: " << packet.sequence_number;
//            udpsocket->stop();
//            udpsocket->join();
//            udpsocket->stop(false);
//            udpsocket->rem = packet.sequence_number;
//            udpsocket->start();
              udpsocket->restartUPload(packet.sequence_number );
            break;
        }

        default:
        {
            LError("Fatal TCP: Not a valid state")

        }
    };

}

#if _Main_

namespace hm {

    std::string ip = "18.228.58.178";
   // std::string ip = "3.21.171.200";
    
   // std::string ip = "127.0.0.1";

    const int port = 47001;


    hmTcpClient *thread;

    void init() {

        thread = new hmTcpClient(ip, port);
    }

    void exitit() {
        SInfo << "hm::exit() ";
        thread->shutdown();
        thread->join();
        delete thread;
    }
       
    void upload(const std::string driverId, const std::string metaDataJson, const std::string file) {
        thread->upload(file, driverId, metaDataJson);
        thread->start();

        thread->fnUpdateProgess = [&](const std::string str, int progess) {

            SInfo << "Percentage uploaded " << progess;

        };
        
         thread->fnSuccess = [&](const std::string& , const std::string&) {

            SInfo << "success ";
            std::thread th(&exitit) ;
            th.detach();
        };
        
        thread->fnFailure = [&](const std::string& , const std::string&, const int&) {

            SInfo << "failure " ;
            
            std::thread th(&exitit) ;
            th.detach();
           
                 
           // a1();
        };
        
       

    }

    void stop() {

        thread->stop();

    }

 

}// end hm

int main(int argc, char** argv) {
    Logger::instance().add(new ConsoleChannel("debug", Level::Trace));

    LTrace("./runhmUdpClient test.mp4 127.0.0.1")
    LTrace("./runhmUdpClient test.mp4")
    Application app;


    std::string filename = "./test.mp4"; //complete path
    if (argc > 1) {
        filename = argv[1];
    }

    if (argc > 2) {
        hm::ip = argv[2];
    }

    hm::init();
        
    std::string file = filename;
    std::string metadata = "{filename:driver-1234-1232323.mp4, gps-latitude:28.674109, gps-longitude:77.438009, timestamp:20200309194530, uploadmode:normal}";

    hm::upload("driver-1234", metadata, file);



    base::sleep(20000000);

    LTrace("About to exit")
    std::thread th(&hm::exitit) ;
            th.detach();

    
    base::sleep(100000);
        
    return 0;
}

#endif