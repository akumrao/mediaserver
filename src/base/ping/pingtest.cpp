
#include "base/process.h"
#include "base/thread.h"
#include  <iostream>
#include  <string>
#include "base/logger.h"

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

int main(int argc, char** argv) {

    //Logger::instance().add(new RotatingFileChannel("test", "/tmp/test", Level::Trace, "log", 10));
    Logger::instance().add(new ConsoleChannel("debug", Level::Trace));

    bool gotStdout = false, gotExit = false;
    //Process proc{ "ping", "-c 29", "8.8.8.8"};

     Process proc{ "ls", "-a"};
    proc.onstdout = [&](std::string line) {
        std::cout << "process stdout: " << line << std::endl;
        gotStdout = true;
        //proc.kill();
    };
    proc.onexit = [&](int64_t status) {
        std::cout << "process exit: " << status << std::endl;
        gotExit = true;
    };
    proc.spawn();
    uv_run(uv_default_loop(), UV_RUN_DEFAULT);


    return 0;


}
