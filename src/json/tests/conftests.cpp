#include "conftests.h"

#include "json/configuration.h"
//#include "json/json.hpp"
#include <iostream>     // std::cout
#include <fstream>      // std::ifstream

using std::cout;
using std::cerr;
using std::endl;



//using namespace nlohmann;
using namespace base;
using namespace base::cnfg;
using namespace base::test;

int main(int argc, char** argv) {


    Logger::instance().add(new RotatingFileChannel("test", "/tmp/test", Level::Trace, "log", 10));

    test::init();

    // =========================================================================
    // Thread
    //
    describe("configure", []()
    {

        cnfg::Configuration config;

        config.load("./test2.json");

        std::string key = "version"; 
        config.setRaw(key, "22") ;

        std::string value;
        config.getRaw(key , value);
        
        expects( value == "22"  ); 
        config.save();

    });


    describe("json.hpp test, please use json.h instead of json.hpp", []()
    {

        std::ifstream i("./test.json");
        json j;
        i >> j;


        std::ofstream o("./pretty.json");
        o << j << std::endl;

    });

    test::runAll();

    return test::finalize();


}
