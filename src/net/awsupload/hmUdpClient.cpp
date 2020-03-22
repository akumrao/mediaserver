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

hmUdpClient::hmUdpClient(std::string IP, int port, hmTcpClient *tcpObc) : IP(IP), port(port), tcpClient(tcpObc) {

//    for (int x = 0; x < clientCount; ++x) {
//        clinetstorage[x] = new char[UdpDataSize];
//    }

    udpClient = nullptr;
    storage = nullptr;

    size_of_packet = sizeof (struct Packet);
    send_buffer = new char[size_of_packet];
}

hmUdpClient::~hmUdpClient() {

    join();

    LTrace("~hmUdpClient()" )
}

void hmUdpClient::run() {
    
    LTrace("start UDP client")
    udpClient = new UdpSocket(IP, port);
    udpClient->connect();
    
     sendFile(m_fileName);

}

//void hmUdpClient::send(char* data, unsigned int lent) {
//    std::cout << "sending data " << lent << std::endl;
//    udpClient->send(data, lent);
//}

void hmUdpClient::shutdown() {
    
   // stop();


//    for (int x = 0; x < clientCount; ++x) {
//        delete [] clinetstorage[x];
//    }

    if(udpClient)
    {
        udpClient->Close();

        join();

        delete []send_buffer;
        delete udpClient;


        munmap(storage, size);

        close(fd);
        udpClient = nullptr;

    }

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
        return true;
    else
        return false;
}

void hmUdpClient::sendPacket(uint8_t type, uint16_t payloadNo, uint16_t payloadsize, char *payload) {

    if(!( payloadNo % 30))
    STrace << "UpdClient Send Pakcet: Type " << (int) type  << " payload no " <<  payloadNo << " payloadsize " << payloadsize;
    
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


void hmUdpClient::sendFile(const std::string fileName) {


//    {
//        std::ifstream infile;
//        infile.open(fileName, std::ios::binary | std::ios::in);
//
//        if (infile.is_open()) {
//
//            infile.seekg(0, infile.end);
//            float length = infile.tellg();
//            infile.seekg(0, infile.beg);
//
//            lastPacketNo = ceil(length / (UdpDataSize));
//
//            LError(length);
//
//            LError(lastPacketNo);
//
//            std::string mtTmp = m_driverId + ";" + m_metaData;
//            //sendPacket(0, lastPacketNo, mtTmp.length() + 1, (char *) mtTmp.c_str());
//
//            int bcst = 0;
//            int rem = 0;
//
//            while (!stopped() && infile.read(clinetstorage[rem], UdpDataSize)) {
//                // char *output = str2md5(data_packet.data, data_size);
//                //char *output1 = str2md5(buffer[send_count], data_size);
//                //sendPacket(1, bcst, UdpDataSize, clinetstorage[rem]);
//
//                printf("value1 %c", clinetstorage[rem][100]);
//                printf("value2 %c", clinetstorage[rem][101]);
//
//                rem = (++bcst) % clientCount;
//
//                //base::sleep(5);
//               // break;
//            }
//
//            lastPacketLen = infile.gcount();
//
//
//            LError(lastPacketLen);
//
//
//            sendPacket(1, bcst, infile.gcount(), clinetstorage[rem]);
//            infile.close();
//        }
//
//    }



    // start_time = base::Application::GetTime();

    if (fd> 0 ) {



        lastPacketNo = ceil((float)size / (float) (UdpDataSize));

        LError(size);

        LError(lastPacketNo);

        
        std::string mtTmp = m_driverId  +";" + m_metaData;
        sendPacket(0, lastPacketNo, mtTmp.length()+1, (char*)mtTmp.c_str());

        //int bcst = 0;
        int rem = 0;
;

        storage = (char *)mmap(0, size, PROT_READ ,MAP_SHARED , fd, 0);


        while (!stopped() && rem  < lastPacketNo-1) {

            char *pppp = storage_row(rem);

            // char *output = str2md5(data_packet.data, data_size);
            //char *output1 = str2md5(buffer[send_count], data_size);
            sendPacket(1, rem, UdpDataSize , storage_row(rem));
            ++rem;
            base::sleep(5);


        }

        if (!stopped() && rem  < lastPacketNo) {

            lastPacketLen = size - rem*UdpDataSize;
            sendPacket(1, rem, lastPacketLen, storage_row(rem));
        }

    } else {
        SError << "Cannot open file: " << fileName ;


    }

   // end_time = base::Application::GetTime();

   // STrace << "time_s " << double(end_time - start_time) / 1000.00 ;

}

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