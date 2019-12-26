//#include "httptests.h"
#include "http/HttpServer.h"
#include "base/test.h"
#include "base/logger.h"
#include "base/application.h"

using namespace base;
using namespace base::net;
using namespace base::test;

class BasicResponder : public net::ServerResponder
/// Basic server responder (make echo?)
{
public:

    BasicResponder(net::HttpBase* conn) :
    ServerResponder(conn) {
        STrace << "BasicResponder" << std::endl;
    }

    virtual void onClose() {
        ;
        LDebug("On close")

    }

    void onRequest(net::Request& request, net::Response& response) {
        STrace << "On complete" << std::endl;

        response.setContentLength(14); // headers will be auto flushed

        connection()->send((const char *) "hello universe", 14);
        connection()->Close();
    }
};

class StreamingResponderFactory : public ServerConnectionFactory {
public:

    ServerResponder* createResponder(HttpBase* conn) {
        
         auto& request = conn->_request;

        // Log incoming requests
        STrace << "Incoming connection from " << ": URI:\n" << request.getURI() << ": Request:\n" << request << std::endl;

        // Handle websocket connections
        if (request.getURI().find("/upload") == 0 ) {   // || request.has("Sec-WebSocket-Key")) {
            return new BasicResponder(conn);
        }
        else
        {
            return new BasicResponder(conn);
        }

        
        
        
    }
};

int main(int argc, char** argv) {

    Logger::instance().add(new ConsoleChannel("debug", Level::Trace));
    //test::init();
  
        Application app;
        net::HttpServer socket("0.0.0.0", 8000 );
        socket.start();

        app.waitForShutdown([&](void*) {

            socket.shutdown();

        });

    
    /*
     
  
    /// for websocket we do not need responder
        Application app;
        net::HttpServer websocket("0.0.0.0", 8000  );
        websocket.start();

        app.waitForShutdown([&](void*) {

            websocket.shutdown();

        });
     */



    return 0;

    //  test::runAll();

    // return test::finalize();
}
