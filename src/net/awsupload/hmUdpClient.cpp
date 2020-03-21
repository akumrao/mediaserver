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
//
//    for (int x = 0; x < clientCount; ++x) {
//        clinetstorage[x] = new char[UdpDataSize];
//    }

    clinetstorage = nullptr;

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
    
    stop();


//    for (int x = 0; x < clientCount; ++x) {
//        delete [] clinetstorage[x];
//    }

    if(udpClient)
    {
        udpClient->Close();

        delete []send_buffer;
        delete udpClient;


        munmap(clinetstorage, size);

        close(fd);
        udpClient = nullptr;

    }

}

void hmUdpClient::sendPacket(uint8_t type, uint16_t payloadNo, uint16_t payloadsize, char *payload) {

    if(!( payloadNo % 10))
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
    return clinetstorage + (n * UdpDataSize);
}


void hmUdpClient::sendFile(const std::string fileName) {

   // int64_t start_time;
   // int64_t end_time;

    struct stat st;
    fd = open(fileName.c_str(), O_RDONLY);

    int rc = fstat(fd, &st);


    size=st.st_size;




    // start_time = base::Application::GetTime();
   // std::ifstream infile;
   // infile.open(fileName, std::ios::binary | std::ios::in);


    if (fd> 0 ) {



        lastPacketNo = ceil(size / (UdpDataSize));
        
        std::string mtTmp = m_driverId  +";" + m_metaData;
        sendPacket(0, lastPacketNo, mtTmp.length()+1, (char*)mtTmp.c_str());

        int bcst = 0;
        int rem = 0;

//        float *inputTensor2Ptr = reinterpret_cast<float *>(mmap(nullptr, tensorSize_ * sizeof(float),
//                                                                PROT_READ | PROT_WRITE, MAP_SHARED,
//                                                                inputTensor2Fd_, 0));

        clinetstorage = (char *)mmap(0, size, PROT_READ ,MAP_SHARED , fd, 0);


        while (!stopped() && rem  < lastPacketNo) {
            // char *output = str2md5(data_packet.data, data_size);
            //char *output1 = str2md5(buffer[send_count], data_size);
            sendPacket(1, rem, UdpDataSize , storage_row(rem));
            ++rem;
            base::sleep(5);
        }

        if (!stopped() && rem  <= lastPacketNo) {

            int left = size - rem*UdpDataSize;

            sendPacket(1, bcst, left, storage_row(rem));
        }

    } else {
        SError << "Cannot open file: " << fileName ;

        if (tcpClient->fnFailure  )
            tcpClient->fnFailure(fileName, "Cannot open file" , -2 );
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