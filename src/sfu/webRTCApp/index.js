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
	   

	  if( socket.id == serverSocketid)
	  {
	  	
	  	console.log("server down " + serverSocketid);
	  	serverSocketid = null;

	  	for( let soc in io.sockets.connected ){
	  		io.sockets.connected[soc].emit('leave', socket.room, -1, -1);	
	  		io.sockets.connected[soc].disconnect();
		}

	  }
	  else
	  {

	  	  var clientsInRoom = io.sockets.adapter.rooms[socket.room];
	  	  var numClients = clientsInRoom ? Object.keys(clientsInRoom.sockets).length : 0;
	  	  console.log("disconnect " + socket.id + " from room " + socket.room + " numClients " + numClients);

	  	  ////////////////////////////////////////////////////////////////////
		  if( serverSocketid &&  io.sockets.connected[serverSocketid])
			  io.sockets.connected[serverSocketid].emit('disconnectClient', socket.id);

		  io.sockets.to(socket.room).emit('leave', socket.room, socket.id, numClients);	

		  console.log("unsubscribe " + socket.id);


		  // var room_list = {};
		  // var rooms = socket.adapter.rooms;
		  //
		  // for (var room in rooms){
			//   if (!rooms[room].hasOwnProperty(room)) {
			// 	  console.log(rooms[room]);
			// 	  room_list[room] = Object.keys(rooms[room]).length;
			//   }
		  // }
		  // console.log(room_list);


		  // var rooms = socket.adapter.rooms;
		  //
		  // for(var room in rooms  ) {
		  //
			//   if( Object.keys(rooms[room].sockets)[0] == room )
			// 	  continue;
		  //
			//   var clientsInRoom = io.sockets.adapter.rooms[room];
			//   var numClients = clientsInRoom ? Object.keys(clientsInRoom.sockets).length : 0;
		  //
		  //
		  //
			//   console.log("Number of participant " + numClients + " for room " + room);
			//   var message = {
			// 	  room: room,
			// 	  type: "subscribe"
			//   };
			//   var revClient = [];
			//   revClient.push(socket.id);
		  //
			//   for (const member in clientsInRoom.sockets) {
		  //
			// 	  if (member !== message.from) {
			// 		  message.from = member;
			// 		  message.desc = revClient;
			// 		  console.log('unsubscribe: ', message);
			// 		  io.sockets.connected[serverSocketid].emit('message', message);
		  //
			// 	  }
			//   }
		  // }



		  ///////////////////////////////////////////////////////////////////

	  }





	});




	socket.on('CreateSFU', function() {
		log('Received request to create CreateSFU');
		
		if (serverSocketid !== null && io.sockets.connected[serverSocketid] ) {
			io.sockets.connected[serverSocketid].disconnect();
			serverSocketid =  null;
		}
		 
		log('SFU ID ' , socket.id) ;
	
		serverSocketid = socket.id;

	});

	socket.on('create or join', function(roomId, user) {

		log('Received request to create or join room ' + roomId);

		if( serverSocketid == null || io.sockets.connected[serverSocketid] == null)
		{
			io.emit('leave', socket.room, -1, -1);
		  	return  console.error("SFU server is down");
		}

		
		if(socket.room)
        	socket.leave(socket.room);

		socket.room = roomId;

		socket.join(roomId);

        	if(user)
    		socket.user = user;

		var numClients = io.sockets.adapter.rooms[roomId].length;  //For socket.io versions >= 1.4:

		log('Room ' + roomId + ' now has ' + numClients + ' client(s)');

		if (numClients === 1 ) {
		  
			log('Client ID ' + socket.id + ' created room ' + roomId);
		
			io.sockets.connected[serverSocketid].emit('created', roomId, serverSocketid);


			socket.emit('created', roomId, socket.id);

			socket.emit('joined', roomId, socket.id, numClients);

		} else if (numClients > 1 ) {
			log('Client ID ' + socket.id + ' joined room ' + roomId);
			io.sockets.in(roomId).emit('join', roomId, socket.id, numClients);

			socket.emit('joined', roomId, socket.id, numClients);
			io.sockets.in(roomId).emit('ready');
		} 

	});




	socket.on('message', function(message) {

		console.log('sfuserver message: ', message);

		message.from = socket.id;
		
		if(socket.user)
		message.user = socket.user;

		socket.to(message.to).emit('message', message);
	  });

//////////////////////////////////////////////////////////////////////////
	socket.on('sfu-message', function(message) {

		message.from = socket.id;

		if(message.type ==="subscribe")
		{
			console.log("subscribe");

			var clientsInRoom = io.sockets.adapter.rooms[message.room];
			var numClients = clientsInRoom ? Object.keys(clientsInRoom.sockets).length : 0;

			if(numClients < 1)
			{
				console.log("Number of participant " + numClients );
				return;
			}

			console.log("Number of participant " + numClients );
			let objCopy = Object.assign({}, message);
			//var revMessage = message.clone();
			let revClient = [];
			revClient.push(message.from);




			let clients = [];

			console.log("message.from "+ message.from);
			for( const member in clientsInRoom.sockets ) {

				if( member !==  message.from ) {

					//console.log("member "+ member);

					clients.push(member);

					objCopy.from = member;
					objCopy.desc = revClient;
					console.log('app revMessage: ', objCopy);
					//io.sockets.connected[serverSocketid].emit('message', objCopy);

				}
			}


			message.desc = clients;

			//return;
		}

		console.log('app message: ', message);
		if(io.sockets.connected[serverSocketid])
		io.sockets.connected[serverSocketid].emit('message', message);



	});


	socket.on('postAppMessage', function(message) {

		if(message.type ==="user")
		{
			message.user = message.desc;
		}

		console.log('notification ' + JSON.stringify(message, null, 4) );
		message.from = socket.id;
		
		if(socket.user)
		message.user = socket.user;

		if(message.type ==="chat")
		{
			if ('room' in message) {
				io.in(message.room).emit('message', message);
			}
		}
		else
		{
			if ('room' in message) {
				socket.to(message.room).emit('message', message);
			}

		}



	});



	socket.on('bye', function(){
		console.log('received bye');
	});

});


}
