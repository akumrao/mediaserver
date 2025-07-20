
'use strict';

var os = require('os');
const fs = require('fs');
var nodeStatic = require('node-static');
var http = require('http');
const config = require('./config');
const express = require('express');
//var WebSocketServer = require('websocket').server;
var WebSocketServer = require("ws").Server;

const {
    v4: uuidV4
} = require('uuid');

let webServer;
let socketServer;
let expressApp;
let wsServer;

(async () => {
    try {
        await runExpressApp();
        await runWebServer();
        await runSocketServer();
    } catch (err) {
        console.error(err);
    }
})();

console.log("To browse https://localhost/ or https://ip/");

let serverSocketid = null;

var fileServer = new(nodeStatic.Server)();

async function runExpressApp() {
    expressApp = express();
    expressApp.use(express.json());
    expressApp.use(express.static(__dirname));

    expressApp.use((error, req, res, next) => {
        if (error) {
            console.log('Express app error,', error.message);

            error.status = error.status || (error.name === 'TypeError' ? 400 : 500);
            res.statusMessage = error.message;
            res.status(error.status).send(String(error));
        } else {
            next();
        }
    });
}

async function runWebServer() {
    console.error('runWebServer');

    const {
        sslKey,
        sslCrt
    } = config;
    if (!fs.existsSync(sslKey) || !fs.existsSync(sslCrt)) {
        console.error('SSL files are not found. check your config.js file');
        process.exit(0);
    }
    const tls = {
        cert: fs.readFileSync(sslCrt),
        key: fs.readFileSync(sslKey),
    };
    
    //webServer = http.createServer(tls, expressApp);
    webServer = http.createServer(expressApp); // for http
    webServer.on('error', (err) => {
        console.error('starting web server failed:', err.message);
    });

    await new Promise((resolve) => {
        const {
            listenIp,
            listenPort
        } = config;
        webServer.listen(listenPort, listenIp, () => {
            console.log('server is running');
            console.log(`open http://127.0.0.1:${listenPort} in your web browser`);
            resolve();
        });
    });
}

async function runSocketServer() {
    console.error('runSocketServer');
   // io = socketIO.listen(webServer);


        // wsServer = new WebSocketServer({
        //     httpServer: webServer,
        //     // You should not use autoAcceptConnections for production 
        //     // applications, as it defeats all standard cross-origin protection 
        //     // facilities built into the protocol and the browser.  You should 
        //     // *always* verify the connection's origin and decide whether or not 
        //     // to accept it. 
        //     autoAcceptConnections: false
        // });

        wsServer =  new WebSocketServer({server: webServer});
         
        var numConnections = 0;

        wsServer.on('connection', function (ws, req) {

          numConnections++;

          ws.on('message', function (message) {
            console.log('received: %s', message + " id: " + numConnections);
          })



            ws.on('close', function(code, reason) {
            console.log("streamer disconnected  id: " + numConnections);
            numConnections--;
        });

        ws.on('error', function(error) {
            console.error(`streamer connection error: ${error}`);
            ws.close(1006 /* abnormal closure */, error);

        });


         ws.send(numConnections);
          
        })

}


















