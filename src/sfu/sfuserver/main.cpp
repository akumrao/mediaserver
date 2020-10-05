
#include "json/configuration.h"
#include "common.h"
#include "DepLibSRTP.h"
#include "base/application.h"
#include "DepLibWebRTC.h"
#include "DepOpenSSL.h"
#include "DepUsrSCTP.h"
#include "LoggerTag.h"
#include "base/error.h"
#include "Settings.h"
#include "Utils.h"

#include "Channel/Notifier.h"

#include "RTC/DtlsTransport.h"
#include "RTC/SrtpSession.h"
#include <uv.h>
#include <cerrno>
#include <csignal>  // sigaction()
#include <cstdlib>  // std::_Exit(), std::genenv()
#include <iostream> // std::cerr, std::endl
#include <map>
#include <string>

#include "base/platform.h"
#include "sdp/signaler.h"



#define SERVER_HOST "127.0.0.1"
#define SERVER_PORT 8080 //443
#define USE_SSL     0 //1
//#define JOIN_ROOM  "foo"  


using namespace base;

void IgnoreSignals();

int main(int argc, char* argv[]) {

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
        MS_ERROR_STD("settings error: %s", error.what());

        std::_Exit(-1);
    } 

    
#if defined(MS_LITTLE_ENDIAN)
    MS_DEBUG_TAG(info, "little-endian CPU detected");
#elif defined(MS_BIG_ENDIAN)
    MS_DEBUG_TAG(info, "big-endian CPU detected");
#else
    MS_WARN_TAG(info, "cannot determine whether little-endian or big-endian");
#endif

#if defined(INTPTR_MAX) && defined(INT32_MAX) && (INTPTR_MAX == INT32_MAX)
    MS_DEBUG_TAG(info, "32 bits architecture detected");
#elif defined(INTPTR_MAX) && defined(INT64_MAX) && (INTPTR_MAX == INT64_MAX)
    MS_DEBUG_TAG(info, "64 bits architecture detected");
#else
    MS_WARN_TAG(info, "cannot determine 32 or 64 bits architecture");
#endif

  
    Settings::PrintConfiguration();
   
    //try {
        
         
        base::Application app;
        SdpParse::Signaler sig;

        // Initialize static stuff.
        DepOpenSSL::ClassInit();
        DepLibSRTP::ClassInit();
        DepUsrSCTP::ClassInit();
        DepLibWebRTC::ClassInit();
        Utils::Crypto::ClassInit();
        RTC::DtlsTransport::ClassInit();
        RTC::SrtpSession::ClassInit();
        Channel::Notifier::ClassInit(&sig);

        // Ignore some signals.
        IgnoreSignals();

        //std::string sourceFile(sampleDataDir("test.mp4"));

        //sig.startStreaming(sourceFile, true);
        sig.connect(SERVER_HOST, SERVER_PORT);


        app.waitForShutdown([&](void*) {

            DepLibSRTP::ClassDestroy();
            Utils::Crypto::ClassDestroy();
            DepLibWebRTC::ClassDestroy();
            RTC::DtlsTransport::ClassDestroy();
            DepUsrSCTP::ClassDestroy();

        });

        // Free static stuff.
        //base::Application::ClassDestroy();


        // Wait a bit so peding messages to stdout/Channel arrive to the Node
        // process.
        //uv_sleep(200);
        //base::sleep(200);

//        std::_Exit(EXIT_SUCCESS);
//    } catch (const exception& error) {
//        MS_ERROR_STD("failure exit: %s", error.what());
//
//        std::_Exit(EXIT_FAILURE);
//    }

  
}

void IgnoreSignals() {
#ifndef _WIN32
    MS_TRACE();

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
