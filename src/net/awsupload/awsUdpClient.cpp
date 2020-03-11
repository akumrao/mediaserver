
#include "awsUdpClient.h"
#include "base/logger.h"
#include "base/application.h"
#include "base/platform.h"
#include <math.h>       /* ceil */


//using std::endl;
using namespace base;
using namespace net;

awsUdpClient::awsUdpClient(std::string IP, int port) : IP(IP), port(port) {

    for (int x = 0; x < clientCount; ++x) {
        clinetstorage[x] = new char[UdpDataSize];
    }

    size_of_packet = sizeof (struct Packet);
    send_buffer = new char[size_of_packet];
}

awsUdpClient::~awsUdpClient() {


    for (int x = 0; x < clientCount; ++x) {
        delete [] clinetstorage[x];
    }

    delete []send_buffer;
    delete udpClient;
}

void awsUdpClient::start() {
    LTrace("start UDP client")
    udpClient = new UdpSocket(IP, port);
    udpClient->connect();

}

//void awsUdpClient::send(char* data, unsigned int lent) {
//    std::cout << "sending data " << lent << std::endl;
//    udpClient->send(data, lent);
//}

void awsUdpClient::shutdown() {
    delete udpClient;
    udpClient = nullptr;

}

void awsUdpClient::sendPacket(uint8_t type, uint16_t payloadNo, uint16_t payloadsize, char *payload) {

  
    
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

void awsUdpClient::sendFile(const std::string fileName) {

    int64_t start_time;
    int64_t end_time;

    start_time = base::Application::GetTime();
    std::ifstream infile;
    infile.open(fileName, std::ios::binary | std::ios::in);

    if (infile.is_open()) {

        infile.seekg(0, infile.end);
        float length = infile.tellg();
        infile.seekg(0, infile.beg);

        lastPacketNo = ceil(length / UdpDataSize);
        sendPacket(0, lastPacketNo, fileName.length()+1, (char*)fileName.c_str());

        int bcst = 0;
        int rem = 0;

        while (infile.read(clinetstorage[rem], UdpDataSize)) {
            // char *output = str2md5(data_packet.data, data_size);
            //char *output1 = str2md5(buffer[send_count], data_size);
            sendPacket(1, bcst,UdpDataSize , clinetstorage[rem]);
            rem = (++bcst) % clientCount;
            
            base::sleep(1000);
        }
        
        lastPacketLen = infile.gcount();
        sendPacket(1, bcst, infile.gcount(), clinetstorage[rem]);
        infile.close();
    } else {
        SError << "Cannot open file: " << fileName ;
    }

    end_time = base::Application::GetTime();

    STrace << "time_s " << double(end_time - start_time) / 1000.00 ;

}

#if 0

    int main(int argc, char** argv) {
        Logger::instance().add(new ConsoleChannel("debug", Level::Trace));


        LTrace("./runawsUdpClient test.mp4 127.0.0.1")
        LTrace("./runawsUdpClient test.mp4")
        Application app;

        int port = 51038;
        std::string ip = "52.14.171.173";
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