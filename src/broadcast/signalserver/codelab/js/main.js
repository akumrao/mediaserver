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

function createPeerConnection() {
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

  sessionDescription.sdp = sessionDescription.sdp.replace("useinbandfec=1", "useinbandfec=1; minptime=10; cbr=1; stereo=1; maxaveragebitrate=510000");
  
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

   // pc.ontrack = ({transceiver, streams: [stream]})



   //  pc.ontrack = ({transceiver, streams: [stream]}) => {


   //  if(transceiver.direction != 'inactive' && transceiver.currentDirection != 'inactive')
   //  {   
   //  var track = transceiver.receiver.track;
   //  console.log("pc.ontrack with transceiver and streams " + track.kind);


   //  if (track.kind === 'video' || track.kind === 'audio') {

   //      let el = document.createElement(track.kind);

   //      el.setAttribute('playsinline', true);
   //      el.setAttribute('autoplay', true);
   //      el.setAttribute('id', `va-${track.id}`);
   //      el.controls= true;
   //      //videoPlayerElement.load();
   //      //el.id = `video-${track.id}`;


   //      // set some attributes on our audio and video elements to make
   //      // mobile Safari happy. note that for audio to play you need to be


   //      var div = document.createElement('div');

   //      var para = document.createElement("P");
   //      para.innerHTML = "<span> <small> trackid:" +  track.id  + "<br>"+  "peerID:" +  stream.id  + "<br>" +   "</small> </span>";
   //      div.appendChild(para);
        

   //      //div.textContent = `streamid-${stream.id}`
   //     // div.potato= store;

   //      div.appendChild(el);

   //      div.id = `consumer-div-${track.id.substring(0, 36)}`;


   //      let pause = document.createElement('span'),
   //      checkbox = document.createElement('input'),
   //      label = document.createElement('label');

   //      pause.classList = 'nowrap';
   //      checkbox.type = 'checkbox';
   //      checkbox.id=track.id;
   //      checkbox.checked = false;
   //      checkbox.onchange = async () => {
   //    if (checkbox.checked) {
   //        await btn_subscribe_pause (checkbox.id);
   //    } else {
   //        await btn_subscribe_resume(checkbox.id);
   //    }

   //    }
   //      label.id = `consumer-stats-${track.id}`;
        
   //      if(track.kind === 'video') {
   //      label.innerHTML = "Pause " + track.kind;
   //      }
   //      else if(track.kind === 'audio') {
   //      label.innerHTML = "Mute " + track.kind;

   //      }

   //      var divStore = document.createElement('fieldset');

   //      let statButton;
   //      if(track.kind === 'video') {
   //        statButton = document.createElement('button');
   //        statButton.id=track.id;
   //        statButton.innerHTML += 'video Stats';
   //        statButton.onclick = function(){
   //           // alert('here be dragons');return false;
   //            btn_subscribe_stats(statButton.id);
   //            return false;
   //        };


   //          var labelName = document.createElement("label");
   //          labelName.innerHTML =  track.id.substring(41);
   //          //divStore.className="divTableRow";
   //          divStore.appendChild(labelName);

   //    }


   //      pause.appendChild(checkbox);
   //      pause.appendChild(label);
  

   //      // pause.appendChild(checkbox);
   //      divStore.appendChild(pause);

   //      if(statButton)
   //    divStore.appendChild(statButton);


    
   //      if(track.kind === 'video')
   //      {
   //          var td = document.createElement('div');
   //          td.className ="box";
   //          td.id = `box-${track.id}`;
   //          td.appendChild(div);
            
   //          td.appendChild(divStore);   
   //          //objJson[track.id] = td;

   //          //changePage(current_page);
   //           document.getElementById("traddCtrl2").append(td);
           
   //      }

   //      if (track.kind === 'audio') {

   //          var trExt = document.createElement('tr');
   //          trExt.id = `ConAudiostream-${track.id}`;

   //          var tr = document.createElement('tr');


   //          var divLevel = document.createElement('hr');
   //          divLevel.className = "new4";
   //          //divLevel.id=`consoundLevel-${track.id}`;
   //          divLevel.id=`consoundLevel-${track.id.substring(0, 36)}`;
   //          // tr.appendChild(divLevel);

   //          var labelName = document.createElement("label");
   //          //labelName.id = `conNameAud-${stream.id}`;
   //          labelName.innerHTML =  track.id.substring(41)
            

   //          var tr = document.createElement('fieldset');
   //          // var td = document.createElement('td');

   //          //var trImg = document.createElement('img');
   //         // trImg.src ="speaker.png"
   //          //tr.appendChild(trImg);
            
   //          tr.appendChild(labelName);
   //          tr.appendChild(divLevel);
   //          tr.appendChild(div);

   //          // var para = document.createElement("P");
   //          // para.innerHTML = "<span> <small> trackid:" +  track.id  + "<br>"+  "peerID:" +  stream.id  + "<br>" +   "</small> </span>";
  
            
   //          tr.appendChild(divStore);

   //          trExt.appendChild(tr);

   //          //trExt.id = 'constd' + track.id;
   //         // trExt.class='tr';
   //          //trExt.style.width = "200px";

   //          document.getElementById("traddCtrl0").append(trExt);

   //      }
   //      // else if(track.kind === 'video')
   //      // $('#traddCtrl2').append(td);
   //  }



   //  stream.onaddtrack = (event) =>{ 
   //  console.log("stream.onaddtrack " + event.track.kind)
   //  return;
   //  }



   //  }//if(transceiver
   //  stream.onremovetrack = (event) =>{

   //   console.log("stream.onremovetrack " + track.id);

   //  var parentVideo = document.getElementById("traddCtrl2");
   //  var childVideo = document.getElementById(`box-${track.id}`);
   //  if (parentVideo != null && childVideo != null) {
   //      parentVideo.removeChild(childVideo);
   //  }


   //  var len1 = Object.keys(objJson).length

   //  if (objJson.hasOwnProperty(track.id))
   //  {
   //       console.log("found it");
   //       delete objJson[track.id];
   //  }


   //  var len2 = Object.keys(objJson).length


   //  var parentAudio = document.getElementById("traddCtrl0");
   //  var childAudio = document.getElementById(`ConAudiostream-${track.id}`);
   //  if (parentAudio != null && childAudio != null) {
   //      parentAudio.removeChild(childAudio);
   //  }




   
   //  }
   //  transceiver.receiver.track.onmute = () => console.log("transceiver.receiver.track.onmute " + track.id);
   //  transceiver.receiver.track.onended = () => {


   //       var parentVideo = document.getElementById("traddCtrl2");
   //      var childVideo = document.getElementById(`box-${track.id}`);
   //      if (parentVideo != null && childVideo != null) {
   //          parentVideo.removeChild(childVideo);
   //      }

   //      var parentAudio = document.getElementById("traddCtrl0");
   //      var childAudio = document.getElementById(`ConAudiostream-${track.id}`);
   //      if (parentAudio != null && childAudio != null) {
   //          parentAudio.removeChild(childAudio);
   //      }


   //      console.log("transceiver.receiver.track.onended " + track.id)
   //  };
   //  transceiver.receiver.track.onunmute = (event) => {
   //  console.log("transceiver.receiver.track.onunmute " + track.id);
   //  var elVideo = document.getElementById(`va-${track.id}`); 
   //  if(elVideo != null)  
   //  elVideo.srcObject = new MediaStream([ track.clone() ]);

   //  };


   //  };


 