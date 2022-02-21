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
#define SERVER_PORT 9093
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
  
   // json cnfg;
   
//    if( !config.getRaw("webrtc", cnfg))
//    {
//        std::cout << "Could not parse config file";
//    }
            
    Settings::init();
    
    try {
        Settings::SetConfiguration(config.root);
    } catch (const std::exception& error) {

       Settings::exit();
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
    
    
    
    fmp4::ReadMp4 *readmp4 = new  fmp4::ReadMp4("0.0.0.0", 8080, sig,  new fmp4::StreamingResponderFactory1( sig)  );
     
    

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
    Settings::exit();         
    rtc::CleanupSSL();
    Logger::destroy();
    
    readmp4->stop();
        
    readmp4->shutdown();
    
   });

    return 0;
}

/*
 ./configure --pkg-config-flags="--static" --libdir=/usr/local/lib --disable-shared --enable-static --enable-gpl --enable-pthreads --enable-nonfree  --enable-libfdk-aac    --enable-libx264 --enable-filters --enable-runtime-cpudetect --disable-lzma
 
 export PATH=/export/webrtc/depot_tools:$PATH
 * 
 gn gen out/m75 --args='is_debug=true symbol_level=2 is_component_build=false is_clang=false rtc_include_tests=false rtc_use_h264=false rtc_enable_protobuf=false use_rtti=false use_custom_libcxx=false treat_warnings_as_errors=false use_ozone=false  use_glib=false rtc_use_pipewire=false rtc_use_gtk=false rtc_include_pulse_audio=false rtc_include_tests=false  treat_warnings_as_errors=false rtc_include_ilbc=false rtc_build_examples=false rtc_build_tools=false enable_iterator_debugging=false rtc_use_x11=false use_gio=false '
ninja -C out/m75 webrtc
 
 
* 
 gn gen out/m75 --args='is_debug=false symbol_level=0 is_component_build=false is_clang=false rtc_include_tests=false rtc_use_h264=false rtc_enable_protobuf=false use_rtti=false use_custom_libcxx=false treat_warnings_as_errors=false use_ozone=false  use_glib=false rtc_use_pipewire=false rtc_use_gtk=false rtc_include_pulse_audio=false rtc_include_tests=false  treat_warnings_as_errors=false rtc_include_ilbc=false rtc_build_examples=false rtc_build_tools=false enable_iterator_debugging=false rtc_use_x11=false use_gio=false '
ninja -C out/m75/ webrtc
 

core dump location

/var/lib/apport/coredump/
  
pmap -x `pidof runWebrtc`

lsof -p `pidof runWebrtc`   // number of file descriptor 

valgrind --leak-check=full   --show-leak-kinds=all  --track-origins=yes  ./runWebrtc  >& /var/tmp/leak.txt



 *  
 
valgrind --leak-check=full   --show-leak-kinds=all  --track-origins=yes  ./runWebrtc  >& /var/tmp/leak.txt

==16187== LEAK SUMMARY:
==16187==    definitely lost: 0 bytes in 0 blocks
==16187==    indirectly lost: 0 bytes in 0 blocks
==16187==      possibly lost: 29,517,882 bytes in 1,441 blocks
==16187==    still reachable: 3,692,609 bytes in 5,000 blocks
==16187==                       of which reachable via heuristic:
==16187==                         length64           : 22,043 bytes in 250 blocks
==16187==                         multipleinheritance: 392 bytes in 1 blocks
==16187==         suppressed: 0 bytes in 0 blocks
==16187== 
==16187== For counts of detected and suppressed errors, rerun with: -v
==16187== ERROR SUMMARY: 7444071 errors from 278 contexts (suppressed: 0 from 0)

 */ 
 
