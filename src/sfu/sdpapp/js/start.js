'use strict';

var isChannelReady = true;
var isInitiator = false;
var isStarted = false;
//var localStream;
//var track;

//var remoteStream;
//var turnReady;

var room = 'foo'; /*think as a group  peerName@room */
var  remotePeerID;
var  peerID;
var  remotePeerName;
var  peerName;


var pc1;
var pc2;
var socket = io.connect();


socket.on('created', function(room) {
  console.log('Created room ' + room);
  isInitiator = true;
});

socket.on('full', function(room) {
  console.log('Room ' + room + ' is full');
});

socket.on('join', function (room){
  console.log('Another peer made a request to join room ' + room);
  console.log('This peer is the initiator of room ' + room + '!');
  isChannelReady = true;
});

socket.on('joined', function(room, id) {
 console.log('joined: ' + room + ' with peerID: ' + id);
 //log('joined: ' + room + ' with peerID: ' + id);
  isChannelReady = true;
  peerID = id;


  //if (isInitiator) {

    // when working with web enable bellow line
    // doCall();
    // disable  send message 
    //  sendMessage ({
    //   from: peerID,
    //   to: remotePeerID,
    //   type: 'offer',
    //   desc:'sessionDescription'
    // });

 // }

});

socket.on('log', function(array) {
  console.log.apply(console, array);
});

////////////////////////////////////////////////

function sendMessage(message) {
  console.log('Client sending message: ', message);
  socket.emit('message', message);
}

// This client receives a message
socket.on('message', function(message) {
  console.log('Client received message:', message);


  if (!message.offer && remotePeerID && remotePeerID != message.from) {
      console.log('Dropping message from unknown peer', message);

      return;
  }


  if (message === 'got user media') {
    maybeStart();
  } else if (message.type === 'offer') {
   // if (!isInitiator && !isStarted) {
   //   maybeStart();
   // }
    remotePeerID=message.from;
     
     console.log( " Pc2 offers %o", message.desc);
     
   // pc2.setRemoteDescription(new RTCSessionDescription(message.desc));
   // doAnswer();
  } else if (message.type === 'answer' && isStarted) {
    console.log("publish andwer %o", message)
    pc1.setRemoteDescription(new RTCSessionDescription(message.desc));
  } else if (message.type === 'candidate' && isStarted) {
    var candidate = new RTCIceCandidate({
      sdpMLineIndex: message.candidate.sdpMLineIndex,
      sdpMid: message.candidate.sdpMid,
      candidate: message.candidate.candidate
    });
    pc.addIceCandidate(candidate);
  } else if (message.type === 'bye' && isStarted) {
    handleRemoteHangup();
  }
});

////////////////////////////////////////////////////


async function getUserMedia1( isWebcam) {


  let stream;
  try {

  stream =  await navigator.mediaDevices.getUserMedia({ video: true });

    // stream = isWebcam ?
    //   await navigator.mediaDevices.getUserMedia({ video: true }) :
    //   await navigator.mediaDevices.getDisplayMedia({ video: true });
  } catch (err) {
    console.error('getUserMedia() failed:', err.message);
    throw err;
  }
  return stream;
}

/////////////////////////////////////////////////////////////////////////////

async function subscribe() {

  var parser = new URL(window.location.href); 
  var istcp = parser.searchParams;
  const tcpValue = istcp.get('forceTcp') ? true : false;


  socket.emit('createConsumerTransport', {  forceTcp: tcpValue }, async function (data) {

  console.log(data);	

  const transport = device.createRecvTransport(data);

  //////////////////
  transport.on('connect', async ({ dtlsParameters }, callback, errback) => {
    socket.emit('connectConsumerTransport', { transportId: transport.id, dtlsParameters }, async function () {
    	 console.log("connectConsumerTransport");
    	  callback();
    });
    

  });
  //////////////////
  transport.on('connectionstatechange', (state) => {
    switch (state) {
      case 'connecting':
        // $txtSubscription.innerHTML = 'subscribing...';
        // $fsSubscribe.disabled = true;
        console.log('consume subscribing...');
        break;

      case 'connected':
       // document.querySelector('#remote_video').srcObject = stream;
        // $txtSubscription.innerHTML = 'subscribed';
        // $fsSubscribe.disabled = true;
        console.log('consume subscribed.');
        break;

      case 'failed':
        transport.close();
        // $txtSubscription.innerHTML = 'failed';
        // $fsSubscribe.disabled = false;
        console.log('consumer failed');
        break;

      default: break;
    }
  });

  const stream = await consume(transport);

  /////////////////

});//end createconsumer
 
}//end subscribe 

async function consume(transport) {
  const { rtpCapabilities } = device;

   socket.emit('consume', {rtpCapabilities }, async function (data) {
    	 console.log("consume");
    	  const {    producerId,   id,   kind,   rtpParameters,  } = data;

    	    let codecOptions = {};
			  const consumer = await transport.consume({
			    id,
			    producerId,
			    kind,
			    rtpParameters,
			    codecOptions,
			  });
			  const stream = new MediaStream();
			  stream.addTrack(consumer.track);

			  socket.emit('resume');

			  document.querySelector('#remote_video').srcObject = stream;

			  return stream;
    });

}
    

///////////////////////////////////////////////////////////////////////////

//var pc1
async function publish() 
{
  // const isWebcam = (e.target.id === 'btn_webcam');
  // $txtPublish = isWebcam ? $txtWebcam : $txtScreen;

  var parser = new URL(window.location.href); 
  var istcp = parser.searchParams;
  const tcpValue = istcp.get('forceTcp') ? true : false;


   let stream;

   stream =  await getUserMedia1(true);
   const track = stream.getVideoTracks()[0];
   const params = { track };



    pc1 = new RTCPeerConnection(
      {
        iceServers         : [],
        iceTransportPolicy : 'all',
        bundlePolicy       : 'max-bundle',
        rtcpMuxPolicy      : 'require',
        sdpSemantics       : 'unified-plan'
      });

        // Handle RTCPeerConnection connection status.
        pc1.addEventListener('iceconnectionstatechange', () =>
        {
            switch (pc1.iceConnectionState)
            {
                case 'checking':
                     console.log( 'publishing...');
                    break;
                case 'connected':
                case 'completed':
                    document.querySelector('#local_video').srcObject = stream;
                  // $txtPublish.innerHTML = 'published';
                  // $fsPublish.disabled = true;
                  // $fsSubscribe.disabled = false;
                   console.log( 'published...');
                    break;
                case 'failed':
                    pc1.close();
                    // $txtPublish.innerHTML = 'failed';
                    // $fsPublish.disabled = false;
                    // $fsSubscribe.disabled = true;
                    console.log( 'failed...');
                    break;
                case 'disconnected':
                                         pc1.close();
                    // $txtPublish.innerHTML = 'failed';
                    // $fsPublish.disabled = false;
                    // $fsSubscribe.disabled = true;
                    console.log( 'failed...');
                    break;
                case 'closed':
                     pc1.close();
                    // $txtPublish.innerHTML = 'failed';
                    // $fsPublish.disabled = false;
                    // $fsSubscribe.disabled = true;
                    console.log( 'failed...');
                    break;
            }
        });

        var encodings;
        var _stream = new MediaStream();

        const transceiver = pc1.addTransceiver(
            track,
            {
                direction     : 'sendonly'

            });
         
         

        const offer = await pc1.createOffer();

        console.log( "publish offer: %o", offer);

         await pc1.setLocalDescription(offer);

        // We can now get the transceiver.mid.
        const localId = transceiver.mid;

        console.log("arvind transceiver.mid " + transceiver.mid);

        isStarted = true;



        sendMessage ({
          from: peerID,
          to: remotePeerID,
          type: pc1.localDescription.type,
          desc: pc1.localDescription
        });





  //socket.emit('createProducerTransport', {  forceTcp: tcpValue,  rtpCapabilities: device.rtpCapabilities,
 


 







    //////////////////////////////////////////////////////////////

  
}
