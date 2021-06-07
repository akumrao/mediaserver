'use strict';


  var dataChannelLabel = "testchannel";

var isChannelReady = true;
var isInitiator = false;
var isStarted = false;
//var localStream;
var pc;
var remoteStream;
var turnReady;

var roomId = 'foo'; /*think as a group  peerName@room */
//var  remotePeerID;
var  peerID;
//var  remotePeerName;
var  peerName;

 var sourceBuffer = null;

var pcConfig = {
  'iceServers': [{
    'urls': 'stun:stun.l.google.com:19302'
  }]
};

// Set up audio and video regardless of what devices are present.
/*var sdpConstraints = {
  offerToReceiveAudio: true,
  offerToReceiveVideo: true
};*/

/////////////////////////////////////////////


// Could prompt for room name:
// room = prompt('Enter room name:');

var socket = io.connect();


socket.on('created', function(room) {
  console.log('Created room ' + room);
  isInitiator = true;
});



socket.on('join', function (room, id, numClients){
  console.log('New peer joins room ' + room + '!' +" client id " + id);
  isChannelReady = true;
});
socket.on('joined', function(room, id, numClients) {
 console.log('joined: ' + room + ' with peerID: ' + id);
 log('joined: ' + room + ' with peerID: ' + id);
  isChannelReady = true;
  peerID = id;


  if (isInitiator) {

    // when working with web enable bellow line
    // doCall();
    // disable  send message 
     sendMessage ({
      room: roomId,
      //to: remotePeerID,
      type: 'offer',
      desc:'sessionDescription'
    });

  }

});

socket.on('log', function(array) {
  console.log.apply(console, array);
});

////////////////////////////////////////////////

function sendMessage(message) {
  console.log('Client sending message: ', message);
  log('Client sending message: ', message);
  //socket.emit('message', message);
  socket.emit('sfu-message', message);
}

// This client receives a message
socket.on('message', function(message) {
  console.log('Client received message:', message);
  log('Client received message:', message);


  if (message === 'got user media') {
    maybeStart();
  } else if (message.type === 'offer') {
    if (!isInitiator && !isStarted) {
      maybeStart();
    }
   // remotePeerID=message.from;
    //log('got offfer from remotePeerID: ' + remotePeerID);

    pc.setRemoteDescription(new RTCSessionDescription(message.desc));
    doAnswer();
  } else if (message.type === 'answer' && isStarted) {
    pc.setRemoteDescription(new RTCSessionDescription(message.desc));
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

//var remoteVideo = document.querySelector('#remoteVideo');

// navigator.mediaDevices.getUserMedia({
//   audio: true,
//   video: true
// })
// .then(gotStream)
// .catch(function(e) {
//   alert('getUserMedia() error: ' + e.name);
// });

// function gotStream(stream) {
//   console.log('Adding local stream.');
//   localStream = stream;
//   localVideo.srcObject = stream;
//   sendMessage('got user media');
//     isInitiator = true;
//   if (isInitiator) {
//     maybeStart();
//   }
// }

//arvind else  if no gotStream
isInitiator = true;
if (isInitiator) {
     maybeStart();
   }

// if (location.hostname !== 'localhost') {
//   requestTurn(
//     'https://computeengineondemand.appspot.com/turn?username=41784574&key=4080218913'
//   );
// }

function maybeStart() {
  console.log('>>>>>>> maybeStart() ', isStarted, isChannelReady);
  if (!isStarted  && isChannelReady) {
    console.log('>>>>>> creating peer connection');
    createPeerConnection();
   // pc.addStream(localStream);
    isStarted = true;
    console.log('isInitiator', isInitiator);
   // if (isInitiator) {
     // doCall();
   // }
      if (roomId !== '') {
        socket.emit('create or join', roomId);
        console.log('Attempted to create or  join room', roomId);
      }


  }
}

window.onbeforeunload = function() {
    sendMessage({
      room: roomId,
      //to: remotePeerID,
      type: 'bye'
    });
};

/////////////////////////////////////////////////////////

function createPeerConnection() {



   
      var video = document.querySelector('#remoteVideo');
      var mimeCodec = 'video/mp4; codecs="avc1.42E01E, mp4a.40.2"';

      if ('MediaSource' in window && MediaSource.isTypeSupported(mimeCodec)) {
        var mediaSource = new MediaSource;
        video.src = URL.createObjectURL(mediaSource);
        mediaSource.addEventListener('sourceopen', function () {
          sourceBuffer = mediaSource.addSourceBuffer(mimeCodec);
          var bufferMode = sourceBuffer.mode
          // if (bufferMode == "segments") {
          //   sourceBuffer.mode = "sequence"
          // }

          sourceBuffer.addEventListener('updateend', function () {

            if (mediaSource.duration !== Number.POSITIVE_INFINITY && video.currentTime === 0 && mediaSource.duration > 0) {
                        video.currentTime = mediaSource.duration - 1;
                        mediaSource.duration = Number.POSITIVE_INFINITY;
              }


            video.play();
          });
        });
      } else {
        console.error("Unsupported MIME type or codec: ", mimeCodec);
      }





  try {
    //pc = new RTCPeerConnection(null);

     pc = new RTCPeerConnection(
        {
            iceServers         : [{'urls': 'stun:stun.l.google.com:19302'}],
            iceTransportPolicy : 'all',
            bundlePolicy       : 'max-bundle',
            rtcpMuxPolicy      : 'require',
            sdpSemantics       : 'unified-plan'
        });



    pc.onicecandidate = handleIceCandidate;



   pc.ondatachannel = function (event) {
      if (event.channel.label == dataChannelLabel) {
        //dataChannel = event.channel;
        console.log("DataChannel received");
        setupDataChannel(event.channel);
      } else {
        console.log("Unknown CataChannel label: " + event.channel.label);
      }
    }




    // pc.onaddstream = handleRemoteStreamAdded;
    // pc.onremovestream = handleRemoteStreamRemoved;
    console.log('Created RTCPeerConnnection');
  } catch (e) {
    console.log('Failed to create PeerConnection, exception: ' + e.message);
    alert('Cannot create RTCPeerConnection object.');
    return;
  }
}

function handleIceCandidate(event) {
  console.log('icecandidate event: ', event);
  if (event.candidate) {
    sendMessage({
      room: roomId,
      //to: remotePeerID,
      type: 'candidate',
      candidate: event.candidate
    });
  } else {
    console.log('End of candidates.');
  }
}

function handleCreateOfferError(event) {
  console.log('createOffer() error: ', event);
}

function doCall() {
  console.log('Sending offer to peer');
  pc.createOffer(setLocalAndSendMessage, handleCreateOfferError);
}

function doAnswer() {
  console.log('Sending answer to peer.');
  pc.createAnswer().then(
    setLocalAndSendMessage,
    onCreateSessionDescriptionError
  );
}

function setLocalAndSendMessage(sessionDescription) {

 // sessionDescription.sdp = sessionDescription.sdp.replace("useinbandfec=1", "useinbandfec=1; minptime=10; cbr=1; stereo=1; sprop-stereo=1; maxaveragebitrate=510000");
  sessionDescription.sdp = sessionDescription.sdp.replace("useinbandfec=1", "useinbandfec=1; minptime=10; stereo=1; maxaveragebitrate=510000");
  pc.setLocalDescription(sessionDescription);
  console.log('setLocalAndSendMessage sending message', sessionDescription);

   sendMessage ({
      room: roomId,
      //to: remotePeerID,
      type: sessionDescription.type,
      desc:sessionDescription
    });
}

function onCreateSessionDescriptionError(error) {
  log('Failed to create session description: ' + error.toString());
  console.log('Failed to create session description: ' + error.toString());
  
}




function handleRemoteStreamAdded(event) {
  console.log('Remote stream added.');
  remoteStream = event.stream;
  remoteVideo.srcObject = remoteStream;
}

function handleRemoteStreamRemoved(event) {
  console.log('Remote stream removed. Event: ', event);
}

function hangup() {
  console.log('Hanging up.');
  stop();
  sendMessage({
      room: roomId,
      //to: remotePeerID,
      type: 'bye'
    });
}

function handleRemoteHangup() {
  console.log('Session terminated.');
  stop();
  //isInitiator = false;
}

function stop() {
  isStarted = false;
  pc.close();
  pc = null;
  //localStream=null;
}



   


pc.ontrack = ({transceiver, streams: [stream]}) => {
  //log("pc.ontrack with transceiver and streams");

  if(transceiver.direction != 'inactive' && transceiver.currentDirection != 'inactive')
  {   
    var track = transceiver.receiver.track;
    console.log("pc.ontrack with transceiver and streams " + track.kind);
  }

  stream.onaddtrack = () => console.log("stream.onaddtrack");
  stream.onremovetrack = () => console.log("stream.onremovetrack");
  transceiver.receiver.track.onmute = () => console.log("transceiver.receiver.track.onmute " + track.id);
  transceiver.receiver.track.onended = () => console.log("transceiver.receiver.track.onended " + track.id);
  transceiver.receiver.track.onunmute = () => {
  console.log("transceiver.receiver.track.onunmute " + track.id);
  remoteVideo.srcObject = stream;

     // var atracks =  streams.getAudioTracks();

     //  for (var tsn in atracks) 
     //  {
     //         var trc = atracks[tsn];
     //          trc.enable = false;

     //         var x = 1;
     //  }
            


  };
};


 pc.addEventListener('iceconnectionstatechange', () =>
  {
      switch (pc.iceConnectionState)
      {
          case 'checking':
              console.log( 'subscribing...');
              break;
          case 'connected':
          case 'completed':


              console.log( 'subscribed...');

              break;
          case 'failed':
             // pc2.close();

              console.log( 'failed...');
              break;
          case 'disconnected':
             // pc2.close();
              console.log( 'Peerconnection disconnected...');
              break;
          case 'closed':
              //pc2.close();
              console.log( 'failed...');
              break;
      }
  });



function onMuteClick() {
  // Get the checkbox
  var checkBox = document.getElementById("checkmute");
  // Get the output text
  // If the checkbox is checked, display the output text
  if (checkBox.checked == true){
    //text.style.display = "block";
  } else {
    //text.style.display = "none";
  }


  sendMessage ({
      room: roomId,
      //to: remotePeerID,
      type: 'mute',
      desc: checkBox.checked
    });

}



var setupDataChannel = function (dataChannel) {
  dataChannel.onopen = function (e) {
    console.log("DataChannel open and ready to be used");

    // $("#send_datachannel_msg").click(function () {
    //   var msg = $("#datachannel_msg").val();
    //   console.log("Sending message: " + msg);
    //   dataChannel.send(msg);
    // });
  };

  dataChannel.onclose = function () {
    console.log("DataChannel closed");
  };

  dataChannel.onerror = function (e) {
    console.log("DataChannel error: " + e.message);
    console.log(e);
  };


  var find_first_iframe = false
  var first_frame = true

  dataChannel.onmessage = function (e) {
    console.log("Received message: " + e.data);
    if (sourceBuffer != null) {
      sourceBuffer.appendBuffer(e.data);
    } else {
      console.log("Got data but sourceBuffer is null");
    }
  };
};

function is_ftyp(d) {
                return (
                  d.length > 8 &&
                  d[4] == 0x66 &&
                  d[5] == 0x74 &&
                  d[6] == 0x79 &&
                  d[7] == 0x70
                )
              }
  function is_moof(d) {
    return (
      d.length > 8 &&
      d[4] == 0x6d &&
      d[5] == 0x6f &&
      d[6] == 0x6f &&
      d[7] == 0x66
    )
  }

  function is_iframe(d) {
    if (is_moof(d)) {
      return d.length > 116 && d[116] == 0x65
    }
    return false
  }

