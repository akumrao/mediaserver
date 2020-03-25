#include "hmTcpClient.h"
#include "hmUdpClient.h"
#include "base/logger.h"
#include "base/application.h"
#include "base/platform.h"
#include <math.h>       /* ceil */


////////////////////
#include <stdio.h>
#include <sys/stat.h>

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
//////////////////

//using std::endl;
using namespace base;
using namespace net;

hmUdpClient::hmUdpClient(std::string IP, int port, hmTcpClient *tcpObc) : IP(IP), port(port), tcpClient(tcpObc),restartPacketNo(0), uploadedPacketNO(0) {

//    for (int x = 0; x < clientCount; ++x) {
//        clinetstorage[x] = new char[UdpDataSize];
//    }


    udpClient = new UdpSocket(IP, port);
    storage = nullptr;

    size_of_packet = sizeof (struct Packet);
    send_buffer = new char[size_of_packet];
    
    //sendheader = true;
   // sendfile= true;

    //uv_sem_init(&sem, 0);
    sem_init( &sem, 0, 0);
    
    rem=0;

}

hmUdpClient::~hmUdpClient() {

//    join();
    
    delete []send_buffer;
     
    if(fd  > 0 )
    {
      munmap(storage, size);
      close(fd);
    }  
    
    delete udpClient;
    udpClient = nullptr;

    uv_sem_destroy(&sem);


    LTrace("~hmUdpClient()" )
}

void hmUdpClient::restartUPload(uint32_t uploaded)
{
   // udp_client_mutex.lock();
 //   restUpload = true;

    udp_client_mutex.lock();
    restartPacketNo = uploaded;
    udp_client_mutex.unlock();

    uv_sem_post(&sem);

    //udp_client_mutex.unlock();

}

void hmUdpClient::run() {

    LTrace("run start")
    SInfo << "Send File start";

    while( !stopped()  && uploadedPacketNO < lastPacketNo  ) {
        //
        //

        if (!rem) {
            udpClient->connect();
            sendHeader(m_fileName);
        }

        if (rem < lastPacketNo)
            sendFile();


        ++rem;


        udp_client_mutex.lock();
        if (restartPacketNo) {
            rem = restartPacketNo;
            restartPacketNo = 0;
            STrace << "restartPacketNo Frame " << rem << " Uploaded " << uploadedPacketNO << " Lastpacketno " <<  lastPacketNo ;

        }
        udp_client_mutex.unlock();

       // STrace << "Read Packet Frame " << rem;

        if( ( (uploadedPacketNO < lastPacketNo) && (rem >  lastPacketNo)  ))
        {
          STrace << "Read Packet Frame " << rem << " Uploaded " << uploadedPacketNO << " Lastpacketno " <<  lastPacketNo ;
          sem_wait(&sem); /* should block */
           //udpClient->connect();
        } else {

            clock_gettime(CLOCK_REALTIME, &tm);
            tm.tv_nsec += 4200;
            sem_timedwait(&sem, &tm);
        }



    }

    LTrace("Upload over")
}

//void hmUdpClient::send(char* data, unsigned int lent) {
//    std::cout << "sending data " << lent << std::endl;
//    udpClient->send(data, lent);
//}

void hmUdpClient::shutdown() {
    
    LInfo("hmUdpClient::shutdown()::stop");
    
    stop();
    restartUPload(lastPacketNo);
    join();
    
    if(udpClient)
    {

        udpClient->Close();
        
      //  base::sleep(500);
        
     
         LInfo("hmUdpClient::shutdown()::udpClient");

    }
    
    LInfo("hmUdpClient::shutdown()::over");
}

bool hmUdpClient::upload( std::string fileName, std::string driverId, std::string metaData)
{
    struct stat st;
    fd = open(fileName.c_str(), O_RDONLY,1);

    int rc = fstat(fd, &st);

    size=st.st_size;
    m_fileName = fileName;
    m_driverId  = driverId;
    m_metaData = metaData;

    if(fd > 0)
    {
        lastPacketNo = ceil((float)size / (float) (UdpDataSize));
        SInfo << "fileSize: "  <<  size ;
        SInfo << "Last packet no: "  <<  lastPacketNo ;

        storage = (char *)mmap(0, size, PROT_READ ,MAP_SHARED , fd, 0);
        return true;
    }
    else {
        SError << "Cannot open file: " << fileName ;
        return false;
    }
}

void hmUdpClient::sendPacket(uint8_t type, uint32_t payloadNo, uint32_t payloadsize, char *payload) {

  
    
    Packet packet;
    packet.type = type;
    packet.payload_number = payloadNo;
    packet.payloadlen = payloadsize;

            
    memcpy(packet.payload, payload, payloadsize);
    memset(send_buffer, 0, size_of_packet);
    memcpy(send_buffer, (char*) &packet, size_of_packet);

    udpClient->send(send_buffer, size_of_packet);

}

char *hmUdpClient::storage_row(unsigned int n) {
    return storage + (n * UdpDataSize);
}




void hmUdpClient::sendHeader(const std::string fileName) {

    SInfo << "Send Header";
            
    if (fd> 0 ) {

         if (!stopped())
         {

            std::string mtTmp = m_driverId  +";" + m_metaData;
            sendPacket(0, lastPacketNo, mtTmp.length()+1, (char*)mtTmp.c_str());
         }
    

    }
}

void hmUdpClient::sendFile() {


    if (rem  < lastPacketNo-1) {
         // char *output = str2md5(data_packet.data, data_size);
        //char *output1 = str2md5(buffer[send_count], data_size);
        sendPacket(1, rem, UdpDataSize , storage_row(rem));
        //usleep(400); //2900
    }

    else if( rem  < lastPacketNo) {
        uint32_t lastPacketLen = size - rem*UdpDataSize;
        sendPacket(1, rem, lastPacketLen, storage_row(rem));
    }
}

   // end_time = base::Application::GetTime();

   // STrace << "time_s " << double(end_time - start_time) / 1000.00 ;


#if 0

    int main(int argc, char** argv) {
        Logger::instance().add(new ConsoleChannel("debug", Level::Trace));


        LTrace("./runhmUdpClient test.mp4 127.0.0.1")
        LTrace("./runhmUdpClient test.mp4")
        Application app;

        int port = 51038;
        std::string ip = "18.228.58.178";
        std::string filename;
        if (argc > 1) {
            filename = argv[1];
        }

        if (argc > 2) {
            ip = argv[2];
        }

        if (argc > 3) {
            port = atoi(argv[3]);
        }





        start_time = base::Application::GetTime();

        std::ifstream infile;
        infile.open(filename, std::ios::binary | std::ios::in);

        Packet init_packet;

        int size_of_packet = sizeof (struct Packet);

        init_packet.type = 0;
        init_packet.sequence_number = -1;
        init_packet.len = UdpDataSize;

        strncpy(init_packet.data, filename.c_str(), UdpDataSize);

        char *send_buffer = (char*) malloc(size_of_packet);

        memset(send_buffer, 0, size_of_packet);
        memcpy(send_buffer, (char*) &init_packet, size_of_packet);

        socket.send(send_buffer, size_of_packet);

        free(send_buffer);



        if (infile.is_open()) {

            int bcst = 0;
            int rem = 0;
            Packet packet;
            send_buffer = (char*) malloc(sizeof (struct Packet));


            while (infile.read(clinetstorage[rem], UdpDataSize)) {
                packet.type = 1;
                packet.sequence_number = bcst;
                packet.len = UdpDataSize;
                memcpy(packet.data, clinetstorage[rem], UdpDataSize);

                memset(send_buffer, 0, size_of_packet);
                memcpy(send_buffer, (char*) &packet, size_of_packet);
                // char *output = str2md5(data_packet.data, data_size);
                //char *output1 = str2md5(buffer[send_count], data_size);

                socket.send(send_buffer, size_of_packet);
                rem = (++bcst) % clientCount;

            }

            packet.type = 2;
            packet.sequence_number = bcst;
            packet.len = infile.gcount();
            memcpy(packet.data, clinetstorage[rem], infile.gcount());

            memset(send_buffer, 0, size_of_packet);
            memcpy(send_buffer, (char*) &packet, size_of_packet);
            socket.send(send_buffer, size_of_packet);

            infile.close();
        } else {
            std::cerr << "Cannot open file:" << filename << endl;
        }


        for (int x = 0; x < clientCount; ++x) {
            delete [] clinetstorage[x];
        }

        end_time = base::Application::GetTime();

        std::cout << "time_s " << double(end_time - start_time) / 1000.00 << std::endl << std::flush;


        app.run();



        return 0;
    }
#endif