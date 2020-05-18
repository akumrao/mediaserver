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

    //  listenIps = config.webRtcTransport.listenIps;
      //const ip = listenIps.announcedIp || listenIps.ip;
     // console.log('listen ips ' + JSON.stringify(listenIps, null, 4) );

      resolve();
    });
  });
}

async function runSocketServer() {

    console.error('runSocketServer');


   io = socketIO.listen(webServer);



  io.sockets.on('connection', function(socket) {

	// convenience function to log server messages on the client
	function log() {
		var array = ['Message from server:'];
		 array.push.apply(array, arguments);
		 socket.emit('log', array);
		 console.log(array);
	}


	socket.on('disconnect', function() {
	  console.log("disconnect " + socket.id);
	  if( socket.id == serverSocketid)
	  {
	  	serverSocketid = null;
	  	console.log(serverSocketid);
	  }
	  else
	  {
	  	if( serverSocketid &&  io.sockets.connected[serverSocketid] &&  roomid)
	  	io.sockets.connected[serverSocketid].emit('disconnectClient', roomid, socket.id);
	  }


	});




	socket.on('CreateSFU', function() {
		log('Received request to create CreateSFU');
		
		if (serverSocketid !== null && io.sockets.connected[serverSocketid] ) {
			io.sockets.connected[serverSocketid].disconnect();
			serverSocketid =  null;
		}
		 
		log('SFU ID ' , socket.id) ;
	
		//roomid = room;
		serverSocketid = socket.id;


	});

	socket.on('create or join', function(room) {

		log('Received request to create or join room ' + room);

		if( serverSocketid == null || io.sockets.connected[serverSocketid] == null)
		 return  console.error("SFU server is down");	

		var clientsInRoom = io.sockets.adapter.rooms[room];
		var numClients = clientsInRoom ? Object.keys(clientsInRoom.sockets).length : 0;
		log('Room ' + room + ' now has ' + numClients + ' client(s)');


		socket.join(room);
		if (numClients === 0 ) {
		 
			log('Client ID ' + socket.id + ' created room ' + room);
		
			roomid = room;

			io.sockets.connected[serverSocketid].emit('created', room, serverSocketid, function (data) {

			console.log("ack"); // data will be 'woot'
			console.log( JSON.stringify(data, null, 4)); // data will be 'woot'
			});


			socket.emit('created', room, socket.id, function (data) {

			console.log("ack"); // data will be 'woot'
			console.log( JSON.stringify(data, null, 4)); // data will be 'woot'
			});

			socket.emit('joined', room, socket.id);

		} else if (numClients ) {
			log('Client ID ' + socket.id + ' joined room ' + room);
			io.sockets.in(room).emit('join', room, socket.id);

			socket.emit('joined', room, socket.id);
			io.sockets.in(room).emit('ready');
		} 

	});




//////////////////////////////////////////////////////////////////////////
	socket.on('message', function(message) {

		//console.log('notification ' + JSON.stringify(data, null, 4) );






	 	if ('to' in message) {
			socket.to(message.to).emit('message', message);
		}
		else
		{
			if(message.type ==="subscribe")
			{
				var clientsInRoom = io.sockets.adapter.rooms[message.room];
				var numClients = clientsInRoom ? Object.keys(clientsInRoom.sockets).length : 0;

				if(numClients ===1)
					return;

				console.log("Number of participant " + numClients );
				let objCopy = Object.assign({}, message);
				//var revMessage = message.clone();
				let revClient = [];
				revClient.push(message.from);




				let clients = [];

				console.log("message.from "+ message.from);
				for( const member in clientsInRoom.sockets ) {

					if( member !==  message.from ) {

						console.log("member "+ member);

						clients.push(member);

						objCopy.from = member;
						objCopy.desc = revClient;
						// console.log('app revMessage: ', objCopy);
						// io.sockets.connected[serverSocketid].emit('message', objCopy);

					}
				}


				message.desc = clients;

			//	return;
			}

			console.log('app message: ', message);

			io.sockets.connected[serverSocketid].emit('message', message);
		}


    // for a real app, would be room-only (not broadcast)
    	

	});


	socket.on('postAppMessage', function(message) {

		console.log('notification ' + JSON.stringify(message, null, 4) );

		if ('to' in message) {
			socket.to(message.to).emit('message', message);
		}

	});



	socket.on('bye', function(){
		console.log('received bye');
	});

});


}
