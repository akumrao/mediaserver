'use strict';

var os = require('os');
var nodeStatic = require('node-static');
var http = require('http');
var socketIO = require('socket.io');

var fileServer = new(nodeStatic.Server)();
var app = http.createServer(function(req, res) {
  fileServer.serve(req, res);
}).listen(8080);



var io = socketIO.listen(app);
io.sockets.on('connection', function(socket) {

  // convenience function to log server messages on the client
  function log() {
    var array = ['Message from server:'];
    array.push.apply(array, arguments);
    socket.emit('log', array);
     console.log(array);
  }

  socket.on('message', function(message) {
    log('Client said: ', message);
    // for a real app, would be room-only (not broadcast)
    socket.broadcast.emit('message', message);
  });

  socket.on('create or join', function(room) {
    log('Received request to create or join room ' + room);

    var clientsInRoom = io.sockets.adapter.rooms[room];
    var numClients = clientsInRoom ? Object.keys(clientsInRoom.sockets).length : 0;
    log('Room ' + room + ' now has ' + numClients + ' client(s)');


     socket.join(room);

     var arr2 = [];

     var clients = io.sockets.adapter.rooms[room].sockets;   
      for (var clientId in clients ) {
           //this is the socket of each client in the room.
           var clientSocket = io.sockets.connected[clientId];

            var obj = { "clientid": clientSocket.id, "status":"connected"};
           
           //you can do whatever you need with this
           ///clientSocket.emit('new event', "Updates");
           arr2.push(obj);

           console.log(arr2); 
      }
 
    if (numClients === 0) {
     
      log('Client ID ' + socket.id + ' created room ' + room);
      socket.emit('created', room, socket.id, arr2);
    } else if (numClients <= 10) {
      log('Client ID ' + socket.id + ' joined room ' + room);
      // io.sockets.in(room).emit('join', room);
      socket.emit('joined', room, socket.id,arr2);
      io.sockets.in(room).emit('ready', room);
      socket.broadcast.emit('ready', room);
    } else { // max two clients
      socket.emit('full', room);
    }
  });

  socket.on('ipaddr', function() {
    var ifaces = os.networkInterfaces();
    for (var dev in ifaces) {
      ifaces[dev].forEach(function(details) {
        if (details.family === 'IPv4' && details.address !== '127.0.0.1') {
          socket.emit('ipaddr', details.address);
        }
      });
    }
  });

  socket.on('disconnect', function(reason) {
    console.log(`Peer or server disconnected. Reason: ${reason}.`);
    var obj = { "clientid": socket.id, "status":"connected"};
    socket.broadcast.emit('bye', obj);
  });

  socket.on('bye', function(room) {
    console.log(`Peer said bye on room ${room}.`);
  });
});
