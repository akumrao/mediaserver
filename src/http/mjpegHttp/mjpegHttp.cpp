#include "http/HttpServer.h"
#include "base/test.h"
#include "base/logger.h"
#include "base/application.h"

using namespace base;
using namespace base::net;
using namespace base::test;





class BasicResponder: public net::ServerResponder
	/// Basic server responder (make echo?)
{
public:
	BasicResponder(net::TcpHTTPConnection* conn) : 
		ServerResponder(conn)
	{
///		DebugL << "Creating" << endl;
	}

	void onRequest(net::Request& request, net::Response& response) 
	{
		//DebugL << "On complete" << endl;

		response.setContentLength(14);  // headers will be auto flushed

		connection()->Send((const uint8_t *) "hello universe", 14); 
		connection()->Close();
	}
};


class StreamingResponderFactory : public ServerConnectionFactory
{
public:
    ServerResponder* createResponder(net::TcpHTTPConnection* conn)
    {
        return new BasicResponder(conn);
    }
};



int main(int argc, char** argv) {
    Logger::instance().add(new ConsoleChannel("debug", Level::Trace));
    //test::init();

    Application app;
    net::HttpServer socket("0.0.0.0", 7000, new StreamingResponderFactory );
    socket.start();

    app.waitForShutdown([&](void*) {

        socket.shutdown();

    });


    return 0;
    
  //  test::runAll();

   // return test::finalize();
}
