var app = require('express')();
var http = require('http').Server(app);
var io = require('socket.io')(http);

app.get('/', function(req, res){
    res.sendFile(__dirname + '/chat.html');
});

io.on('connection', function(socket){
    console.log('a user connected');
    socket.on('joined', function(data) {
        console.log(data);
        socket.emit('acknowledge', 'Acknowledged');
    });
    socket.on('chat message', function(msg){
        console.log('message: ' + msg);
        socket.emit('response message', msg + '  from server');
        //socket.broadcast.emit('response message', msg + '  from server');
    });


/**********************Acknowletement testing*************************************/
 console.log("ack");
   socket.emit('ferret', 'tobi', function (data) {
       console.log("ack"); // data will be 'woot'
       console.log(data); // data will be 'woot'
	});

  socket.on('ferret', function (name, fn) {
    fn(name + ' says woot');
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

