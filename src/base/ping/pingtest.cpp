
#include "base/process.h"
#include "base/thread.h"
#include  <iostream>
#include  <string>
#include "base/logger.h"
#include <sstream>

using std::cout;
using std::cerr;
using std::endl;
using namespace base;

void test_it(void* p) {
    std::cout << "ok!\n";
}

class Thread3 {
public:

    Thread3(std::function< void (void*) > func) {
        typedef void function_t(void*);
        std::cout << func.target<function_t*>() << std::endl; // <-- Always nullptr
    }
};



#define CALLBACK_T(fn)     void (*fn)(void*)      // Declaring a callback
#define CALLBACKARG_T(num) void* (*fn##num)(void*) // Declaring a function arg to accept a callback
#define CALLBACKARG(num)   fn##num                 // Used to access a callback function arg
/*
void fn(void* ithis) {
    std::cout << "ok!\n";

}

class Thread2 : public Thread {
public:

    Thread2() {
    }
    // virtual ~Thread2(void);

    void run() {
        std::cout << "Thread2!\n";
    }
};
*/
class PingThread : public Thread {
public:

    PingThread(std::string host)  {

        proc.args = {"ping", "-W",  "4", host};
    }
    // virtual ~Thread2(void);

    void run() {

        proc.onstdout = [&](std::string line) {
         
            
            std::stringstream X(line); 
            std::string T; 
  
            while (std::getline(X, T, '\n')) { 
                LTrace(T);
            } 
           
            
        };
        proc.onexit = [&](int64_t status) {
            LTrace( "onexit", status);
        };
        proc.spawn();
        uv_run(uv_default_loop(), UV_RUN_DEFAULT);

    }

    void stop() {
        proc.kill(SIGINT);
    }

    Process proc;

};

int main(int argc, char** argv) {

    //Logger::instance().add(new RotatingFileChannel("test", "/tmp/test", Level::Trace, "log", 10));
    Logger::instance().add(new ConsoleChannel("debug", Level::Trace, "" ));

    bool gotStdout = false, gotExit = false;
    //Process proc{ "ping", "-c 29", "8.8.8.8"};

    // Process proc{ "ls", "-a"};
    std::string host = "www.google.com";
    //std::string host = "8.8.8.8";
     
    PingThread pingThread(host);

    pingThread.start();
    
    sleep(3);
    LTrace( "stop");
    
    pingThread.stop();
    
   LTrace( "stop1");
   sleep(100);
    return 0;


}
