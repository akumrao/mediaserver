# node-socket-example
Node Js Chat Socket IO Example
This is a sampe application to demonstrate a sample chat system using NodeJs socket.io programming. The complete explanation and walk through of the code is avilable on my blog - [Node JS Socket IO Chat Example](http://www.devglan.com/node-js/nodejs-chat-socket-io-example)

## Running the application
Assuming that node Js is already installed, traverse to the project location and execute the command - node index.js and hit the url (http://localhost:8080) to access the vhat application.






var WebSocketServer = require('websocket').server;
var http = require('http');

var server = http.createServer(function(request, response) {
    console.log((new Date()) + ' Received request for ' + request.url);
    response.writeHead(404);
    response.end();
});
server.listen(5050, function() {
    console.log((new Date()) + ' Server is listening on port 5050');
});

wsServer = new WebSocketServer({
    httpServer: server,
    // You should not use autoAcceptConnections for production 
    // applications, as it defeats all standard cross-origin protection 
    // facilities built into the protocol and the browser.  You should 
    // *always* verify the connection's origin and decide whether or not 
    // to accept it. 
    autoAcceptConnections: false
});

function originIsAllowed(origin) {
  // put logic here to detect whether the specified origin is allowed. 
  return true;
}

//create an array to hold your connections
var connections = [];

wsServer.on('request', function(request) {
    if (!originIsAllowed(request.origin)) {
      // Make sure we only accept requests from an allowed origin 
      request.reject();
      console.log((new Date()) + ' Connection from origin ' + request.origin + ' rejected.');
      return;
    }
    var connection = request.accept('echo-protocol', request.origin);

    //store the new connection in your array of connections
    connections.push(connection);
    
    console.log((new Date()) + ' Connection accepted.');
    connection.on('message', function(message) {
        if (message.type === 'utf8') {
            console.log('Received Message: ' + message.utf8Data);

            //send the received message to all of the 
            //connections in the connection array
            for(var i = 0; i < connections.length; i++) {
                connections[i].sendUTF(message.utf8Data);
            }
        }
        else if (message.type === 'binary') {
            console.log('Received Binary Message of ' + message.binaryData.length + ' bytes');
            connection.sendBytes(message.binaryData);
        }
    });
    connection.on('close', function(reasonCode, description) {
        console.log((new Date()) + ' Peer ' + connection.remoteAddress + ' disconnected.');
    });
});
 Jason 2665

E
