/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#include "base/test.h"
#include "base/logger.h"
#include "base/application.h"
#include "http/HTTPResponder.h"

#include "fmp4.h"
//#include "ff/ff.h"
//#include "ff/mediacapture.h"

//#include <libavutil/timestamp.h>
extern "C"
{
//#include <libavutil/timestamp.h>
#include <avformat.h>
}

using namespace base;

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

    //Logger::instance().add(new ConsoleChannel("debug", Level::Debug));
    //test::init();
    
    Logger::instance().add(new FileChannel("mediaserver","/var/log/mediaserver", Level::Trace));
    Logger::instance().setWriter(new AsyncLogWriter);
    
    Application app;
   
    av_register_all();
    // init network
    avformat_network_init();
    avcodec_register_all();

      
    fmp4::ReadMp4 *readmp4 = new  fmp4::ReadMp4("0.0.0.0", 80, new net::StreamingResponderFactory()  );
    
    //readmp4.websocketConnect();

    // Ignore some signals.
    IgnoreSignals();
        
    app.waitForShutdown([&](void*) {

    readmp4->stop();
        
    readmp4->shutdown();
    });
    
   // app.run();
    
   

}
