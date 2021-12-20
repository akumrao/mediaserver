/* This file is part of mediaserver. A webrtc RTSP server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.

  for testing reset api use Postman
 * https://web.postman.co/workspace/My-Workspace~292b44c7-cae4-44d6-8253-174622f0233e/request/create?requestId=e6995876-3b8c-4b7e-b170-83a733a631db
 */

#include "base/filesystem.h"
#include "base/application.h"
#include "base/util.h"
#include "base/idler.h"
#include "base/logger.h"
#include "Settings.h"
#include "fmp4.h"
#include "json/configuration.h"
#include "rtc_base/ssl_adapter.h"
#include "webrtc/signaler.h"


using namespace std;
using namespace base;

/*
// Detect Memory Leaks
#ifdef _DEBUG
#include "MemLeakDetect/MemLeakDetect.h"
#include "MemLeakDetect/MemLeakDetect.cpp"
CMemLeakDetect memLeakDetect;
#endif
 */

#define SERVER_HOST "127.0.0.1"
#define SERVER_PORT 443
#define JOIN_ROOM  "foo"        



//std::string sampleDataDir(const std::string& file) {
//    std::string dir;
//   // fs::addnode(dir, base_SOURCE_DIR);
//    fs::addnode(dir, "/");
//    fs::addnode(dir, "var");
//    fs::addnode(dir, "tmp");
//    if (!file.empty())
//        fs::addnode(dir, file);
//    return dir;
//}


void IgnoreSignals() {
#ifndef _WIN32

    int err;
    struct sigaction act; // NOLINT(cppcoreguidelines-pro-type-member-init)

    // clang-format off
    std::map<std::string, int> ignoredSignals ={
        { "PIPE", SIGPIPE},
        { "HUP", SIGHUP},
        { "ALRM", SIGALRM},
        { "USR1", SIGUSR1},
        { "USR2", SIGUSR2}
    };
    // clang-format on

    act.sa_handler = SIG_IGN; // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
    act.sa_flags = 0;
    err = sigfillset(&act.sa_mask);

    if (err != 0)
        base::uv::throwError("sigfillset() failed: ", errno);

    for (auto& kv : ignoredSignals) {
        auto& sigName = kv.first;
        int sigId = kv.second;

        err = sigaction(sigId, &act, nullptr);

        if (err != 0)
            base::uv::throwError("sigaction() failed for signal " + sigName, errno);
    }
#endif
}

int main(int argc, char** argv) {
     //Logger::instance().add(new ConsoleChannel("debug", Level::Info));
    
   // ConsoleChannel *ch =  new ConsoleChannel("debug", Level::Info);
            
    //Logger::instance().add(ch);
   
     av_register_all();
    // init network
     avformat_network_init();
     avcodec_register_all();
       
    //Logger::instance().add(new FileChannel("mediaserver","/var/log/mediaserver", Level::Info));
   // Logger::instance().setWriter(new AsyncLogWriter);
    
    base::cnfg::Configuration config;

    config.load("./config.js");
  
    json cnfg;
   
    if( !config.getRaw("webrtc", cnfg))
    {
        std::cout << "Could not parse config file";
    }
            

    try {
        Settings::SetConfiguration(cnfg);
    } catch (const std::exception& error) {

        std::_Exit(-1);
    } 


    // Setup WebRTC environment
    rtc::LogMessage::LogToDebug(rtc::LS_ERROR); // LS_VERBOSE, LS_INFO, LS_ERROR
    // rtc::LogMessage::LogTimestamps();
    // rtc::LogMessage::LogThreads();

    rtc::InitializeSSL();


    Application app;

    //std::string sourceFile(sampleDataDir("test.mp3"));

    base::wrtc::Signaler sig;

   // sig.startStreaming("/var/tmp/songs", "", "mp3",  false);
    
    //sig.startStreaming("/var/tmp/videos", "", "mp4",  false);
    
    //sig.startStreaming("", "/var/tmp/test.mp4", "mp4", true); // single file play in loop, this feauture migt be broken.
    

    sig.connect(SERVER_HOST, SERVER_PORT, JOIN_ROOM);
    
    
    
    fmp4::ReadMp4 *readmp4 = new  fmp4::ReadMp4("0.0.0.0", 8080, new fmp4::StreamingResponderFactory1( sig)  );
     
    

    // test._capturer.start();

//    auto rtcthread = rtc::Thread::Current();
//    Idler rtc([rtcthread]() {
//        rtcthread->ProcessMessages(3);
//       // LTrace(" rtcthread->ProcessMessages")
//        base::sleep(1000);
//    });

    //LTrace("app.run() run start")
   // app.run();

   // Ignore some signals.
    IgnoreSignals();
        
   app.waitForShutdown([&](void*)
   {

    LTrace("app.run() is over")
    rtc::CleanupSSL();
    Logger::destroy();
    
    readmp4->stop();
        
    readmp4->shutdown();
    
   });

    return 0;
}



