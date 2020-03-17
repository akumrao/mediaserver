

using std::endl;
using namespace base;
using namespace net;


   
 namespace hm {


 //const std::string ip = "18.228.58.178";
 const std::string ip = "127.0.0.1";
 
  
 const int port = 47001;
     
 
awsTcpClient *thread;

void init( )
{
     thread = new awsTcpClient(ip, port);
}


void upload(  const std::string driverId, const std::string metaDataJson, const std::string file)
{
    thread->upload( file, driverId, metaDataJson);
    thread->start();
    
     thread->fnUpdateProgess = [&](const std::string str, int progess) {
            
         SInfo << "Percentage uploaded " <<progess;
         
         
        };
    
}

void  stop( )
{

    thread->stop();
    
}



void  exit( )
{
    delete thread;
}

}// end hm

 

         
int main(int argc, char** argv) {
    Logger::instance().add(new ConsoleChannel("debug", Level::Trace));

    hm::init();
    
    std::string file= "./test.mp4"; //complete path
    std::string metadata = "{filename:driver-1234-1232323.mp4, gps-latitude:28.674109, gps-longitude:77.438009, timestamp:20200309194530, uploadmode:normal}";
   
    hm::upload( "driver-1234", metadata, file);
    
    
    
    base::sleep(444444444);

    hm::exit();

    return 0;
}
