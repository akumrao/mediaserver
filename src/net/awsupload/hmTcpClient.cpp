#include "base/base.h"
#include "base/logger.h"
#include "base/application.h"

#include "hmUdpClient.h"
#include "base/time.h"
#include "tcpUpload.h"
#include "base/platform.h"
#include "hmTcpClient.h"

using std::endl;
using namespace base;
using namespace net;

static void async_cb_upload(uv_async_t* handle) {

    LTrace(" Upload::async_cb_upload")

    hmTcpClient *p = ( hmTcpClient *) handle->data;
    uv_close((uv_handle_t*)&p->async, nullptr);

    p->Close();
    p->stop();
    p->app.stop();

   
    p->app.uvDestroy();

    LTrace(" Upload::async_cb_upload over")

}


 hmTcpClient::~hmTcpClient() {
    LTrace(" Upload::~hmTcpClient ")
}
void hmTcpClient::run() {

  
    Connect(m_IP, m_port);
    
    async.data = this;
    int r = uv_async_init(app.uvGetLoop(), &async, async_cb_upload);
    assert(r == 0);
    
    
    app.run();
    SInfo << "run over";
}

void hmTcpClient::upload(std::string fileName, std::string driverId, std::string metaData) {
    m_fileName = fileName;
    m_driverId = driverId;
    m_metaData = metaData;
}

void hmTcpClient::sendPacket(uint8_t type, uint16_t payload) {

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
}

void hmTcpClient::shutdown() {
    if(udpsocket)
    {
        udpsocket->shutdown();
        delete udpsocket;
    }
    int  r = uv_async_send(&async);
    assert(r == 0);
    
    join();
      
    STrace << "shutdown ";
}

void hmTcpClient::on_close(Listener* connection) {
    std::cout << " Close Con LocalIP" << connection->GetLocalIp() << " PeerIP" << connection->GetPeerIp() << std::endl << std::flush;
}

void hmTcpClient::on_read(Listener* connection, const char* data, size_t len) {
    STrace << "TCP on_read " << "len: " << len;
    // connection->send((const char*) send.c_str(), 5);

    if (len != sizeof (struct TcpPacket)) {
        LTrace(data)
        LError("Fatal error: Some part of packet lost. ")
        return;
    }

    TcpPacket packet;
    memcpy(&packet, (void*) data, len);

    switch (packet.type) {
        case 1:
        {
            //LTrace("First TCP Packet received. ")

            SInfo << "UDP Client connect at: " <<  packet.sequence_number;  ;
            
            udpsocket = new hmUdpClient(m_IP, packet.sequence_number, this);
            udpsocket->upload(m_fileName, m_driverId, m_metaData);
            udpsocket->start();

            break;
        }
        case 2:
        {
            SInfo << "TCP Received type " << (int) packet.type << " Retransmission: " << packet.sequence_number;


            uint16_t payloadsize = UdpDataSize;

            if (packet.sequence_number == udpsocket->lastPacketNo) {
                payloadsize = udpsocket->lastPacketLen;
                SInfo << "Retransmission of lastpacket: " << packet.sequence_number << " size " << payloadsize;
            }


            int rem = packet.sequence_number % clientCount;
            udpsocket->sendPacket(1, packet.sequence_number, payloadsize, udpsocket->clinetstorage[rem]);
            break;
        }

        case 3:
        {
            STrace << "TCP Received type " << packet.type << " payload:" << packet.sequence_number;


            if (fnUpdateProgess)
                fnUpdateProgess(m_fileName, packet.sequence_number);
            
            if(   packet.sequence_number == 100 )
            {

                if (fnSuccess)
                    fnSuccess(m_fileName, "Upload Completed");

                shutdown();

            }

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

    void upload(const std::string driverId, const std::string metaDataJson, const std::string file) {
        thread->upload(file, driverId, metaDataJson);
        thread->start();

        thread->fnUpdateProgess = [&](const std::string str, int progess) {

            SInfo << "Percentage uploaded " << progess;

        };

    }

    void stop() {

        thread->stop();

    }

    void exit() {
        thread->shutdown();
        delete thread;
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



    base::sleep(5000000);

    LTrace("About to exit")
    hm::exit();

    
    base::sleep(100000);
        
    return 0;
}

#endif