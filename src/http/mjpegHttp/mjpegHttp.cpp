
#include "base/test.h"
#include "base/logger.h"
#include "base/application.h"

#include "http/HttpServer.h"
#include "http/packetizers.h"
#include "base/queue.h"


using namespace std;
using namespace base;
using namespace base::net;
using namespace base::test;

#include "opencv2/opencv.hpp"
/// apt-get install libopencv-dev

using namespace cv;

RunnableQueue< std::vector<uchar> > test1(20);

class MediaCapture : public Thread {
public:

    MediaCapture(std::string file = "") : file(file) {

    }

    virtual ~MediaCapture(void) {
        cap.release();
    };

    void run() {

        cap.open(0);

        if (!cap.isOpened()) {
            printf("no cam found ;(.\n");
            cap.release();
            abort();
        }

        //cap >> frame;
        //test.write(frame);
        //frame.release();
        //test.start();

        while (cap.isOpened() && !stopped()) {
            cap >> frame;
            std::vector<uchar> *outbuf = new std::vector<uchar>;
            std::vector<int> params;
            params.push_back(CV_IMWRITE_JPEG_QUALITY);
            params.push_back(50);
            if (frame.data) {
                imencode(".jpg", frame, *outbuf, params);

                int outlen = outbuf->size();

                cout << "Cam buffer len " << outlen << std::endl << std::flush;

                test1.push(outbuf);
            }

            base::sleep(1);

            // std::vector<uchar> outbuf1 = test1.popNext();

            frame.release();

        }

    }

    Mat frame;
    VideoCapture cap;
    std::string file;
};

MediaCapture* gVideoCapture;

class BasicResponder : public net::ServerResponder
/// Basic server responder (make echo?)
{
public:

    BasicResponder(net::TcpHTTPConnection* conn) :
    ServerResponder(conn) {
        LDebug("Starting Server responder.")
                //test1.ondispatch = BasicResponder::onVideoEncoded;
                // test1.start();
        if (!gVideoCapture->running()) {
            gVideoCapture->start();
        }
    }

    virtual void onClose() {
        
        LDebug("ServerResponder::On close")
        //stream.emitter -= packetSlot(this, &MPEGResponder::onVideoEncoded);
        test1.stop();
        gVideoCapture->stop();
        // delete gVideoCapture;
        //stream.stop();
    }

    void onRequest(net::Request& request, net::Response& response) {
        //DebugL << "On complete" << endl;

        connection()->shouldSendHeader(false);

        // response.setContentLength(14); // headers will be auto flushed

        //connection()->Send((const uint8_t *) "hello universe", 14);
        //connection()->Close();

        packetizer = new MultipartAdapter("image/jpeg",connection(), false);
        //stream.attach(packetizer, 10, true);

        using namespace std::placeholders;
        test1.ondispatch = std::bind(&MultipartAdapter::process, packetizer, _1);
        
        test1.start();


    }

    void onVideoEncoded(std::vector<uchar> & packet) {
        LOG_CALL;
        SDebug << "Send packet: "
                // assert(!connection().socket()->closed());
                << packet.size() << ": " << "fpsCounter.fps" << std::endl;

        /*   try
           {
               connection()->Send((const uint8_t *) packet.data(), packet.size());
               //fpsCounter.tick();
           } catch (std::exception& exc)
           {
               LError(exc.what())
               connection()->Close();
           }
         */
    }

    MultipartAdapter* packetizer;
};

class StreamingResponderFactory : public ServerConnectionFactory {
public:

    ServerResponder* createResponder(net::TcpHTTPConnection* conn) {
        return new BasicResponder(conn);
    }
};

int main(int argc, char** argv) {
    Logger::instance().add(new ConsoleChannel("debug", Level::Trace));
    //test::init();

    gVideoCapture = new MediaCapture();
    //     gAVVideoCapture->openDevice(0);
    //  gVideoCapture->openFile(VIDEO_FILE_SOURCE);
    gVideoCapture->start();



    Application app;
    net::HttpServer socket("0.0.0.0", 7000, new StreamingResponderFactory);
    socket.start();

    //base::sleep(45000);


    app.waitForShutdown([&](void*) {

        gVideoCapture->stop();
        delete gVideoCapture;

        socket.shutdown();

    });


    return 0;

    //  test::runAll();

    // return test::finalize();
}
