#include "httpClientTest.h"
#include "http/client.h"
#include "base/test.h"
#include "base/logger.h"
#include "base/application.h"

#include "http/url.h"
#include "http/util.h"
#include "base/filesystem.h"
#include "http/client.h"

using namespace base;
using namespace base::net;
using namespace base::test;

/*
class BasicResponder : public net::ServerResponder
/// Basic server responder (make echo?)
{
public:

    BasicResponder(net::TcpHTTPConnection* conn) :
    ServerResponder(conn) {
        ///		DebugL << "Creating" << endl;
    }

    virtual void onClose() {
        ;
        LDebug("On close")

    }

    void onRequest(net::Request& request, net::Response& response) {
        //DebugL << "On complete" << endl;

        response.setContentLength(14); // headers will be auto flushed

        connection()->Send((const uint8_t *) "hello universe", 14);
        connection()->Close();
    }
};

class StreamingResponderFactory : public ServerConnectionFactory {
public:

    ServerResponder* createResponder(TcpHTTPConnection* conn) {
        return new BasicResponder(conn);
    }
};
 */







int main(int argc, char** argv) {

    GetAddrInfoReq infoReq;
    
    
    infoReq.resolve("zlib.net", 80);
    
    uv_run(uv_default_loop(), UV_RUN_DEFAULT);
    
    
    
    
    Logger::instance().add(new ConsoleChannel("debug", Level::Trace));
    test::init();

    Application app;
    std::string path("/var/tmp/");
    fs::addnode(path, "zlib-1.2.8.tar.gz");

    Client *conn = new Client("http://zlib.net/fossils/zlib-1.2.8.tar.gz");
    conn->start();
    conn->clientConn->fnComplete = [&](const Response & response) {
        std::cout << "Lerver response:";
    };
    conn->clientConn->_request.setMethod("GET");
    conn->clientConn->_request.setKeepAlive(false);
    conn->clientConn->setReadStream(new std::ofstream(path, std::ios_base::out | std::ios_base::binary));
    conn->clientConn->Send();

    

    app.waitForShutdown([&](void*) {

        conn->shutdown();

    });

    //uv::runLoop();

    //expect(fs::exists(path));
    // expect(crypto::checksum("MD5", path) == "44d667c142d7cda120332623eab69f40");
    //fs::unlink(path);

    return 0;


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





    test::runAll();

    return test::finalize();
}
