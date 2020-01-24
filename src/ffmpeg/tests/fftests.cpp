#include "ff/ff.h"
#include "ff/mediacapture.h"
#include "base/define.h"
#include "base/test.h"
#include "base/filesystem.h"
#include "base/application.h"

using namespace std;
using namespace base;
using namespace base::ff;
using namespace base::test;


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

    Application app;
    
      ff::MediaCapture  _capturer;

      std::string sourceFile(sampleDataDir("test.mp4"));
      LTrace("file: ", sourceFile)
     _capturer.openFile(sourceFile);
      
      _capturer.start();
      
   //   _capturer.start();

       app.waitForShutdown([&](void*) {

          _capturer.stop();

    });
      
    
   
      
   // test::runAll();

    return test::finalize();
}

