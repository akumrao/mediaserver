
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


  var verbose = false;
            // var verbose = true; // enable for saturating the console ..
  var buffering_sec = 1; // use some reasonable value

  var buffering_sec_seek = buffering_sec*0.9; 
  // ..seek the stream if it's this much away or
  // from the last available timestamp
  var buffering_sec_seek_distance = buffering_sec*0.5; 
  // .. jump to this distance from the last avail. timestamp

  // *** INTERNAL PARAMETERS ***
  // set mimetype and codec
  var mimeType = "video/mp4";
 // var codecs = "avc1.4D401F"; // https://wiki.whatwg.org/wiki/Video_type_parameters
  // if your stream has audio, remember to include it in these definitions.. otherwise your mse goes sour

  var codecs = "avc1.4D401F,mp4a.40.2";
  var codecPars = mimeType+';codecs="'+codecs+'"';
  
  var stream_started = false; // is the source_buffer updateend callback active nor not
  
  // create media source instance
  var ms = new MediaSource();
  
  // queue for incoming media packets
  var queue = [];
  
  var stream_live; // the HTMLMediaElement (i.e. <video> element)
  var ws; // websocket
  var seeked = false; // have have seeked manually once ..
  var cc = 0;
  
  var source_buffer; // source_buffer instance
  
  var pass = 0;
        







/////////////////////////////////////////////////////////////////////////////////////////////////////////////////


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

   
 
  startup();



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
  dataChannel.binaryType = "arraybuffer";
  dataChannel.onopen = function (e) {
    console.log("DataChannel open and ready to be used");

    // $("#send_datachannel_msg").click(function () {
    //   var msg = $("#datachannel_msg").val();
    //   console.log("Sending message: " + msg);
    //   dataChannel.send(msg);
    // });

    dataChannel.send("reset");
  };

  dataChannel.onclose = function () {
    console.log("DataChannel closed");
  };

  dataChannel.onerror = function (e) {
    console.log("DataChannel error: " + e.message);
    console.log(e);
  };




  dataChannel.onmessage = function (e) {
    // console.log("Received message: " + e.data);
    // if (sourceBuffer != null) {
    //   sourceBuffer.appendBuffer(e.data);
    // } else {
    //   console.log("Got data but sourceBuffer is null");
    // }

     putPacket(e.data);
  };
};



   
            // *** MP4 Box manipulation functions ***
            // taken from here: https://stackoverflow.com/questions/54186634/sending-periodic-metadata-in-fragmented-live-mp4-stream/
            
            function toInt(arr, index) { // From bytes to big-endian 32-bit integer.  Input: Uint8Array, index
                var dv = new DataView(arr.buffer, 0);
                return dv.getInt32(index, false); // big endian
            }

            function toString(arr, fr, to) { // From bytes to string.  Input: Uint8Array, start index, stop index.
                // https://developers.google.com/web/updates/2012/06/How-to-convert-ArrayBuffer-to-and-from-String
                return String.fromCharCode.apply(null, arr.slice(fr,to));
            }

            function getBox(arr, i) { // input Uint8Array, start index
                return [toInt(arr, i), toString(arr, i+4, i+8)]
            }

            function getSubBox(arr, box_name) { // input Uint8Array, box name
                var i = 0;
                res = getBox(arr, i);
                main_length = res[0]; name = res[1]; // this boxes length and name
                i = i + 8;
                
                var sub_box = null;
                
                while (i < main_length) {
                    res = getBox(arr, i);
                    l = res[0]; name = res[1];
                    
                    if (box_name == name) {
                        sub_box = arr.slice(i, i+l)
                    }
                    i = i + l;
                }
                return sub_box;
            }

            function hasFirstSampleFlag(arr) { // input Uint8Array
                // [moof [mfhd] [traf [tfhd] [tfdt] [trun]]]
                
                var traf = getSubBox(arr, "traf");
                if (traf==null) { return false; }
                
                var trun = getSubBox(traf, "trun");
                if (trun==null) { return false; }
                
                // ISO/IEC 14496-12:2012(E) .. pages 5 and 57
                // bytes: (size 4), (name 4), (version 1 + tr_flags 3)
                var flags = trun.slice(10,13); // console.log(flags);
                f = flags[1] & 4; // console.log(f);
                return f == 4;
            }

            
            // consider these callbacks:
            // - putPacket : called when websocket receives data
            // - loadPacket : called when source_buffer is ready for more data
            // Both operate on a common fifo
            
            function putPacket(arr) { 
                // receives ArrayBuffer.  Called when websocket gets more data
                // first packet ever to arrive: write directly to source_buffer
                // source_buffer ready to accept: write directly to source_buffer
                // otherwise insert it to queue
                
                var memview   = new Uint8Array(arr);
                if (verbose) { console.log("got", arr.byteLength, "bytes.  Values=", memview[0], memview[1], memview[2], memview[3], memview[4]); }
   
                res = getBox(memview, 0);
                main_length = res[0]; name = res[1]; // this boxes length and name
                
                if ((name=="ftyp") && (pass==0)) {
                    pass = pass + 1;
                    console.log("got ftyp");
                }
                else if ((name=="moov") && (pass==1)) {
                    pass = pass + 1;
                    console.log("got moov");
                }
                else if ((name=="moof") && (pass==2)) {
                    if (hasFirstSampleFlag(memview)) {
                        pass = pass + 1;
                        console.log("got that special moof");
                    }
                    else {
                        return;
                    }
                }
                else if (pass < 3) {
                    return;
                }
                
                // keep the latency to minimum
                let latest = stream_live.duration;
                if ((stream_live.duration >= buffering_sec) && 
                    ((latest - stream_live.currentTime) > buffering_sec_seek)) {
                    console.log("seek from ", stream_live.currentTime, " to ", latest);
                    df = (stream_live.duration - stream_live.currentTime); // this much away from the last available frame
                    if ((df > buffering_sec_seek)) {
                        seek_to = stream_live.duration - buffering_sec_seek_distance;
                        stream_live.currentTime = seek_to;
                        }
                }

                data = arr;
                if (!stream_started) {
                    if (verbose) {console.log("Streaming started: ", memview[0], memview[1], memview[2], memview[3], memview[4]);}
                    source_buffer.appendBuffer(data);
                    stream_started = true;
                    cc = cc + 1;
                    return;
                }
                
                queue.push(data); // add to the end
                if (verbose) { console.log("queue push:", queue.length); }
            }
            
            
            function loadPacket() { // called when source_buffer is ready for more
               if (!source_buffer.updating) { // really, really ready
                    if (queue.length>0) {
                    
                        inp = queue.shift(); // pop from the beginning
                        if (verbose) { console.log("queue pop:", queue.length); }
                    
                        var memview = new Uint8Array(inp);
                        
                        if (verbose) { console.log(" ==> writing buffer with", memview[0], memview[1], memview[2], memview[3]); }
                        
                        source_buffer.appendBuffer(inp);
                        cc = cc + 1;
                        }
                    else { // the queue runs empty, so the next packet is fed directly
                        stream_started = false;
                    }
                }
                else { // so it was not?
                }
            }


               function opened() 
               { // MediaSource object is ready to go
                // https://developer.mozilla.org/en-US/docs/Web/API/MediaSource/duration
                ms.duration = buffering_sec;
                source_buffer = ms.addSourceBuffer(codecPars);
                
                // https://developer.mozilla.org/en-US/docs/Web/API/source_buffer/mode
                var myMode = source_buffer.mode;
                source_buffer.mode = 'sequence';
                // source_buffer.mode = 'segments';
                
                source_buffer.addEventListener("updateend",loadPacket);
           
                
              }

            function startup() {
                ms.addEventListener('sourceopen',opened,false);
                
                // get reference to video
                stream_live = document.getElementById('stream_live');
            
                // set mediasource as source of video
                stream_live.src = window.URL.createObjectURL(ms);
            }
            
            
          //  window.onload = function() {
            //    startup();
           // }