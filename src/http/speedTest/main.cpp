#include <iostream>
#include <map>
#include <iomanip>
#include "SpeedTest.h"
#include "TestConfigTemplate.h"
#include <csignal>
#include "base/logger.h"
#include "base/thread.h"
#include "base/platform.h"

using namespace base;

class Speed : public Thread {
public:

    Speed();
    ~Speed();
    void run();

};

Speed::Speed() {

    signal(SIGPIPE, SIG_IGN);
}

Speed::~Speed() {
    LTrace("~Speed()")
    join();
}

void Speed::run() {

    LTrace("Speed OnRun");


    SpeedTest sp = SpeedTest(SPEED_TEST_MIN_SERVER_VERSION);
    //IPInfo info;
    ServerInfo serverInfo;
    ServerInfo serverQualityInfo;


    auto serverList = sp.serverList();

    STrace << serverList.size() << " Servers online" << std::endl;

    serverInfo = sp.bestServer(10, [&](bool success) {

        std::cout << (success ? '.' : '*') << std::flush;

        return stopped();
    });

    if (stopped())
        return;


    STrace << "Server: " << serverInfo.name
            << " " << serverInfo.host
            << " by " << serverInfo.sponsor
            << " (" << serverInfo.distance << " km from you): "
            << sp.latency() << " ms" << std::endl;


    STrace << "Ping: " << sp.latency() << " ms." << std::endl;

    STrace << "Latency:" << sp.latency() << " ms." << std::endl;

    long jitter = 0;
    if (sp.jitter(serverInfo, jitter)) {
        STrace << "Jitter:" << jitter << " ms." << std::endl;
    }


    TestConfig uploadConfig;
    TestConfig downloadConfig;
    downloadConfig = narrowConfigDownload;
    uploadConfig = narrowConfigUpload;

    /*       
       double preSpeed = 0;
       if (!sp.downloadSpeed(serverInfo, preflightConfigDownload, preSpeed, [&programOptions](bool success){
      
               std::cout << (success ? '.' : '*') << std::flush;
                std::cout << "Testing download speed (" << downloadConfig.concurrency << ") "  << std::flush;
       })){
           std::cerr << "Pre-flight check failed." << std::endl;
           return;
       }
       testConfigSelector(preSpeed, uploadConfig, downloadConfig);
     */

    if (stopped())
        return;

    double downloadSpeed = 0;
    if (sp.downloadSpeed(serverInfo, downloadConfig, downloadSpeed, [&](bool success, double matrix) {

            //std::cout << (success ? '.' : '*') << std::flush;
            if (success)
                STrace << std::fixed << std::setprecision(2) << uploadConfig.concurrency * matrix / 1000000 << " Mbit/s" << std::endl << std::flush;
            else {
                STrace << "download failed" << std::endl << std::flush;
                return false;
            }

            return stopped();
        })) {

    STrace <<  "Download: " << std::fixed << std::setprecision(2) << downloadSpeed << " Mbit/s" << std::endl;
   

} else {
        STrace << "Download test failed." << std::endl;

        return;
    }

    if (stopped())
        return;


    double uploadSpeed = 0;
    if (sp.uploadSpeed(serverInfo, uploadConfig, uploadSpeed, [&](bool success, double matrix) {

            // std::cout << (success ? '.' : '*') << std::flush;
            if (success)
                STrace << "Upload:" << std::fixed << std::setprecision(2) << uploadConfig.concurrency * matrix / 1000000 << " Mbit/s" << std::endl << std::flush;
            else {
                std::cout << "upload test failed" << std::endl << std::flush;
                return false;
            }
            return stopped();
        })) {

    STrace << "Upload: " << std::fixed << std::setprecision(2) << uploadSpeed << " Mbit/s" << std::endl;
   

} else {
        STrace << "Upload test failed." << std::endl;
        return;
    }

    LTrace("Speed Over");
}

int main(const int argc, const char **argv) {

    //Logger::instance().add(new RemoteChannel("Remote", Level::Remote, "127.0.0.1", 6000));

    Logger::instance().add(new ConsoleChannel("Trace", Level::Trace));


    Speed *upload = new Speed();

    upload->start();

    base::sleep(75000);

    LTrace("upload stop")

    upload->stop();

    delete upload;

    LTrace("upload done");


    return 0;

}
