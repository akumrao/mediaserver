#include "ff/ff.h"
#include "ff/mediacapture.h"
#include "base/define.h"
#include "base/test.h"
#include "base/filesystem.h"
#include "base/application.h"
#include "base/packetstream.h"
#include "base/packet.h"
#include "base/packetqueue.h"
#include "base/platform.h"

using namespace std;
using namespace base;
using namespace base::ff;
using namespace base::test;


// =============================================================================
// Packet Stream
//
struct MockThreadedPacketProcessor : public PacketProcessor, public Thread
{

    MockThreadedPacketProcessor()
    {
        LTrace("MockThreadedPacketProcessor")
    }
    
     ~MockThreadedPacketProcessor()
    {
         LTrace("~MockThreadedPacketProcessor")
    }

    void run()
    {
    
        for( int i=0; i < 4 ; ++i )
        {
            std::cout << "Emitting" << std::endl;
            std::ostringstream str;
            str << "hell" << i ;
            RawPacket p( str.str().c_str(), 6);
            emit(/*self, */ p);
        }
    }
    void start()
    {
        std::cout << "Start" << std::endl;
        Thread::start();
    }

    void stop()
    {
        std::cout << "Stop" << std::endl;
        // runner.close();
        Thread::stop();
        // std::cout << "Stop: OK" << std::endl;
    }
};

struct MockPacketProcessor : public PacketProcessor
{
    MockPacketProcessor()
    {
          LTrace("MockPacketProcessor")
    }

    ~MockPacketProcessor()
    {
          LTrace("~MockPacketProcessor")
    }
    
    void process(IPacket& packet)
    {
        std::cout << "Process: " << packet.className() << " Data: " << packet.data() << std::endl;
        emit(packet);
    }
};

class PacketStreamTest : public Test
{
public:
    int numPackets;

       void run()
    {
        numPackets = 0;
        // stream.setRunner(std::make_shared<Thread>());
        PacketStream stream;
        // stream.attach(new AsyncPacketQueue, 0, true);
        stream.attachSource(new MockThreadedPacketProcessor,  true);
        // stream.attach(new SyncPacketQueue, 2, true);
        // stream.synchronizeOutput(uv::defaultLoop());
        stream.attach(new MockPacketProcessor,  true);

       // stream.emitter += slot(this, &PacketStreamTest::onPacketStreamOutput);
        // stream.emitter += packetSlot(this,
        // &PacketStreamTest::onPacketStreamOutput);
        // stream.emitter.attach<PacketStreamTest,
        // &PacketStreamTest::onPacketStreamOutput>(this);

        stream.start();

        // TODO: Test pause/resume functionality
        // Run the thread for 100ms
        base::sleep(70000);

        stream.stop();

    }
};


struct MockMediaPacketSource : public PacketProcessor, public Thread
{
   
   // int numFramesRemaining = 10;

   MockMediaPacketSource()
    {
        LTrace("MockMediaPacketSource")
    }
    
     ~MockMediaPacketSource()
    {
         LTrace("~MockMediaPacketSource")
    }

    void run()
    {
    
        for( int i=0; i < 5 ; ++i )
        {
            std::cout << "Emitting" << std::endl;
            
//            std::random_device rd;
//            std::mt19937 rng(rd());
//            std::uniform_int_distribution<int> uni(1000, 1000000 * 5);
//            auto time = uni(rng);
            std::ostringstream str;
            str << "hell" << i ;
           // RawPacket p( str.str().c_str(), 6);
            uint8_t *tmp = new uint8_t[6];
            memcpy(tmp, str.str().c_str(),6 );
            MediaPacket p(tmp, 6, Application::GetTime());
           // p.assignDataOwnership();
            emit(p);
            
        }
    }
    
    void start()
    {
        std::cout << "Start" << std::endl;
        Thread::start();
    }

    void stop()
    {
        std::cout << "Stop" << std::endl;
        // runner.close();
        Thread::stop();
        // std::cout << "Stop: OK" << std::endl;
    }

 
};


class RealtimeMediaQueueTest : public Test
{
public:
     int numFramesRemaining = 4;
        
    PacketStream stream;
   
    void run()
    {
        using std::placeholders::_1;
        // Create the multiplex encoder
        // auto queue(std::make_shared<av::RealtimePacketQueue<>());

        stream.attachSource(new MockMediaPacketSource, true);
        stream.attach(new RealtimePacketQueue<MediaPacket>(), true );
        stream.start();
        //stream.cbProcess = std::bind( &RealtimeMediaQueueTest::onPacketPlayout, this, _1);

         base::sleep(700000);

        stream.stop();
        
    }

    
};
std::string sampleDataDir(const std::string& file)
{
    std::string dir;
    fs::addnode(dir, base_SOURCE_DIR);
    fs::addnode(dir, "ffmpeg");
    fs::addnode(dir, "samples");
    fs::addnode(dir, "data");
    if (!file.empty())
        fs::addnode(dir, file);
    return dir;
}

int main(int argc, char** argv)
{
    Logger::instance().add(new ConsoleChannel("Trace", Level::Trace)); // Level::Trace, Level::Debug
    // Logger::instance().setWriter(new AsyncLogWriter);
    //test::init();
 
     
    //RealtimeMediaQueueTest test;
   // test.run();
    
   // return 0;
    
    //PacketStreamTest test;
    
    //test.run();
    
   // return 0;
    
    Application app;
    
      ff::MediaCapture  _capturer;

      std::string sourceFile(sampleDataDir("test.mp4"));
      LTrace("file: ", sourceFile)
     _capturer.openFile(sourceFile);
      
      _capturer.start();
      
      app.waitForShutdown([&](void*) {

          _capturer.stop();

    });
      
    
   
      
   // test::runAll();

    return test::finalize();
}

