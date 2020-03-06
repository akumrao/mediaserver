#include "base/base.h"
#include "base/logger.h"
#include "base/application.h"
#include "net/UdpSocket.h"
#include "base/test.h"
#include "base/time.h"


using std::endl;
using namespace base;
using namespace net;
using namespace base::test;

class testUdpClient {
public:

    testUdpClient(std::string IP, int port) : IP(IP), port(port) {
    }

    void start() {
        LTrace("start UDP client")
        udpClient = new UdpSocket(IP, port);
        udpClient->connect();

    }

    void send(char* data, unsigned int lent) {
        std::cout << "sending data " << lent << std::endl;
        udpClient->send(data, lent);
    }

    void shutdown() {
        delete udpClient;
        udpClient = nullptr;

    }


    UdpSocket *udpClient;

    std::string IP;
    int port;
    


};




int main(int argc, char** argv) {
    Logger::instance().add(new ConsoleChannel("debug", Level::Trace));

    int64_t start_time;
    int64_t end_time;
    
    LTrace("./runtestUdpClient test.mp4 127.0.0.1")
    LTrace("./runtestUdpClient test.mp4")
    Application app;

    int port = 51038;
    std::string ip = "18.221.232.217";
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


    for( int x=0; x < StorageCount; ++x )
    {
        backstorage[x] = new char[UdpDataSize];
    }
    
    testUdpClient socket(ip, port);
    socket.start();

    start_time = base::Application::GetTime();
   
    std::ifstream infile;
    infile.open(filename, std::ios::binary | std::ios::in);
    
    Packet init_packet;
    
    int size_of_packet = sizeof(struct Packet);

    init_packet.type =0;
     init_packet.sequence_number =-1;
    init_packet.len = 256 ;
    
    strncpy( init_packet.data, filename.c_str(), 256);
    
    char *send_buffer = (char*)malloc(size_of_packet);
    
    memset(send_buffer,0,size_of_packet);
    memcpy(send_buffer,(char*)&init_packet,size_of_packet);
    
    socket.send(send_buffer, size_of_packet);
    
    free(send_buffer);



    if (infile.is_open()) {

        int bcst =0;
        int rem =0;
        Packet packet;
        send_buffer = (char*)malloc(sizeof(struct Packet));
        
        
        while (infile.read(backstorage[rem], UdpDataSize)) {
            packet.type =1;
            packet.sequence_number = bcst;
            packet.len = UdpDataSize;
            memcpy( packet.data, backstorage[rem], UdpDataSize);
            
            memset(send_buffer,0,size_of_packet);
            memcpy(send_buffer,(char*)&packet,size_of_packet);
           // char *output = str2md5(data_packet.data, data_size);
            //char *output1 = str2md5(buffer[send_count], data_size);

            socket.send(send_buffer, size_of_packet);
            rem = (++bcst)%StorageCount;
           
        }
        
        packet.type =2;
        packet.sequence_number = bcst;
        packet.len = infile.gcount();
        memcpy( packet.data, backstorage[rem], infile.gcount());
            
        memset(send_buffer,0,size_of_packet);
        memcpy(send_buffer,(char*)&packet,size_of_packet);
        socket.send(send_buffer, size_of_packet);

        infile.close();
    } else {
        std::cerr << "Cannot open file:" << filename << endl;
    }
    

    for( int x=0; x < StorageCount; ++x )
    {
        delete [] backstorage[x];
    }

    end_time = base::Application::GetTime();    
    
    std::cout << "time_s " << double(end_time - start_time) / 1000.00 << std::endl << std::flush;
        

    app.run();



    return 0;
}
