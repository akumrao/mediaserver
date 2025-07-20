'use strict';

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

var encType;


var pcConfig = {
  'iceServers': [{
    'urls': 'stun:stun.l.google.com:19302'
  }]
};




let trackarr = new Array();


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


  let camid = getUrlVars()["camera"];
   if ( !camid ) {
       camid =1;
    }


  let startTime = getUrlVars()["start"];
   if ( !startTime ) {
     startTime =0;
  }

  let endTime = getUrlVars()["end"];
   if ( !endTime ) {
     endTime =0;
  }


  let width = getUrlVars()["width"];
   if ( !width ) {
     width =800;
  }

  let height = getUrlVars()["height"];
   if ( !height ) {
     height =640;
  }


  let scale = getUrlVars()["scale"];
   if ( !scale ) {
     scale =1;
  }


  let speed = getUrlVars()["speed"];
   if ( !speed ) {
     speed =1;
  }

  let encoder = getUrlVars()["encoder"];
   if ( !encoder ) {
     encoder ="NATIVE";
  }


  if (isInitiator) {

    // when working with web enable bellow line
    // doCall();
    // disable  send message 
     sendMessage ({
      room: roomId,
      cam: camid.toString(),
      start: startTime.toString(),
      end: endTime.toString(),
      width: width.toString(),
      height: height.toString(),
      speed: speed.toString(),
      scale: scale.toString(),
      encoder: encoder.toString(),
      ai: true,
      type: 'offer'
    });

  }

});

socket.on('log', function(array) {
  console.log.apply(console, array);
});

////////////////////////////////////////////////
function getUrlVars() {
    var vars = {};
    var parts = window.location.href.replace(/[?&]+([^=&]+)=([^&]*)/gi, function(m,key,value) {
        vars[key] = value;
    });
    return vars;
}

function sendMessage(message) {
  console.log('Client sending message: ', message);
  log('Client sending message: ', message);
  //socket.emit('message', message);
  socket.emit('messageToWebrtc', message);
}


var text, parser, xmlDoc;


parser = new DOMParser();

var c = document.getElementById("cv1");

var ctx = c.getContext("2d");


function draw( text)
{


  xmlDoc = parser.parseFromString(text,"text/xml");

  ctx.clearRect(0, 0, c.width, c.height);
  
  let ObjectBox = xmlDoc.getElementsByTagName("Object");
  if(!ObjectBox)
  {
    return;
  }

  for( var ncount =0 ; ncount <ObjectBox.length ; ++ncount )
  {

    let boundingBox = ObjectBox[ncount].getElementsByTagName("BoundingBox")[0];

    if(!boundingBox)
    {
      continue;
    }

    var x0 =   parseFloat(boundingBox.attributes["X0"].nodeValue);

    var y0 =   parseFloat(boundingBox.attributes["Y0"].nodeValue);

    var x1 =   parseFloat(boundingBox.attributes["X1"].nodeValue);

    var y1 =   parseFloat(boundingBox.attributes["Y1"].nodeValue);

    let label = ObjectBox[ncount].getElementsByTagName("Classification")[0];

    if(label)
    {
      name = label.attributes["Label"].nodeValue;
    }


    let attrbute = ObjectBox[ncount].getElementsByTagName("Attribute")[0];

    if(attrbute)
    {
      var colorName  =   attrbute.attributes["Name"].nodeValue;
      var colorValue =   attrbute.attributes["Value"].nodeValue;
      name +=  " " +  colorName + " " + colorValue;
    
    }


    

    x0 = x0*c.width;
    y0 = y0*c.height;

    x1 = x1*c.width;
    y1 = y1*c.height;
    
    var w =  x1 - x0;
    var h  = y1 - y0;


    ctx.beginPath();
    ctx.rect(x0, y0, w, h);

   if(label)
   {
     ctx.fillText(name, x0+2, y0+10);
   }

    ctx.stroke();
  }

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
   // log('got offfer from remotePeerID: ' + remotePeerID);

   //https://github.com/fippo/simulcast-playground/blob/gh-pages/active-false-in-addTransceiver.html
   //https://chromium.googlesource.com/external/webrtc-samples/+/c0b26a7235bac580279866f884a749d0aecd0656%5E1..c0b26a7235bac580279866f884a749d0aecd0656/

    var des = new RTCSessionDescription(message.desc);


    let mediaSections = SDPUtils.getMediaSections(des.sdp);

    const mediaSection =mediaSections[mediaSections.length -1];

    const rtpParameters = SDPUtils.parseRtpParameters(mediaSection);

  
    let statsText = '';

    for (var i = 0; i <  rtpParameters.codecs.length; i++) 
    {

      var codecName = rtpParameters.codecs[i].name;

      var tmpObj={};
      
      tmpObj["MaxEnc"] = rtpParameters.codecs[i].parameters["MaxEnc"];
      //tmpObj["PresentEncIns"] = rtpParameters.codecs[i].parameters["PresentEncIns"];

      if(codecName == "VP9")
        tmpObj["SwEnc"] = rtpParameters.codecs[i].parameters["PresentEncIns"];
      else if(codecName == "H264")
      {  
         encType = rtpParameters.codecs[i].parameters["Enc"];

         tmpObj[encType] =rtpParameters.codecs[i].parameters["PresentEncIns"];

      }
      if(codecName == "VP9" || codecName == "H264" )
      {
        statsText += `<div>Encoder: ${ JSON.stringify(tmpObj)}</div>`;
      }

     // console.log( " arv %o", rtpParameters.codecs[i]);
    }

    let statsDiv = document.getElementById("statsEnc");
    statsDiv.innerHTML = statsText;
 

    pc.setRemoteDescription(des);
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

    console.log('Camera state', message.desc);
    log('Camera state:', message.desc);

    handleRemoteHangup();
  }
  else if(message.type === 'error') {
   
    console.log('Camera state', message.desc);
    log('Camera state:', message.desc);
    hangup();
  }

});

////////////////////////////////////////////////////

// var localVideo = document.querySelector('#localVideo');
var remoteVideo = document.querySelector('#remoteVideo');

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


var enc = new TextDecoder("utf-8");
if (!("TextDecoder" in window))
  alert("Sorry, this browser does not support TextDecoder...");

function videoAnalyzer(encodedFrame, controller) {
 
  const view = new DataView(encodedFrame.data);

  //console.log( encodedFrame.data.byteLength);


  //last 4 bytes
  //const txt =  encodedFrame.data.slice( encodedFrame.data.byteLength - 4, encodedFrame.data.byteLength); 
  
  const metasize = view.getUint32(encodedFrame.data.byteLength - 4);

  if( metasize < encodedFrame.data.byteLength &&  metasize < 2000)
  {

    const tag = view.getUint32(encodedFrame.data.byteLength - 8 - metasize);
    const metaData =  encodedFrame.data.slice( encodedFrame.data.byteLength - 4 - metasize, encodedFrame.data.byteLength-4);

    if (view.getUint32(0) == 1) 
    {  //  h264 start code '0001' 


      encodedFrame.data = encodedFrame.data.slice(0, encodedFrame.data.byteLength - 8 - metasize);
      //console.log("h264 =======");
      //console.log(tag);
      //console.log(metasize);
      var xmlDec =  enc.decode(metaData);
      //console.log(xmlDec);
      try
      {
         var xmlFrame= atob(xmlDec);
        // console.log(xmlFrame);
        
      }
      catch(err)
      {
        console.log("Metadata xml parser fails." + err);
        return;
      }

      draw(xmlFrame);

    }
  
  }

    /*
      // We assume that the video is VP9.
      // TODO: Check the codec to see that it is.
      // The lowest value bit in the first byte is the keyframe indicator.
      // https://tools.ietf.org/html/rfc6386#section-9.1
      const keyframeBit = view.getUint8(0) & 0x01;
      // console.log(view.getUint8(0).toString(16));
      if (keyframeBit === 0) {
        keyFrameCount++;
        keyFrameLastSize = encodedFrame.data.byteLength;
      } else {
        interFrameCount++;
        interFrameLastSize = encodedFrame.data.byteLength;
      }
      if (encodedFrame.type === prevFrameType &&
          encodedFrame.timestamp === prevFrameTimestamp &&
          encodedFrame.synchronizationSource === prevFrameSynchronizationSource) {
        duplicateCount++;
      }
      prevFrameType = encodedFrame.type;
      prevFrameTimestamp = encodedFrame.timestamp;
      prevFrameSynchronizationSource = encodedFrame.synchronizationSource;

     console.log( prevFrameType );

      console.log( prevFrameTimestamp );
    */
    controller.enqueue(encodedFrame);
}


function gotRemoteTrack(receiver) {
  console.log('pc2 received remote stream');
  const frameStreams = receiver.createEncodedStreams();
  frameStreams.readable.pipeThrough(new TransformStream({
    transform: videoAnalyzer
  }))
      .pipeTo(frameStreams.writable);
  //remoteVideo.srcObject = e.streams[0];
}



async function createPeerConnection() {
  try {
    //pc = new RTCPeerConnection(null);

     pc = new RTCPeerConnection(
        {
            encodedInsertableStreams: true,
            iceServers         : [{'urls': 'stun:stun.l.google.com:19302'}],
            iceTransportPolicy : 'all',
            bundlePolicy       : 'max-bundle',
            rtcpMuxPolicy      : 'require',
            sdpSemantics       : 'unified-plan'
        });



    pc.onicecandidate = handleIceCandidate;
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
  //sessionDescription.sdp = sessionDescription.sdp.replace("useinbandfec=1", "useinbandfec=1; minptime=10; stereo=1; maxaveragebitrate=510000");

  sessionDescription.sdp = sessionDescription.sdp.replaceAll("level-asymmetry-allowed=1", "level-asymmetry-allowed=1; Enc=" + encType );


  // let mediaSection = SDPUtils.getMediaSections(sessionDescription.sdp)[0];
  // let rtpParameters = SDPUtils.parseRtpParameters(mediaSection);



  // let statsText = '';

  // for (var i = 0; i <  rtpParameters.codecs.length; i++) 
  // {

  //   var codecName = rtpParameters.codecs[i].name;

  //   var tmpObj={};
    
  //   tmpObj["MaxEnc"] = rtpParameters.codecs[i].parameters["MaxEnc"];
  //   //tmpObj["PresentEncIns"] = rtpParameters.codecs[i].parameters["PresentEncIns"];

  //   if(codecName == "VP9")
  //     tmpObj["SwEnc"] = rtpParameters.codecs[i].parameters["PresentEncIns"];
  //   else if(codecName == "H264")
  //   {
  //      rtpParameters.codecs[i].parameters["PresentEncIns"] =1;
  //   }
  //   if(codecName == "VP9" || codecName == "H264" )
  //   {
  //     statsText += `<div>Encoder: ${ JSON.stringify(tmpObj)}</div>`;
  //   }

  //  // console.log( " arv %o", rtpParameters.codecs[i]);
  // }






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

// function requestTurn(turnURL) {
//   var turnExists = false;
//   for (var i in pcConfig.iceServers) {
//     if (pcConfig.iceServers[i].urls.substr(0, 5) === 'turn:') {
//       turnExists = true;
//       turnReady = true;
//       break;
//     }
//   }
//   if (!turnExists) {
//     console.log('Getting TURN server from ', turnURL);
//     // No TURN server. Get one from computeengineondemand.appspot.com:
//     var xhr = new XMLHttpRequest();
//     xhr.onreadystatechange = function() {
//       if (xhr.readyState === 4 && xhr.status === 200) {
//         var turnServer = JSON.parse(xhr.responseText);
//         console.log('Got TURN server: ', turnServer);
//         pcConfig.iceServers.push({
//           'urls': 'turn:' + turnServer.username + '@' + turnServer.turn,
//           'credential': turnServer.password
//         });
//         turnReady = true;
//       }
//     };
//     xhr.open('GET', turnURL, true);
//     xhr.send();
//   }
// }

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



   


pc.ontrack = ({transceiver, receiver, streams: [stream]}) => {
  //log("pc.ontrack with transceiver and streams");

  if(transceiver.direction != 'inactive' && transceiver.currentDirection != 'inactive')
  {   
    var track = transceiver.receiver.track;
    console.log("pc.ontrack with transceiver and streams " + track.kind);
  }

  gotRemoteTrack(receiver);

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
            {
                start();
                setupWebRtcPlayer(pc);
                onWebRtcAnswer();

                //sendStartLatencyTest();

               console.log( 'checking...');
            }
              break;
          case 'connected':
           
              console.log( 'connected...');

          break;

          case 'completed':
   
              console.log( 'completed...');
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
        remoteVideo.muted = true;
    } else {
      remoteVideo.muted = false;
    }





   if( !trackarr.length) 
   {

     trackarr.push("1");
      //checkBox.checked = false;
      //alert("Please click and select elements to pause");
      //return;
   }



    sendMessage ({
        room: roomId,
        //to: remotePeerID,
        type: 'command',
        desc: 'muteaudio',
        trackids: trackarr,
        act:checkBox.checked
      });



}



// function listPalette(senders) {

//   //codecList = sender.getCapabilities('video');

//   let codecList = null;

//   for (var i = 0; i < senders.length; i++) 
//   {

//      codecList = senders[i].getParameters().codecs;
    

//       for (var j = 0; j < codecList.length; j++) 
//       {

//       //if (codecList[i].mimeType == "audio/opus") {
//        //params.codecs.unshift(params.codecs.splice(i, 1));
//        //break;

//         console.log( " arv %o", codecList[j]);
//         console.log( codecList[j].mimeType);
//       }


//   }

// }              

function changeVideoCodec(mimeType) {
  const transceivers = pc.getTransceivers();

  transceivers.forEach((transceiver) => {
   // const kind = transceiver.sender.track.kind;
    let sendCodecs = RTCRtpSender.getCapabilities('video').codecs;
    let recvCodecs = RTCRtpReceiver.getCapabilities('video').codecs;

    if (kind === "video") {
      sendCodecs = preferCodec(mimeType);
      recvCodecs = preferCodec(mimeType);
      transceiver.setCodecPreferences([...sendCodecs, ...recvCodecs]);
    }
  });

  pc.onnegotiationneeded();
}


function addAICamera()
{

  var camid = document.getElementById("camId").value;
  var startTime =  document.getElementById("startTime").value;

  var endTime = 0;

  var width = document.getElementById("widthVideo").value;
  var height= document.getElementById("heightVideo").value;
  

  var speed = document.getElementById("speed").value;

  if( startTime=="0" && speed != "1")
  {
     alert("Please enter start time for Speed > 1")
      document.getElementById("speed").value = 1;
     return;
  }

  var scale = document.getElementById("scale").value;

  var encoder = document.getElementById("encoder").value;

  // if(scale < 0 && startTime == 0)
  // {
  //    const epoch =new Date().getTime()/1000;
  //    startTime = epoch;
  //    document.getElementById("startTime").value = epoch;
  // }


  sendMessage ({
      room: roomId,
      cam: camid.toString(),
      start: startTime.toString(),
      end: endTime.toString(),
      width: width.toString(),
      height: height.toString(),
      speed: speed.toString(),
      scale: scale.toString(),
      encoder: encoder.toString(),
      ai: true,
      type: 'offer',

    });

}

function addCamera()
{
  
  var camid = document.getElementById("camId").value;
  var startTime =  document.getElementById("startTime").value;

  var endTime = 0;

  var width = document.getElementById("widthVideo").value;
  var height= document.getElementById("heightVideo").value;
  

  var speed = document.getElementById("speed").value;

  if( startTime=="0" && speed != "1")
  {
     alert("Please enter start time for Speed > 1")
      document.getElementById("speed").value = 1;
     return;
  }

  var scale = document.getElementById("scale").value;

  var encoder = document.getElementById("encoder").value;

  // if(scale < 0 && startTime == 0)
  // {
  //    const epoch =new Date().getTime()/1000;
  //    startTime = epoch;
  //    document.getElementById("startTime").value = epoch;
  // }


  sendMessage ({
      room: roomId,
      cam: camid.toString(),
      start: startTime.toString(),
      end: endTime.toString(),
      width: width.toString(),
      height: height.toString(),
      speed: speed.toString(),
      scale: scale.toString(),
      encoder: encoder.toString(),
      type: 'offer',

    });

}



function applyCamera()
{
  
  var camids = document.getElementById("camId").value;

  var endTime = 0;

  var width = document.getElementById("widthVideo").value;
  var height= document.getElementById("heightVideo").value;
  

  var speed = document.getElementById("speed").value;
  var scale = document.getElementById("scale").value;

  var startTime =  document.getElementById("startTime").value;



  if(!trackarr.length) 
  {
    alert("Please click and select elements to reverseplay or change resolution");
    return;
  }

  for( var x =0; x < trackarr.length; ++x)
  {
    let el = document.getElementById(`vd-${trackarr[x]}`);

    el.style.maxWidth =  width +'px';
    // el.style.width =  400;
    // el.style.height = 400;

      el.width =  width +'px';

  }

  if( startTime=="0" && speed != "1")
  {
     alert("Please enter starttime for Speed > 1")
      document.getElementById("speed").value = 1;
     return;
  }

  sendMessage ({
      room: roomId,
      start: startTime.toString(),
      end: endTime.toString(),
      width: width.toString(),
      height: height.toString(),
      speed: speed.toString(),
      scale: scale.toString(),
      type: 'command',
      trackids: trackarr,
      desc: 'apply',
      act:true

    });


}


function forward()
{

    document.getElementById("scale").value = 1;

    applyCamera();

}


function backward()
{
     document.getElementById("scale").value = -1;
     applyCamera();

}
