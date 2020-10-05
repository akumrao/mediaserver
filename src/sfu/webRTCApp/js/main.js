

// Set up audio and video regardless of what devices are present.
/*var sdpConstraints = {
  offerToReceiveAudio: true,
  offerToReceiveVideo: true
};*/

/////////////////////////////////////////////


// Could prompt for room name:
// room = prompt('Enter room name:');




//var localVideo = document.querySelector('#localVideo');
//var remoteVideo = document.querySelector('#remoteVideo');

// navigator.mediaDevices.getUserMedia({
//   audio: true,
//   video: true
// })
// .then(gotStream)
// .catch(function(e) {
//   alert('getUserMedia() error: ' + e.name);
// });



/////////////////////////////////////////////////////////

// function createPeerConnection() {
//   try {
//     //pc = new RTCPeerConnection(null);

//     pc = new RTCPeerConnection(
//       {
//         iceServers         : [],
//         iceTransportPolicy : 'all',
//         bundlePolicy       : 'max-bundle',
//         rtcpMuxPolicy      : 'require',
//         sdpSemantics       : 'unified-plan'
//       });


//     pc2 = new RTCPeerConnection(
//       {
//         iceServers         : [],
//         iceTransportPolicy : 'all',
//         bundlePolicy       : 'max-bundle',
//         rtcpMuxPolicy      : 'require',
//         sdpSemantics       : 'unified-plan'
//       });


//      // pc.addTransceiver('audio');
//       //pc.addTransceiver('video');
//     // _stream = new MediaStream();


//     // const transceiver = pc.addTransceiver(
//     //     track,
//     //     {
//     //         direction     : 'sendonly',
//     //         streams       : [ _stream ],
//     //         sendEncodings : encodings
//     //     });


//     pc.onicecandidate = handleIceCandidate;
//     pc.onaddstream = handleRemoteStreamAdded;
//     pc.onremovestream = handleRemoteStreamRemoved;
//     console.log('Created RTCPeerConnnection');
//   } catch (e) {
//     console.log('Failed to create PeerConnection, exception: ' + e.message);
//     alert('Cannot create RTCPeerConnection object.');
//     return;
//   }
// }







$(document).ready(function(){
 

  $("#btn_connect").click(function(){

    console.log("arvind");
          $("#div1").html("<h2>btn_connect!</h2>");
   socket.emit('create or join', roomId);
 // const data = await socket.request('getRouterRtpCapabilities');

  });


  $("#btn_webcam").click(function(){

    console.log("btn_webcam click");

   publish(true);
  });

  $("#btn_screen").click(function(){

        console.log("btn_screen click");

        publish(false);
  });


  $("#btn_subscribe").click(function(){

    console.log("btn_subscribe click");

   subscribe();
  });

 $("#btn_subscribe_resume").click(function(){

    console.log("btn_subscribe_resume click");

   btn_subscribe_resume();
  });

  
  $("#btn_subscribe_pause").click(function(){

    console.log("btn_subscribe_pause click");

   btn_subscribe_pause();
  });


  $("#btn_audio_level_start").click(function(){

    console.log("btn_audio_level_start click");
    btn_audio_level_start();
  });

   $("#btn_audio_level_stop").click(function(){

    console.log("btn_audio_level_start click");

   btn_audio_level_stop();
  });


  $("#btn_producer_stats").click(function(){

    console.log("btn_producer_stats click");

   btn_producer_stats();
  });

  
  $("#btn_subscribe_stats").click(function(){

    console.log("btn_subscribe_stats click");
   btn_subscribe_stats();
  });




 });
