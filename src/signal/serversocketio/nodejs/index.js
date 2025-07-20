var app = require('express')();
var http = require('http').Server(app);
var io = require('socket.io')(http);

app.get('/', function(req, res){
    res.sendFile(__dirname + '/chat.html');
});

app.get('/loop', function(req, res) {
     res.sendFile(__dirname + '/loop.html');
});


io.on('connection', function(socket){
    console.log('a user connected');
    socket.on('joined', function(data) {
        console.log(data);
       
        console.log("connected: " + socket.client.conn.server.clientsCount  + " id: " + socket.id);

        socket.emit('acknowledge', socket.client.conn.server.clientsCount );
    });
    socket.on('chat message', function(msg){
        console.log('message: ' + msg);
        socket.emit('response message', msg + '  from server');
        //socket.broadcast.emit('response message', msg + '  from server');
    });


     socket.on('disconnect', function() {
      console.log("disconnected: " + socket.client.conn.server.clientsCount + " id: " + socket.id);
     });



/***********************Acknowletement testing end*************************************/

});

http.listen(8080, function(){
    console.log('listening on *:8080');
});




/*****************nsp tesgting****************************************/

const nsp = io.of('/my-namespace');
nsp.on('connection', function(socket){
nsp.emit('hi', 'hi everyone of nsp!');
  console.log('someone connected at nsp');
});


nsp.on('new message', (data) => {
	console.log(data);
  
  });
/*********************************************************/

