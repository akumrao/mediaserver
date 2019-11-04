#include "basetests.h"

using std::cout;
using std::cerr;
using std::endl;
using namespace base;
using namespace base::test;

void test_it(void* p) {
    std::cout << "ok!\n";
}

class Thread3
{
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

class Thread2 : public Thread
{
public:

    Thread2() {
    }
    // virtual ~Thread2(void);

    void run() {
        std::cout << "Thread2!\n";
    }
};

int main(int argc, char** argv) {


    //Logger::instance().add(new RotatingFileChannel("debug", Level::Trace));

    Logger::instance().add(new RotatingFileChannel("test", "/tmp/test", Level::Trace, "log", 10));



    test::init();

    // =========================================================================
    // Thread
    //
    describe("thread", []()
    {
        bool ran = false;
        Thread t1;
        t1.start(fn, &ran);


        t1.join();
        //expects(ran == true);
        expects(t1.running() == false);

        // Reuse the same thread container
        ran = false;
        t1.start(fn, &ran);
        t1.join();
        //expects(ran == true);
        expects(t1.running() == false);

        Thread2 th;
        th.start();
        th.join();

        CALLBACK_T(FnObj) = [](void* data)
        {
            std::chrono::steady_clock clock;
            std::chrono::time_point<std::chrono::steady_clock> tref1, tref2;
            std::chrono::duration<int, std::milli> ms(500);
            tref1 = clock.now();
            while (1)
            {
                static int i = 0;

                if (tref2 - tref1 >= ms)
                {
                    tref1 = clock.now();
                            std::cout << "check " << i << std::endl;
                    if (++i > 5) break;
                    }
                tref2 = clock.now();
            }
        };

        th.start(FnObj, nullptr);

        th.join();

    });



    // =========================================================================
    // Filesystem
    //
    describe("filesystem", []()
    {
        std::string path(base::getExePath());
        // cout << "executable path: " << path << endl;
        expects(fs::exists(path));

        std::string junkPath(path + "junkname.huh");
        // cout << "junk path: " << junkPath << endl;
        expects(!fs::exists(junkPath));

        std::string dir(fs::dirname(path));
        // cout << "dir name: " << dir << endl;
        expects(fs::exists(dir));
        expects(fs::exists(dir + "/"));
        expects(fs::exists(dir + "\\"));
        expects(fs::dirname("file.a") == ".");
        expects(fs::dirname("/some/file.a") == "/some");
        expects(fs::dirname("/some/dir") == "/some");
        expects(fs::dirname("/some/dir/") == "/some/dir");

        std::string test(fs::dirname(dir));
        fs::addnode(test, "tests");
        expects(test == dir);
    });

    // =========================================================================
    // Logger

    describe("logger", []()
    {
        Logger& logger = Logger::instance();

        // Test default synchronous writer
        logger.setWriter(new LogWriter);
        clock_t start = clock();
        for (unsigned i = 0; i < 1000; i++)
                LTrace("sync log ", i)
                cout << "logger: synchronous test completed after: " << (clock() - start) << "ms" << endl;

                // Test default synchronous var args writer
                start = clock();
            for (unsigned i = 0; i < 1000; i++)
                    LTrace("sync log", i)
                    cout << "logger: synchronous var args test completed after: " << (clock() - start) << "ms" << endl;

                    // Test asynchronous writer (approx 10x faster with console output)
                    logger.setWriter(new AsyncLogWriter);
                    start = clock();
                for (unsigned i = 0; i < 1000; i++)
                        LTrace("async log ", i)
                        cout << "logger: asynchronous test completed after: " << (clock() - start) << "ms" << endl;

                        //// Test function logging
                        //start = clock();
                        //for (unsigned i = 0; i < 1000; i++)
                        //    LTrace("test: ", i)
                        //cout << "logger: asynchronous function logging completed after: "
                        //     << (clock() - start) << endl;

                        //// Test function and mem address logging
                        //start = clock();
                        //for (unsigned i = 0; i < 1000; i++)
                        //    LTrace("test: ", i)
                        //cout << "logger: asynchronous function and mem address logging completed after: "
                        //     << (clock() - start) << endl;

                        logger.setWriter(nullptr);

                        // TODO: Test log filtering
                        // logger.getDefault()->setFilter("base::*");
                        // Destory the current Logger instance to ensure no crash
                        Logger::destroy();
                });



    test::runAll();

    return test::finalize();
}
