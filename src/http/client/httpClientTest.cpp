#include "net/netInterface.h"
#include "httpClientTest.h"
#include "http/client.h"
#include "base/test.h"
#include "base/logger.h"
#include "base/application.h"
#include "base/platform.h"

#include "http/url.h"
#include "http/util.h"
#include "base/filesystem.h"
#include "http/HttpClient.h"
#include "http/HttpsClient.h"
#include "crypto/hash.h"
#include "base/platform.h"

#include "http/form.h"

using namespace base;
using namespace base::net;
using namespace base::test;

class Download : public Thread {
public:

    Download(std::string url);

    ~Download();


    void run();

    void stop(bool flag = true);

    URL _url;
    ClientConnecton *client{nullptr};

    Application app;

    uv_async_t async;
};

Download::Download(std::string url) : _url(url) {

    if (!client) {
        if (_url.scheme() == "http" || _url.scheme() == "ws") {
            client = new HttpClient(nullptr, _url, HTTP_RESPONSE);
        } else {
            LTrace("Only Http download is supported")
        }

    }

}

void Download::stop(bool flag) {

    LTrace(" Download::stop")

    int r = uv_async_send(&async);
    assert(r == 0);
}

Download::~Download() {

    LTrace("~Download()")

    join();
}

static void async_cb_download(uv_async_t* handle) {

   LTrace("async_cb_download");
    Download *p = ( Download *) handle->data;
   // uv_close((uv_handle_t*)&p->async, nullptr);

    p->client->Close();
    p->app.stop();
    p->join();
    p->app.uvDestroy();
    LTrace("async_cb_download over");


}

void Download::run() {

    LTrace("Download OnRun");

    ////////////////////////////////////

    std::string path("./");
    fs::addnode(path, "test.zip");


    //Client *conn = new Client("http://zlib.net/index.html");
    // client->start();
    client->fnComplete = [&](const Response & response) {
        std::cout << "client->fnComplete" << std::endl << std::flush;
        uv_close((uv_handle_t*)&async, nullptr);
        client->Close();
        //            app.stop()
    };

    client->fnUpdateProgess = [&](const std::string str) {
        std::cout << "final test " << str << std::endl << std::flush;
    };

    client->fnConnect = [&](HttpBase * conn) {
        ClientConnecton* con = (ClientConnecton*) conn;
        con->OutgoingProgress.start();
    };

    client->fnPayload = [&](HttpBase * conn, const char* data, size_t sz) {
        ClientConnecton* con = (ClientConnecton*) conn;
        con->OutgoingProgress.update(sz, con);
    };
    
    
    client->fnClose = [&](HttpBase * con, std::string string) {
        LTrace("fnClose")
        uv_close((uv_handle_t*)&async, nullptr);
    };

    client->_request.setMethod("GET");
    client->_request.setKeepAlive(false);
    client->setReadStream(new std::ofstream(path, std::ios_base::out | std::ios_base::binary));
    client->send();

    async.data = this;
    int r = uv_async_init(app.uvGetLoop(), &async, async_cb_download);
    assert(r == 0);
    app.run();
    LTrace("run Over");

    // expects(fs::exists(path));
    //expects(crypto::checksum("MD5", path) == "44d667c142d7cda120332623eab69f40");
    // fs::unlink(path);

    // std::cout << "app.run() over " << std::endl << std::flush;
    //  stop();

    //    exit = true;

    delete client;
    client = nullptr;

    LTrace("Download Over");
}

/*****************************************************************************************/

class Upload : public Thread {
public:

    Upload(std::string url);

    ~Upload();


    void run();
    void stop(bool flag = true);
    URL _url;
    ClientConnecton *client{nullptr};

    Application app;
    FormWriter *form;

    uv_async_t async;
};

Upload::Upload(std::string url) : _url(url) {

    if (!client) {
        if (_url.scheme() == "http" || _url.scheme() == "ws") {
            client = new HttpClient(nullptr, _url, HTTP_RESPONSE);
            //client->shouldSendHeader(false);
        } else {
            LTrace("Only Http Upload is supported")
        }

    }

}

void Upload::stop(bool flag) {

 LTrace(" Upload::stop")

    if (client) {

        form->stop(true);
        form->join();
        //app.stopAsync();
        int  r = uv_async_send(&async);
        assert(r == 0);
    }

    LTrace("Upload::stop over");

}

Upload::~Upload() {
    LTrace("~Upload()")
    join();
}

static void async_cb_upload(uv_async_t* handle) {

 
    LTrace(" Upload::async_cb_upload")

    Upload *p = ( Upload *) handle->data;
    uv_close((uv_handle_t*)&p->async, nullptr);

    p->client->Close();
    p->app.stop();

    p->join();
    p->app.uvDestroy();

    LTrace(" Upload::async_cb_upload over")



}

void Upload::run() {

    LTrace("Upload OnRun");

    ////////////////////////////////////

    //std::string accessToken("yy.1.AADtNcmfp6VvAQ6asnYqbDi5CKEfzwL7lfNqtbUiLeL4v07b_I");
    //std::string metadata("{ \"title\": \"My File\" }");


    // std::string path("/tmp/");
    //fs::addnode(path, "zlib-1.2.8.tar.gz");

    //Client *conn = new Client("http://zlib.net/index.html");
    // client->start();


    client->fnUpdateProgess = [&](const std::string str) {
        // std::cout << "final test " << str << std::endl << std::flush;
    };


    client->_request.setMethod("PUT");
    // client->_request.add( "Expect", "100-continue");
    client->_request.add("Accept", "*/*");
    client->_request.setKeepAlive(true);

    //for multipart
    /* 
    client->_request.setChunkedTransferEncoding(false);
    auto form = FormWriter::create(client, FormWriter::ENCODING_MULTIPART_FORM);
    form->addPart("metadata", new StringPart(metadata, "application/json; charset=UTF-8"));
    form->addPart("file", new FilePart("/var/tmp/a.txt", "text/plain"));
     */

    // for chunked
    client->_request.setChunkedTransferEncoding(true);
    //auto form = FormWriter::create(client, FormWriter::TEXT_PLAIN);
    //form->addPart("file", new FilePart("/var/tmp/a.txt", "text/plain"));
    form = FormWriter::create(client, FormWriter::APPLICATION_ZIP);
    form->addPart("file", new StringPart(std::string(FILE_CHUNK_SIZE * 1024, '*'), FormWriter::APPLICATION_ZIP));


    form->header();

    client->fnConnect = [&](HttpBase * con) {
        LTrace("fnConnect")
        form->start();
    };

    client->fnClose = [&](HttpBase * con, std::string string) {
        LTrace("fnClose")
        uv_close((uv_handle_t*)&async, nullptr);
    };




    client->fnFormClose = [&](ClientConnecton * con) {
         int  r = uv_async_send(&async);
    };




    client->fnComplete = [&](const Response & response) {
        std::cout << "client->fnComplete" << std::endl << std::flush;
        //form->condWait.signal();
    };

    client->fnPayload = [&](HttpBase * con, const char* data, size_t sz) {

    };


    client->send();

    async.data = this;
    int r = uv_async_init(app.uvGetLoop(), &async, async_cb_upload);
    assert(r == 0);
    app.run();

    std::cout << "app.run() over " << std::endl << std::flush;

    if (form)
        delete form;

    // expects(fs::exists(path));
    //expects(crypto::checksum("MD5", path) == "44d667c142d7cda120332623eab69f40");
    // fs::unlink(path);


    //  stop();

    //    exit = true;


    delete client;
    client = nullptr;

    LTrace("Upload Over");
}

/*********************************************************************************************/

/***********************************************************************************************/
int main(int argc, char** argv) {

    //Logger::instance().add(new RemoteChannel("Remote", Level::Remote, "127.0.0.1", 6000));

    Logger::instance().add(new ConsoleChannel("Trace", Level::Trace));
    /*
    {
       Application app;
       
       std::string path("./");
       fs::addnode(path, "test.html");
       
       ClientConnecton *conn = new HttpsClient("https://www.google.com/");

        // conn->Complete += sdelegate(&context, &CallbackContext::onClientConnectionComplete);
        conn->fnComplete = [&](const Response & response) {
             std::cout << "Lerver response:";
        };
        conn->_request.setKeepAlive(false);
        // conn->setReadStream(new std::stringstream);
        
        conn->_request.setMethod("GET");
        conn->_request.setKeepAlive(false);
        conn->setReadStream(new std::ofstream(path, std::ios_base::out | std::ios_base::binary));
        
        conn->send();
        app.run();
      //  expects(conn->closed());
      //  expects(!conn->eerror().any());
        
       
        return 0;
        
    }
    
    {

        Application app;

        ClientConnecton *conn = new HttpsClient("wss", "arvindubuntu", 1338, "/websocket");


        conn->fnConnect = [&](HttpBase * con) {
            conn->send("Ping");

        };

        conn->fnPayload = [&](HttpBase * con, const char* data, size_t sz) {
            std::cout << "client->fnPayload" << data << std::endl << std::flush;
        };

        //       conn->clientConn->_request.setMethod("GET");
        //        conn->clientConn->_request.setKeepAlive(false);
        //        conn->clientConn->setReadStream(new std::ofstream(path, std::ios_base::out | std::ios_base::binary));

        conn->send();


        app.run();
        
       
        return 0;
        
    }
*/
    
    
    
    LTrace("Download")
    {
          Download *download = new Download("http://speedtest.tele2.net/20MB.zip");

          download->start();

          base::sleep(55000);

          //base::sleep(5000);

          LTrace("download stop ")

          download->stop();

          delete download;

          LTrace("Download done");
          

      }

         LTrace("Upload start");

        {
         //Upload *upload = new Upload("http://arvindubuntu:8000/upload.php");
         Upload *upload = new Upload("http://speedtest.tele2.net/upload.php");

         upload->start();

         base::sleep(100000);

         LTrace("upload stop")

         upload->stop();

         delete upload;

         LTrace("upload done");

         // base::sleep(91000000000000);

         return 0;
     }
          


            // Logger::instance().setWriter(new AsyncLogWriter());

            //  test::init();


 {
        Application app;

        ClientConnecton *conn = new HttpClient("ws", "arvindubuntu", 8000, "/websocket");


        conn->fnConnect = [&](HttpBase * con) {
            conn->send("Ping");

        };

        conn->fnPayload = [&](HttpBase * con, const char* data, size_t sz) {
            std::cout << "client->fnPayload" << data << std::endl << std::flush;
        };

        //       conn->clientConn->_request.setMethod("GET");
        //        conn->clientConn->_request.setKeepAlive(false);
        //        conn->clientConn->setReadStream(new std::ofstream(path, std::ios_base::out | std::ios_base::binary));

        conn->send();


        app.run();
    }


    return 0;

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
    conn->start();class Download : public Thread {
    public:

Download(std::string url);

~Download();

// Download(){};
// virtual ~Thread2(void);

void run();

void stop(bool flag = true);


std::string url;

Client *conn={nullptr};

    };

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
        std::string path("/tmp/");
        fs::addnode(path, "zlib-1.2.8.tar.gz");

        ClientConnecton *conn = new HttpClient("http://zlib.net/fossils/zlib-1.2.8.tar.gz");
        //Client *conn = new Client("http://zlib.net/index.html");
        conn->fnComplete = [&](const Response & response) {
            std::cout << "Lerver response:";
        };

        conn->fnUpdateProgess = [&](const std::string str) {
            std::cout << "final test " << str << std::endl << std::flush;
        };

        conn->_request.setMethod("GET");
        conn->_request.setKeepAlive(false);
        conn->setReadStream(new std::ofstream(path, std::ios_base::out | std::ios_base::binary));
        conn->send();

        app.run();

        expects(fs::exists(path));
        expects(crypto::checksum("MD5", path) == "44d667c142d7cda120332623eab69f40");
        fs::unlink(path);


    });


    test::runAll();

    return test::finalize();
}
