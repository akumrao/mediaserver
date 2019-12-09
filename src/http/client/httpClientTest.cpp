#include "net/netInterface.h"
#include "httpClientTest.h"
#include "http/client.h"
#include "base/test.h"
#include "base/logger.h"
#include "base/application.h"

#include "http/url.h"
#include "http/util.h"
#include "base/filesystem.h"
#include "http/client.h"
#include "crypto/hash.h"

using namespace base;
using namespace base::net;
using namespace base::test;

int main(int argc, char** argv) {

    Logger::instance().add(new ConsoleChannel("Info", Level::Trace));

       test::init();
     
           
   /*
    {
        Application app;
 
        Client *conn = new Client();
        conn->createConnection("ws","arvindubuntu",7000,"websocket");
        
        //Client *conn = new Client("http://zlib.net/index.html");
        conn->start();
     //   conn->clientConn->fnComplete = [&](const Response & response) {
       //     std::cout << "Lerver response:";
        //};
 //       conn->clientConn->_request.setMethod("GET");
//        conn->clientConn->_request.setKeepAlive(false);
//        conn->clientConn->setReadStream(new std::ofstream(path, std::ios_base::out | std::ios_base::binary));
        conn->clientConn->send("Ping");

        app.run();
    }
*/
        /*
         {
     
       Application app;
        std::string path("./");
        fs::addnode(path, "testDel.gz");

        Client *conn = new Client("http://ftp.debian.org/debian/dists/Debian8.11/main/Contents-armhf.gz");
       // Client *conn = new Client("http://zlib.net/index.html");
        conn->start();
        conn->clientConn->fnComplete = [&](const Response & response) {
            std::cout << "Lerver response:";
        };
        conn->clientConn->_request.setMethod("GET");
        conn->clientConn->_request.setKeepAlive(false);
        conn->clientConn->setReadStream(new std::ofstream(path, std::ios_base::out | std::ios_base::binary));
        conn->clientConn->send();

        app.run();
        
        expects(fs::exists(path));
        expects(crypto::checksum("MD5", path) == "44d667c142d7cda120332623eab69f40");
        fs::unlink(path);
    }
    
    {
     
        Application app;
        std::string path("./");
        fs::addnode(path, "testdel.gz");

        Client *conn = new Client("https://dl.google.com/dl/android/studio/ide-zips/3.5.2.0/android-studio-ide-191.5977832-linux.tar.gz");
       // Client *conn = new Client("http://zlib.net/index.html");
        conn->start();
        conn->clientConn->fnComplete = [&](const Response & response) {
            std::cout << "Lerver response:";
        };
        conn->clientConn->_request.setMethod("GET");
        conn->clientConn->_request.setKeepAlive(false);
        conn->clientConn->setReadStream(new std::ofstream(path, std::ios_base::out | std::ios_base::binary));
        conn->clientConn->send();

        app.run();
        
        expects(fs::exists(path));
        expects(crypto::checksum("MD5", path) == "44d667c142d7cda120332623eab69f40");
        fs::unlink(path);
    }

    */


    
    
    

    describe("url parser", []() {

        URL url;
        expects(url.scheme().empty());
        expects(url.authority().empty());
        expects(url.userInfo().empty());
        expects(url.host().empty());
        expects(url.port() == 0);
        expects(url.path().empty());
        expects(url.query().empty());
        expects(url.fragment().empty());

        URL url2("HTTP", "localhost", "/home/google/foo.bar");
        expects(url2.scheme() == "http");
        expects(url2.host() == "localhost");
        expects(url2.path() == "/home/google/foo.bar");

        URL url3("http", "www.google.com", "/index.html");
        expects(url3.scheme() == "http");
        expects(url3.authority() == "www.google.com");
        expects(url3.host() == "www.google.com");
        expects(url3.path() == "/index.html");

        URL url4("http", "www.google.com:8000", "/index.html");
        expects(url4.scheme() == "http");
        expects(url4.authority() == "www.google.com:8000");
        expects(url4.host() == "www.google.com");
        expects(url4.path() == "/index.html");

        URL url5("http", "user@www.google.com:8000", "/index.html");
        expects(url5.scheme() == "http");
        expects(url5.userInfo() == "user");
        expects(url5.host() == "www.google.com");
        expects(url5.port() == 8000);
        expects(url5.authority() == "user@www.google.com:8000");
        expects(url5.path() == "/index.html");

        URL url6("http", "user@www.google.com:80", "/index.html");
        expects(url6.scheme() == "http");
        expects(url6.userInfo() == "user");
        expects(url6.host() == "www.google.com");
        expects(url6.port() == 80);
        expects(url6.authority() == "user@www.google.com:80");
        expects(url6.path() == "/index.html");

        URL url7("http", "www.google.com", "/index.html", "query=test", "fragment");
        expects(url7.scheme() == "http");
        expects(url7.authority() == "www.google.com");
        expects(url7.path() == "/index.html");
        expects(url7.pathEtc() == "/index.html?query=test#fragment");
        expects(url7.query() == "query=test");
        expects(url7.fragment() == "fragment");

        URL url8("http", "www.google.com", "/index.html?query=test#fragment");
        expects(url8.scheme() == "http");
        expects(url8.authority() == "www.google.com");
        expects(url8.path() == "/index.html");
        expects(url8.pathEtc() == "/index.html?query=test#fragment");
        expects(url8.query() == "query=test");
        expects(url8.fragment() == "fragment");
    });


    describe("DNS Resolve", []() {
        GetAddrInfoReq infoReq;
        infoReq.resolve("zlib.net", 80);

        uv_run(uv_default_loop(), UV_RUN_DEFAULT);

    });


    describe("http download", []() {

        Application app;
        std::string path("/tmp/tmp/");
        fs::addnode(path, "zlib-1.2.8.tar.gz");

        Client *conn = new Client("http://zlib.net/fossils/zlib-1.2.8.tar.gz");
        //Client *conn = new Client("http://zlib.net/index.html");
        conn->start();
        conn->clientConn->fnComplete = [&](const Response & response) {
            std::cout << "Lerver response:";
        };
        conn->clientConn->_request.setMethod("GET");
        conn->clientConn->_request.setKeepAlive(false);
        conn->clientConn->setReadStream(new std::ofstream(path, std::ios_base::out | std::ios_base::binary));
        conn->clientConn->send();

        app.run();

        expects(fs::exists(path));
        expects(crypto::checksum("MD5", path) == "44d667c142d7cda120332623eab69f40");
        fs::unlink(path);
        

    });


    test::runAll();

    return test::finalize();
}
