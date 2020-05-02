'use strict';

var os = require('os');
const fs = require('fs');
var nodeStatic = require('node-static');
var https = require('https');
var socketIO = require('socket.io');
//const ortc = require('./ortc.js');
const config = require('./config');
const express = require('express');

//const uuidV4 = require('uuid/v4');
////import { v4 as uuidV4 } from 'uuid';
const { v4: uuidV4 } = require('uuid');


let webServer;
let socketServer;
let expressApp;
let io;

(async () => {
  try {
    await runExpressApp();
    await runWebServer();
    await runSocketServer();
  } catch (err) {
    console.error(err);
  }
})();



console.log("https://localhost:8080/");

var roomid;
let serverSocketid =null;
var transport;  //producertranspor
var consumerTransport;
var routerCapabilities;
var producer;
var consumer;

var listenIps;

var fileServer = new(nodeStatic.Server)();
// var app = http.createServer(function(req, res) {
// 	fileServer.serve(req, res);
// }).listen(8080);

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

  const { sslKey, sslCrt } = config;
  if (!fs.existsSync(sslKey) || !fs.existsSync(sslCrt)) {
    console.error('SSL files are not found. check your config.js file');
    process.exit(0);
  }
  const tls = {
    cert: fs.readFileSync(sslCrt),
    key: fs.readFileSync(sslKey),
  };
  webServer = https.createServer(tls, expressApp);
  webServer.on('error', (err) => {
    console.error('starting web server failed:', err.message);
  });

  
  await new Promise((resolve) => {
    const { listenIp, listenPort } = config;
    webServer.listen(listenPort, listenIp, () => {
      console.log('server is running');
      console.log(`open https://127.0.0.1:${listenPort} in your web browser`);

      listenIps = config.webRtcTransport.listenIps;
      //const ip = listenIps.announcedIp || listenIps.ip;
      console.log('listen ips ' + JSON.stringify(listenIps, null, 4) );

      resolve();
    });
  });
}

async function runSocketServer() {

    console.error('runSocketServer');

  // socketServer = socketIO(webServer, {
  //   serveClient: false,
  //   path: '/server',
  //   log: false,
  // });

   io = socketIO.listen(webServer);



  io.sockets.on('connection', function(socket) {

	// convenience function to log server messages on the client
	function log() {
		// var array = ['Message from server:'];
		// array.push.apply(array, arguments);
		// socket.emit('log', array);
		// console.log(array);
	}
	// socket.on('message', function(message) {
	// 	log('Client said: ', message);
	// 	// for a real app, would be room-only (not broadcast)
	// 	socket.broadcast.emit('message', message);
	// });

	socket.on('disconnect', function() {
	  console.log(socket.id);
	  if( socket.id == serverSocketid)
	  {
	  	serverSocketid = null;
	  	console.log(serverSocketid);
	  }

	});


	socket.on('create or join', function(room) {
		log('Received request to create or join room ' + room);

		var clientsInRoom = io.sockets.adapter.rooms[room];
		var numClients = clientsInRoom ? Object.keys(clientsInRoom.sockets).length : 0;
		log('Room ' + room + ' now has ' + numClients + ' client(s)');

		socket.join(room);
		if (numClients === 0 || serverSocketid == null) {
		 
			log('Client ID ' + socket.id + ' created room ' + room);


			var rut = {"id":1,"method":"worker.createRouter","internal":{"routerId":"2e32062d-f04a-4c2d-a656-b586e50498ef"}};

			roomid = room;
			serverSocketid = socket.id;
			console.log('serverSocketid');
			console.log(serverSocketid);

			socket.emit('created', room, socket.id, rut, function (data) {

			console.log("ack"); // data will be 'woot'
			console.log( JSON.stringify(data, null, 4)); // data will be 'woot'
			}

			);

		} else if (numClients === 1) {
			log('Client ID ' + socket.id + ' joined room ' + room);
			io.sockets.in(room).emit('join', room);

			socket.emit('joined', room, socket.id);
			io.sockets.in(room).emit('ready');
		} else { // max two clients
			socket.emit('full', room);

			log('Client ID ' + socket.id + ' joined room ' + room);
			io.sockets.in(room).emit('join', room);

			socket.emit('joined', room, socket.id);
			io.sockets.in(room).emit('ready')
			
		}


	});



 //    socket.on('connectConsumerTransport', async (data, callback) => {

 //      console.log('connectConsumerTransport ' + JSON.stringify(data, null, 4) );

 //      await consumerTransport.connect({ dtlsParameters: data.dtlsParameters });
 //      callback();
 //    });

///////////////////////////////////////////////////////////////////////////

//  function createConsumer(producer, rtpCapabilities) {

  
// }


//////////////////////////////////////////////////////////////////////////
	socket.on('message', function(message) {

		//console.log('notification ' + JSON.stringify(data, null, 4) );


	 console.log('app message: ', message);
    // for a real app, would be room-only (not broadcast)
    	socket.broadcast.emit('message', message);

	});


	socket.on('postAppMessage', function(data) {

		console.log('notification ' + JSON.stringify(data, null, 4) );

	});


	// socket.on('ipaddr', function() {
	// 	var ifaces = os.networkInterfaces();
	// 	for (var dev in ifaces) {
	// 		ifaces[dev].forEach(function(details) {
	// 			if (details.family === 'IPv4' && details.address !== '127.0.0.1') {
	// 				socket.emit('ipaddr', details.address);
	// 			}
	// 		});
	// 	}
	// });

	socket.on('bye', function(){
		console.log('received bye');
	});

});


}
