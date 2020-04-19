'use strict';

var os = require('os');
const fs = require('fs');
var nodeStatic = require('node-static');
var https = require('https');
var socketIO = require('socket.io');
const ortc = require('./ortc.js');
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
      console.warn('Express app error,', error.message);

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

	

	socket.on('getRouterRtpCapabilities', function (fn) {
		console.log("getRouterRtpCapabilities");
		routerCapabilities =  {"codecs":[{"kind":"audio","mimeType":"audio/opus","clockRate":48000,"channels":2,"rtcpFeedback":[{"type":"transport-cc"}],"preferredPayloadType":100,"parameters":{}},{"kind":"video","mimeType":"video/VP8","clockRate":90000,"rtcpFeedback":[{"type":"nack"},{"type":"nack","parameter":"pli"},{"type":"ccm","parameter":"fir"},{"type":"goog-remb"},{"type":"transport-cc"}],"preferredPayloadType":101,"parameters":{"x-google-start-bitrate":1000}},{"kind":"video","mimeType":"video/rtx","preferredPayloadType":102,"clockRate":90000,"rtcpFeedback":[],"parameters":{"apt":101}}],"headerExtensions":[{"kind":"audio","uri":"urn:ietf:params:rtp-hdrext:sdes:mid","preferredId":1,"preferredEncrypt":false,"direction":"recvonly"},{"kind":"video","uri":"urn:ietf:params:rtp-hdrext:sdes:mid","preferredId":1,"preferredEncrypt":false,"direction":"recvonly"},{"kind":"video","uri":"urn:ietf:params:rtp-hdrext:sdes:rtp-stream-id","preferredId":2,"preferredEncrypt":false,"direction":"recvonly"},{"kind":"video","uri":"urn:ietf:params:rtp-hdrext:sdes:repaired-rtp-stream-id","preferredId":3,"preferredEncrypt":false,"direction":"recvonly"},{"kind":"audio","uri":"http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time","preferredId":4,"preferredEncrypt":false,"direction":"sendrecv"},{"kind":"video","uri":"http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time","preferredId":4,"preferredEncrypt":false,"direction":"sendrecv"},{"kind":"audio","uri":"http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01","preferredId":5,"preferredEncrypt":false,"direction":"recvonly"},{"kind":"video","uri":"http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01","preferredId":5,"preferredEncrypt":false,"direction":"sendrecv"},{"kind":"video","uri":"http://tools.ietf.org/html/draft-ietf-avtext-framemarking-07","preferredId":6,"preferredEncrypt":false,"direction":"sendrecv"},{"kind":"video","uri":"urn:ietf:params:rtp-hdrext:framemarking","preferredId":7,"preferredEncrypt":false,"direction":"sendrecv"},{"kind":"audio","uri":"urn:ietf:params:rtp-hdrext:ssrc-audio-level","preferredId":10,"preferredEncrypt":false,"direction":"sendrecv"},{"kind":"video","uri":"urn:3gpp:video-orientation","preferredId":11,"preferredEncrypt":false,"direction":"sendrecv"},{"kind":"video","uri":"urn:ietf:params:rtp-hdrext:toffset","preferredId":12,"preferredEncrypt":false,"direction":"sendrecv"}],"fecMechanisms":[]}
			fn(routerCapabilities);
	});


	socket.on('createProducerTransport', function (data, fn) {

		console.log('createProducerTransport');

	 try {

			console.log('createProducerTransport ' + JSON.stringify(data, null, 4) );

			var trans = {"id":2,"method":"router.createWebRtcTransport","internal":{"routerId":"2e32062d-f04a-4c2d-a656-b586e50498ef","transportId":"e5302612-283c-4532-8acb-8f3cbb87a8a5"},"data":{"listenIps":[{"ip":"127.0.0.1"}],"enableUdp":false,"enableTcp":true,"preferUdp":false,"preferTcp":true,"initialAvailableOutgoingBitrate":1000000,"enableSctp":false,"numSctpStreams":{"OS":1024,"MIS":1024},"maxSctpMessageSize":262144,"isDataChannel":true}};
			
			if(!data.forceTcp)
			{
				trans.data.enableUdp =true;
				trans.data.preferUdp =true;
			}

			trans.internal.transportId = uuidV4();
			trans.data.listenIps=listenIps;

			console.log("transport " +  JSON.stringify(trans, null, 4));


			//if(!transport)
			{ 


					io.sockets.connected[serverSocketid].emit('createWebRtcTransport', roomid, socket.id, trans, function (data1) {
					console.log("ack"); // data will be 'woot'
					
					transport = trans.internal.transportId;
					console.log( JSON.stringify(transport, null, 4)); // data will be 'woot'

					console.log("transprot ack " + JSON.stringify(data1, null, 4)); // data will be 'woot'

////////////////////////////////////////////////////////////////////////////////////////

					var maxbitrate = {"id":3,"method":"transport.setMaxIncomingBitrate","internal":{"routerId":"2e32062d-f04a-4c2d-a656-b586e50498ef","transportId":"e5302612-283c-4532-8acb-8f3cbb87a8a5"},"data":{"bitrate":1500000}}
					maxbitrate.internal.transportId = transport;

					io.sockets.connected[serverSocketid].emit('rest', roomid, socket.id, maxbitrate, function (data2) {
					console.log("ack maxbitrate" ); // data will be 'woot'
					
					console.log( JSON.stringify(data2, null, 4)); // data will be 'woot'

//////////////////////////////////////////////////////////////////////////////////

					fn( 
							{
								id: data1.data.id,
								iceParameters: data1.data.iceParameters,
								iceCandidates: data1.data.iceCandidates,
								dtlsParameters: data1.data.dtlsParameters
							} 
						);

			});// maxbitrate

			});//webrtc transport

			}

		} catch (err) {
			console.error(err);
		 // callback({ error: err.message });
		}
	});


//
//////////////////////////////////////////////////////////////////////
	socket.on('connectProducerTransport', function (data, fn) {

	 console.log('connectProducerTransport');

	 try {

			//console.log('connectProducerTransport1 ' + JSON.stringify(data, null, 4) );

			var prodTras = {"id":4,"method":"transport.connect","internal":{"routerId":"2e32062d-f04a-4c2d-a656-b586e50498ef","transportId":"e5302612-283c-4532-8acb-8f3cbb87a8a5"},"data":{"dtlsParameters":{"role":"server","fingerprints":[{"algorithm":"sha-256","value":"30:D3:F2:7C:DB:12:F3:FD:D4:38:31:19:2F:48:B5:ED:85:59:85:99:D2:5C:E8:A5:AE:A2:57:C6:FF:93:57:65"}]}}};

			prodTras.internal.transportId = transport;

			//console.log('connectProducerTransport2 ' + JSON.stringify(prodTras, null, 4) );

			prodTras.data =data;

			console.log('connectProducerTransport3 ' + JSON.stringify(prodTras, null, 4) );

			io.sockets.connected[serverSocketid].emit('rest', roomid, socket.id, prodTras, function (data1) {
					console.log("ack prodTrasport"); // data will be 'woot'
					console.log( JSON.stringify(data1, null, 4)); // data will be 'woot'


					fn( );

			});// maxbitrate


			// const { transport, params } =  createWebRtcTransport();
			// producerTransport = transport;
			// fn(params);
		} catch (err) {
			console.error(err);
		 // callback({ error: err.message });
		}
	});
/////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
	socket.on('produce', function (data, fn) {

		console.log('produce');

	 try {

			console.log('produce1  data' + JSON.stringify(data, null, 4) );

			const {kind, rtpParameters} = data;
			const paused = false;

			const routerRtpCapabilities = routerCapabilities;
            // This may throw.
            const rtpMapping = ortc.getProducerRtpParametersMapping(rtpParameters, routerRtpCapabilities);
            // This may throw.
            const consumableRtpParameters = ortc.getConsumableRtpParameters(kind, rtpParameters, routerRtpCapabilities, rtpMapping);
          //  const internal = Object.assign(Object.assign({}, this._internal), { producerId: id || v4_1.default() });
            const reqData = { kind, rtpParameters, rtpMapping, paused };

            console.log('reqData ' + JSON.stringify(reqData, null, 4) );
           // console.log('internal ' + JSON.stringify(internal, null, 4) );


			var prod = {"id":5,"method":"transport.produce","internal":{"routerId":"2e32062d-f04a-4c2d-a656-b586e50498ef","transportId":"e5302612-283c-4532-8acb-8f3cbb87a8a5","producerId":"87c81cfc-b307-4270-a213-4f17e4776931"},"data":{"kind":"video","rtpParameters":{"mid":"0","codecs":[{"mimeType":"video/VP8","clockRate":90000,"payloadType":96,"rtcpFeedback":[{"type":"goog-remb"},{"type":"transport-cc"},{"type":"ccm","parameter":"fir"},{"type":"nack"},{"type":"nack","parameter":"pli"}],"parameters":{}},{"mimeType":"video/rtx","clockRate":90000,"payloadType":97,"rtcpFeedback":[],"parameters":{"apt":96}}],"headerExtensions":[{"uri":"urn:ietf:params:rtp-hdrext:sdes:mid","id":4},{"uri":"urn:ietf:params:rtp-hdrext:sdes:rtp-stream-id","id":5},{"uri":"urn:ietf:params:rtp-hdrext:sdes:repaired-rtp-stream-id","id":6},{"uri":"http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time","id":2},{"uri":"http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01","id":3},{"uri":"http://tools.ietf.org/html/draft-ietf-avtext-framemarking-07","id":8},{"uri":"urn:3gpp:video-orientation","id":13},{"uri":"urn:ietf:params:rtp-hdrext:toffset","id":14}],"encodings":[{"ssrc":6897981,"rtx":{"ssrc":3637419867}}],"rtcp":{"cname":"d1blHuW4nPAoUtQb"}},"rtpMapping":{"codecs":[{"payloadType":96,"mappedPayloadType":101},{"payloadType":97,"mappedPayloadType":102}],"encodings":[{"mappedSsrc":632915476,"ssrc":6897981}]},"paused":false}};
			//prod.internal 
			prod.internal.transportId = transport;
			prod.internal.producerId =  uuidV4();

			//console.log('produce2 ' + JSON.stringify(prod, null, 4) );

			//prod.data.kind = data.kind;
			prod.data = reqData;
			console.log('produce3 ' + JSON.stringify(prod, null, 4) );

			io.sockets.connected[serverSocketid].emit('rest', roomid, socket.id, prod, function (data1) {
					
					console.log( "ack producer ",  JSON.stringify(data1, null, 4)); // data will be 'woot'

					producer = {
						       id:prod.internal.producerId,
               				   kind,
                               rtpParameters,
                               type: data1.data.type,
                               consumableRtpParameters
                           };

					console.log( "producer ",  JSON.stringify(producer, null, 4)); // data will be 'woot'

					fn({ id: prod.internal.producerId});

			});// maxbitrate


			// const { transport, params } =  createWebRtcTransport();
			// producerTransport = transport;
			// fn(params);
		} catch (err) {
			console.error(err);
		 // callback({ error: err.message });
		}
	});
/////////////////////////////////////////////////////////////////////
socket.on('createConsumerTransport', function (data, fn) {

		console.log('createConsumerTransport');

	 try {

			console.log('createConsumerTransport data' + JSON.stringify(data, null, 4) );
			var trans = {"id":6,"method":"router.createWebRtcTransport","internal":{"routerId":"2e32062d-f04a-4c2d-a656-b586e50498ef","transportId":"4ca62904-639c-4b5a-86e8-f0bc84bfe776"},"data":{"listenIps":[{"ip":"127.0.0.1"}],"enableUdp":false,"enableTcp":true,"preferUdp":false,"preferTcp":true,"initialAvailableOutgoingBitrate":1000000,"enableSctp":false,"numSctpStreams":{"OS":1024,"MIS":1024},"maxSctpMessageSize":262144,"isDataChannel":true}}

			if(!data.forceTcp)
			{
				trans.data.enableUdp =true;
				trans.data.preferUdp =true;
			}
			
			trans.internal.transportId = uuidV4();
			trans.data.listenIps=listenIps;

			console.log("transport " +  JSON.stringify(trans, null, 4));

			//if(!transport)
			{ 


					io.sockets.connected[serverSocketid].emit('createWebRtcTransport', roomid, socket.id, trans, function (data1) {
					console.log("consumer transport create ack"); // data will be 'woot'
					
					consumerTransport = trans.internal.transportId;
					console.log( JSON.stringify(transport, null, 4)); // data will be 'woot'

					console.log("consumer transport create ack " + JSON.stringify(data1, null, 4)); // data will be 'woot'

////////////////////////////////////////////////////////////////////////////////////////
					var maxbitrate = {"id":7,"method":"transport.setMaxIncomingBitrate","internal":{"routerId":"2e32062d-f04a-4c2d-a656-b586e50498ef","transportId":"4ca62904-639c-4b5a-86e8-f0bc84bfe776"},"data":{"bitrate":1500000}}

					maxbitrate.internal.transportId = consumerTransport;

					io.sockets.connected[serverSocketid].emit('rest', roomid, socket.id, maxbitrate, function (data2) {
					console.log("consumer ack maxbitrate",  JSON.stringify(data2, null, 4) ); // data will be 'woot'
					
//////////////////////////////////////////////////////////////////////////////////
					fn( 
							{
								id: data1.data.id,
								iceParameters: data1.data.iceParameters,
								iceCandidates: data1.data.iceCandidates,
								dtlsParameters: data1.data.dtlsParameters
							} 
						);

			});// maxbitrate

			});//webrtc transport

			}
		} catch (err) {
			console.error(err);
		 // callback({ error: err.message });
		}
	});

 
	socket.on('connectConsumerTransport', function (data, fn) {

	 console.log('connectConsumerTransport');

	 try {

			console.log('connectConsumerTransport data ' + JSON.stringify(data, null, 4) );
			var prodTras =  {"id":9,"method":"transport.connect","internal":{"routerId":"2e32062d-f04a-4c2d-a656-b586e50498ef","transportId":"4ca62904-639c-4b5a-86e8-f0bc84bfe776"},"data":{"dtlsParameters":{"role":"client","fingerprints":[{"algorithm":"sha-256","value":"D0:75:50:E5:7E:B7:EB:5D:0D:A2:2B:C6:2E:E6:F0:20:66:2E:91:25:3D:3E:DF:F5:F1:0C:62:3A:9E:40:60:C0"}]}}};

			prodTras.internal.transportId = consumerTransport;

			//console.log('connectProducerTransport2 ' + JSON.stringify(prodTras, null, 4) );

			prodTras.data =data;

			console.log('connectProducerTransport3 ' + JSON.stringify(prodTras, null, 4) );

			io.sockets.connected[serverSocketid].emit('rest', roomid, socket.id, prodTras, function (data1) {
					console.log("ack prodTrasport"); // data will be 'woot'
					console.log( JSON.stringify(data1, null, 4)); // data will be 'woot'

					fn( );

			});// maxbitrate


			// const { transport, params } =  createWebRtcTransport();
			// producerTransport = transport;
			// fn(params);
		} catch (err) {
			console.error(err);
		 // callback({ error: err.message });
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
    socket.on('consume', function (data, fn) {

      console.log('consume mediaCodecs ' + JSON.stringify(data, null, 4) );

///////////////////////////////////////////////////////////////////////////

		try 
		{
	        if( ! ortc.canConsume(producer.consumableRtpParameters, data.rtpCapabilities))
	        {
	        	 console.error('can not consume');
	    		 return;
	        }
	    }
	    catch (error) {
	        console.error('canConsume() | unexpected error: %s', String(error));
	        return false;
	    }



	  	try {

		    const rtpParameters = ortc.getConsumerRtpParameters(producer.consumableRtpParameters, data.rtpCapabilities);
	      //  const internal = Object.assign(Object.assign({}, this._internal), { consumerId: v4_1.default(), producerId });
	        const paused = producer.kind === 'video';

	        const reqData = {
	            kind: producer.kind,
	            rtpParameters,
	            type: producer.type,
	            consumableRtpEncodings: producer.consumableRtpParameters.encodings,
	            paused            
	        };

	        var consumeData=  {"id":8,"method":"transport.consume","internal":{"routerId":"2e32062d-f04a-4c2d-a656-b586e50498ef","transportId":"4ca62904-639c-4b5a-86e8-f0bc84bfe776","consumerId":"c3dbd428-ff6e-46b6-a1e7-ba3891b70f34","producerId":"87c81cfc-b307-4270-a213-4f17e4776931"},"data":{"kind":"video","rtpParameters":{"codecs":[{"mimeType":"video/VP8","clockRate":90000,"payloadType":101,"rtcpFeedback":[{"type":"transport-cc"},{"type":"ccm","parameter":"fir"},{"type":"nack"},{"type":"nack","parameter":"pli"}],"parameters":{}},{"mimeType":"video/rtx","clockRate":90000,"payloadType":102,"rtcpFeedback":[],"parameters":{"apt":101}}],"headerExtensions":[{"uri":"http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time","id":4},{"uri":"http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01","id":5},{"uri":"http://tools.ietf.org/html/draft-ietf-avtext-framemarking-07","id":6},{"uri":"urn:3gpp:video-orientation","id":11},{"uri":"urn:ietf:params:rtp-hdrext:toffset","id":12}],"encodings":[{"ssrc":685058887,"rtx":{"ssrc":463150472}}],"rtcp":{"cname":"d1blHuW4nPAoUtQb","reducedSize":true,"mux":true}},"type":"simple","consumableRtpEncodings":[{"ssrc":632915476}],"paused":true}};

			consumeData.internal.transportId = consumerTransport;
			consumeData.internal.producerId =  producer.id;
			consumeData.internal.consumerId =  uuidV4();
			

			//console.log('conumer2 ' + JSON.stringify(consumeData, null, 4) );

			consumeData.data = reqData;
			console.log('consumer3 ' + JSON.stringify(consumeData, null, 4) );

		} catch (error) {
		    console.error('consume failed', error);
		    return;
		}

		// if (consumer.type === 'simulcast') {
		//     await consumer.setPreferredLayers({ spatialLayer: 2, temporalLayer: 2 });
		// }

		io.sockets.connected[serverSocketid].emit('rest', roomid, socket.id, consumeData, function (data1) {
					
					console.log("ack consume " + JSON.stringify(data1, null, 4)); // data will be 'woot'
					consumer = { id: consumeData.internal.consumerId,
				    kind: consumeData.data.kind,
				    rtpParameters: consumeData.data.rtpParameters,
				    type: consumeData.data.type,
				    paused: data1.data.paused,
	                producerPaused: data1.data.producerPaused,
	                score: data1.data.score,
	                preferredLayers: data1.data.preferredLayers

			     };


			    console.log("ack consumer " + JSON.stringify(consumer, null, 4)); // data will be 'woot'
				

				fn({
			    producerId: producer.id,
			    id: consumer.id,
			    kind: consumer.kind,
			    rtpParameters: consumer.rtpParameters,
			    type: consumer.type,
			    producerPaused: consumer.producerPaused
			  });

			});
    });



    socket.on('resume', function (data, fn) {

    //console.log('consume reume ' + JSON.stringify(data, null, 4) );
    var resume = {"id":10,"method":"consumer.resume","internal":{"routerId":"2e32062d-f04a-4c2d-a656-b586e50498ef","transportId":"4ca62904-639c-4b5a-86e8-f0bc84bfe776","consumerId":"c3dbd428-ff6e-46b6-a1e7-ba3891b70f34","producerId":"87c81cfc-b307-4270-a213-4f17e4776931"}};
 
	resume.internal.transportId = consumerTransport;
	resume.internal.producerId =  producer.id;
	resume.internal.consumerId =  consumer.id;
			

	console.log('conumer resume ' + JSON.stringify(resume, null, 4) );

	io.sockets.connected[serverSocketid].emit('rest', roomid, socket.id, resume, function (data1) {
			
		console.log("ack consume resume " + JSON.stringify(data1, null, 4)); // data will be 'woot'
		
		//fn();

		});

      ///////
    });





//////////////////////////////////////////////////////////////////////////
	socket.on('message', function(data) {

		console.log('notification ' + JSON.stringify(data, null, 4) );

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

	socket.on('bye', function(){
		console.log('received bye');
	});

});


}
