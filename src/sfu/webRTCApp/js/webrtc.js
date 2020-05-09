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
     
     pc2.setRemoteDescription(new RTCSessionDescription(message.desc));
    doAnswer();
  } else if (message.type === 'answer' && isStarted) {
    remotePeerID=message.from;
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

function doAnswer() {
  console.log('Sending answer to peer.');

  pc2.createAnswer().then(
    setLocalAndSendMessage2,
    onCreateSessionDescriptionError
  );
}

////////////////////////////////////////////////////
function setLocalAndSendMessage2(sessionDescription) {
  pc2.setLocalDescription(sessionDescription);
  console.log('Pc2 answer %o', sessionDescription);

    sendMessage ({
      room: room,
      from: peerID,
      to: remotePeerID,
      type: sessionDescription.type,
      desc:sessionDescription
    });

   const transceivers = pc2.getTransceivers() ;

           
    console.log( "transceivers %o", transceivers);
   if (!transceivers)
            throw new Error('new RTCRtpTransceiver not found');
     

  const stream = new MediaStream();
   for (var transceiver in transceivers) {
        const track = transceivers[transceiver].receiver.track ;
        stream.addTrack(track);

   }




       // socket.emit('resume');

  document.querySelector('#remote_video').srcObject = stream;


    //alert('hi');

   // remoteVideo.srcObject = stream;
}


function onCreateSessionDescriptionError(error) {
  log('Failed to create session description: ' + error.toString());
  console.log('Failed to create session description: ' + error.toString());
  
}
////////////////////////////////////////////////////////////

async function getUserMedia1( isWebcam) {


  let stream;
  try {

  stream =  await navigator.mediaDevices.getUserMedia({ audio: true, video: true });

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
async function btn_subscribe_resume() {

    sendMessage ({
          room: room,
          from: peerID,
          to: remotePeerID,
          type: "subscribe-resume",
        });
}

async function btn_subscribe_pause() {

    sendMessage ({
          room: room,
          from: peerID,
          to: remotePeerID,
          type: "subscribe-pause",
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
   const videotrack = stream.getVideoTracks()[0];

   const audiotrack = stream.getAudioTracks()[0];

   //const params = { track };



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

                    const streamV = new MediaStream();
                    streamV.addTrack(videotrack);

                    document.querySelector('#local_video').srcObject = streamV;
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


        // const transceiver = pc1.addTransceiver(
        //     videotrack,
        //     {
        //         direction     : 'sendonly'

        //     });

        const transceiver1 = pc1.addTransceiver(
            audiotrack,
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

        //document.querySelector('#local_video').srcObject = stream;


        sendMessage ({
          room: room,
          from: peerID,
          to: remotePeerID,
          type: pc1.localDescription.type,
          desc: pc1.localDescription
        });

 
}
/////////////////////////////End Publish


async function subscribe() {

  var parser = new URL(window.location.href); 
  var istcp = parser.searchParams;
  const tcpValue = istcp.get('forceTcp') ? true : false;


  sendMessage ({
          room: room,
          from: peerID,
          to: remotePeerID,
          type: "subscribe",
        });


 ////////////////
  pc2 = new RTCPeerConnection(
      {
        iceServers         : [],
        iceTransportPolicy : 'all',
        bundlePolicy       : 'max-bundle',
        rtcpMuxPolicy      : 'require',
        sdpSemantics       : 'unified-plan'
      });

        // Handle RTCPeerConnection connection status.
        pc2.addEventListener('iceconnectionstatechange', () =>
        {
            switch (pc2.iceConnectionState)
            {
                case 'checking':
                     console.log( 'subscribing...');
                    break;
                case 'connected':
                case 'completed':
                  //  document.querySelector('#local_video').srcObject = stream;
                  // $txtPublish.innerHTML = 'published';
                  // $fsPublish.disabled = true;
                  // $fsSubscribe.disabled = false;
                   console.log( 'subscribed...');
                    break;
                case 'failed':
                    pc2.close();
                    // $txtPublish.innerHTML = 'failed';
                    // $fsPublish.disabled = false;
                    // $fsSubscribe.disabled = true;
                    console.log( 'failed...');
                    break;
                case 'disconnected':
                                         pc2.close();
                    // $txtPublish.innerHTML = 'failed';
                    // $fsPublish.disabled = false;
                    // $fsSubscribe.disabled = true;
                    console.log( 'failed...');
                    break;
                case 'closed':
                     pc2.close();
                    // $txtPublish.innerHTML = 'failed';
                    // $fsPublish.disabled = false;
                    // $fsSubscribe.disabled = true;
                    console.log( 'failed...');
                    break;
            }
        });


  /////////////////

 
}//end subscribe 