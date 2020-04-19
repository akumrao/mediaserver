/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#ifndef HTTP_Tests_H
#define HTTP_Tests_H

/*



using std::cout;
using std::cerr;
using std::endl;
using base::test::Test;


#define TEST_HTTP_PORT 1337
#define TEST_HTTPS_PORT 1338


namespace base {


//
/// HTTP Client Tests
//

/// Initializes a polymorphic HTTP client connection for
/// testing callbacks, and also optionally raises the server.
struct HTTPEchoTest
{
    http::Server server;
    http::Client client;
    http::ClientConnection::Ptr conn;
    int numSuccess;
    int numWanted;

    HTTPEchoTest(int numWanted = 1)
        : server(net::Address("0.0.0.0", TEST_HTTP_PORT))
        , numSuccess(0)
        , numWanted(numWanted)
    {
    }

    void raiseServer()
    {
        server.start();
        server.Connection += [](http::ServerConnection::Ptr conn) {
            conn->Payload += [](http::ServerConnection& conn, const MutableBuffer& buffer) {
                conn.send(bufferCast<const char*>(buffer), buffer.size());
                // conn.close();
            };
        };
    }

    http::ClientConnection::Ptr createConnection(const std::string& protocol, const std::string& query)
    {
        std::ostringstream url;
        url << protocol << "://127.0.0.1:" << TEST_HTTP_PORT << query << std::endl;
        conn = client.createConnection(url.str());
        conn->Connect += slot(this, &HTTPEchoTest::onConnect);
        conn->Headers += slot(this, &HTTPEchoTest::onHeaders);
        conn->Payload += slot(this, &HTTPEchoTest::onPayload);
        conn->Complete += slot(this, &HTTPEchoTest::onComplete);
        conn->Close += slot(this, &HTTPEchoTest::onClose);
        return conn;
    }

    void start()
    {
        conn->send("PING", 4);
    }

    void shutdown()
    {
        // Stop the client and server to release the loop
        server.shutdown();
        client.shutdown();

        expect(numSuccess == numWanted);
    }

    void onConnect()
    {
        LDebug("On connect")
    }

    void onHeaders(http::Response& res)
    {
        LDebug("On headers")
    }

    void onComplete(const http::Response& res)
    {
        std::ostringstream os;
        res.write(os);
        LDebug("Response complete: ", os.str())
    }

    void onPayload(const MutableBuffer& buffer)
    {
        std::string data(bufferCast<const char*>(buffer), buffer.size());
        LDebug("On payload: ", buffer.size(), ": ", data)

        if (data == "PING")
            numSuccess++;

        if (numSuccess == numWanted) {
            shutdown();
        } else {
            conn->send("PING", 4);
        }
    }

    void onClose(http::Connection&)
    {
        LDebug("Connection closed")
        shutdown();
    }
};


} // namespace base

*/
#endif 
