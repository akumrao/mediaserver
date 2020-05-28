'use strict';


////////////////////////////////////////////////////////////////////////////////////////////
/*
* Error produced when calling a method in an invalid state.
*/
class InvalidStateError extends Error
{
    constructor(message)
    {
        super(message);

        this.name = 'InvalidStateError';

        if (Error.hasOwnProperty('captureStackTrace')) // Just in V8.
            Error.captureStackTrace(this, InvalidStateError);
        else
            this.stack = (new Error(message)).stack;
    }
}

class AwaitQueue
{
    constructor({ ClosedErrorClass = Error } = {})
    {
        // Closed flag.
        // @type {Boolean}
        this._closed = false;

        // Queue of pending tasks. Each task is a function that returns a promise
        // or a value directly.
        // @type {Array<Function>}
        this._tasks = [];

        // Error used when rejecting a task after the AwaitQueue has been closed.
        // @type {Error}
        this._closedErrorClass = ClosedErrorClass;
    }

    close()
    {
        this._closed = true;
    }

    /**
     * @param {Function} task - Function that returns a promise or a value directly.
     *
     * @async
     */
    async push(task)
    {
        if (typeof task !== 'function')
            throw new TypeError('given task is not a function');

        return new Promise((resolve, reject) =>
        {
            task._resolve = resolve;
            task._reject = reject;

            // Append task to the queue.
            this._tasks.push(task);

            // And run it if the only task in the queue is the new one.
            if (this._tasks.length === 1)
                this._next();
        });
    }

    async _next()
    {
        // Take the first task.
        const task = this._tasks[0];

        if (!task)
            return;

        // Execute it.
        await this._runTask(task);

        // Remove the first task (the completed one) from the queue.
        this._tasks.shift();

        // And continue.
        this._next();
    }

    async _runTask(task)
    {
        if (this._closed)
        {
            task._reject(new this._closedErrorClass('AwaitQueue closed'));

            return;
        }

        try
        {
            const result = await task();

            if (this._closed)
            {
                task._reject(new this._closedErrorClass('AwaitQueue closed'));

                return;
            }

            // Resolve the task with the given result (if any).
            task._resolve(result);
        }
        catch (error)
        {
            if (this._closed)
            {
                task._reject(new this._closedErrorClass('AwaitQueue closed'));

                return;
            }

            // Reject the task with the error.
            task._reject(error);
        }
    }
}
/////////////////////////////////////////////////////////////////////////////////////


//const $chkSimulcast = $('#chk_simulcast');

var sound_level = document.getElementById("sound_level"); 
var prod_stat = document.getElementById("prod_stat"); 
var cons_stat = document.getElementById("cons_stat"); 

var isChannelReady = true;
var isInitiator = false;
var isStarted = false;
//var localStream;
//var track;

//var remoteStream;
//var turnReady;

var room = 'foo'; /*think as a group  peerName@room */
//var  remotePeerID;
var  peerID;
var  remotePeerName;
var  peerName;


var pc1;
var pc2;
var pc2Connected= false;

var socket = io.connect();


socket.on('created', function(room) {
  console.log('Created room ' + room);
  isInitiator = true;
});

socket.on('full', function(room) {
  console.log('Room ' + room + ' is full');
});

socket.on('join', function (room, id){
  //console.log('Another peer made a request to join room ' + room);
  console.log('This peer is the initiator of room ' + room + '!' +" client id " + id);
  isChannelReady = true;

  // ////////////////////
  //
  //   sendMessage ({
  //       room: room,
  //       from: peerID,
  //       to: remotePeerID,
  //       type: "subscribe",
  //       desc: id
  //   });
  // ////////////////////


});

socket.on('joined', function(room, id) {
 console.log('joined: ' + room + ' with peerID: ' + id);
 //log('joined: ' + room + ' with peerID: ' + id);
  isChannelReady = true;
  peerID = id;


   initPC2();
    // Handle RTCPeerConnection connection status.


});



function initPC2()
{
    pc2 = new RTCPeerConnection(
        {
            iceServers         : [],
            iceTransportPolicy : 'all',
            bundlePolicy       : 'max-bundle',
            rtcpMuxPolicy      : 'require',
            sdpSemantics       : 'unified-plan'
        });

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
                pc2Connected = true;
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
                console.log( 'Peerconnection disconnected...');
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


}


socket.on('log', function(array) {
  console.log.apply(console, array);
});

////////////////////////////////////////////////

function sendMessage(message) {
  console.log('Client sending message: ', message);
  socket.emit('sfu-message', message);
}


async  function processOffer( remotePeerID,  sdp)
{

    console.log( " Pc2 offers %o", sdp);

    await pc2.setRemoteDescription(new RTCSessionDescription(sdp));


    const ret = await doAnswer(remotePeerID);

    return ret;
}




//////////////////////////////////////////////////////////////////////


function publish_simulcast(transceiver)
{

    for (let s of CAM_VIDEO_SIMULCAST_ENCODINGS) {

        var div = document.createElement('div');
        var radio = document.createElement('input');
        var label = document.createElement('label');
        radio.type = 'radio';
        radio.name = `radioProducer`;
        // radio.checked = currentLayer == undefined ?
        //     (i === stats.length-1) :
        //     (i === currentLayer);
        radio.onchange = () => {
            console.log('ask server to set layers ' + i);
            // sig('consumer-set-layers', { consumerId: consumer.id,
            //  spatialLayer: i });

            const parameters =  transceiver.sender.getParameters();
            var radioButtons = document.getElementsByName("radioProducer");
            for(var i = 0; i < radioButtons.length; i++) {
                if (radioButtons[i].checked == true) {
                    parameters.encodings[i].active = true;

                } else
                    parameters.encodings[i].active = false;
            }

            transceiver.sender.setParameters(parameters);

        };
        // let bitrate = Math.floor(s.bitrate / 1000);
        label.innerHTML = s.rid + " " + ( (s.scaleResolutionDownBy != null) ? s.scaleResolutionDownBy:'full') ;

        div.appendChild(radio);
        div.appendChild(label);
        //container.appendChild(div);
        $('#local_video').append(div);

    }

}

var _awaitQueue = new AwaitQueue({ ClosedErrorClass: InvalidStateError });

// This client receives a message
socket.on('message',  async function(message) {
  console.log('Client received message:', message);

  if (message.type === 'offer') {

      return _awaitQueue.push(
          async () =>  processOffer(message.from, message.desc  ));
     //await processOffer(message.from, message.desc  );

  } else if (message.type === 'answer' && isStarted) {
    //remotePeerID=message.from;
    console.log("publish andwer %o", message)
    pc1.setRemoteDescription(new RTCSessionDescription(message.desc))
        .then(function ()
        {
           // subscribe();
           //////

            var mss = pc1.getTransceivers()

            var spatialLayer =0;

            for (var ts in mss) {


                var ms = mss[ts].sender.track;
                if(ms.kind === 'video' ) {

                    const parameters =  mss[ts].sender.getParameters();
                    parameters.encodings.forEach((encoding, idx) => {
                        // if (idx <= spatialLayer)
                        //     encoding.active = true;
                        // else
                        //     encoding.active = false;

                        console.log("idx %o, encoding %o", idx, encoding );
                    });
                    //mss[ts].sender.setParameters(parameters);

                    addProducerVideoAudio(ms);
                    publish_simulcast(mss[ts]);


                }
            }

           //////



              }, function (error) {

            console.error(error);

        });



  } else if (message.type === 'candidate' && isStarted) {
    var candidate = new RTCIceCandidate({
      sdpMLineIndex: message.candidate.sdpMLineIndex,
      sdpMid: message.candidate.sdpMid,
      candidate: message.candidate.candidate
    });
    pc.addIceCandidate(candidate);
  } else if (message.type === 'bye' && isStarted) {
    handleRemoteHangup();
  } else if (message.type === 'soundlevel' && isStarted) {
    soundlevel(message.desc);
  }else if (message.type === 'prodstats' && isStarted) {
    prodstats(message.desc);
  }else if (message.type === 'constats') {
    constats(message.desc);
  }

});

var trackNo= -1;

async function sleep(ms) {
    return new Promise((r) => setTimeout(() => r(), ms));
}

async function doAnswer(remotePeerID) {


  const answer = await  pc2.createAnswer();

  await pc2.setLocalDescription(answer);

    if(!pc2Connected) {
        pc2Connected = true;
        console.log('Sending answer to peer.');
        sendMessage({
            room: room,
            from: peerID,
            to: remotePeerID,
            type: answer.type,
            desc: answer
        });
    }


    // while (!pc2Connected) {
    //     console.log(' transport connstate', pc2Connected );
    //     await sleep(100);
    //
    // }


    console.log("answer %o", answer.type);




    // console.log( "transceivers %o", transceivers);
    // if (!transceivers)
    //     throw new Error('new RTCRtpTransceiver not found');


    var mss = pc2.getRemoteStreams();


    for (var ts in mss) {
        if (ts > trackNo) {
           // console.log("ts%o:%o, ", ts, mss[ts]);

            var ms = mss[ts];
            trackNo = ts;
            addConumerVideoAudio(ms);
        }
    }





    return  true;
 }


function addProducerVideoAudio(ms) {

    var store={};

    var track = ms;

    var divStore = document.createElement('div');

    var statButton;


        store[track.kind] = track.id

        // let pause = document.createElement('span'),
        //     checkbox = document.createElement('input'),
        //     label = document.createElement('label');
        // pause.classList = 'nowrap';
        // checkbox.type = 'checkbox';
        // checkbox.id=track.id;
        // checkbox.checked = false;
        // checkbox.onchange = async () => {
        //     if (checkbox.checked) {
        //         await btn_subscribe_pause (checkbox.id);
        //     } else {
        //         await btn_subscribe_resume(checkbox.id);
        //     }
        //
        // }
        // label.id = `consumer-stats-${track.id}`;
        // label.innerHTML = "Pause " + track.kind;


        if(track.kind === 'video') {
            statButton = document.createElement('button');
            statButton.id=track.id;
            statButton.innerHTML += 'video Stats';
            statButton.onclick = function(){
                // alert('here be dragons');return false;
                btn_producer_stats(statButton.id);
                return false;
            };




        // if (consumer.paused) {
        //     label.innerHTML = '[consumer paused]'
        // } else {
        //     let stats = lastPollSyncData[myPeerId].stats[consumer.id],
        //         bitrate = '-';
        //     if (stats) {
        //         bitrate = Math.floor(stats.bitrate / 1000.0);
        //     }
        //     label.innerHTML = `[consumer playing ${bitrate} kb/s]`;
        // }
        //pause.appendChild(checkbox);
        //pause.appendChild(label);
       // pause.appendChild(checkbox);
       // divStore.appendChild(pause);


    }

    if(statButton)
        divStore.appendChild(statButton);

    let el = document.createElement("video");
// set some attributes on our audio and video elements to make
// mobile Safari happy. note that for audio to play you need to be
// capturing from the mic/camera
    el.setAttribute('playsinline', true);
    el.setAttribute('autoplay', true);


    var div = document.createElement('div');
    div.textContent = ms.id;
    div.potato= store;

    div.appendChild(el);
    var td = document.createElement('td');

    td.appendChild(div);
    td.appendChild(divStore);

    $('#local_video').append(td);

    const streamV = new MediaStream();
    streamV.addTrack(ms);

    el.srcObject = streamV;

    el.play()
        .then(()=>{})
        .catch((e) => {
            err(e);
        });

    return true;

}
function subscribe_simulcast(trackid)
{
    for (let s of CAM_VIDEO_SIMULCAST_ENCODINGS) {

        var div = document.createElement('div');
        var radio = document.createElement('input');
        var label = document.createElement('label');
        radio.type = 'radio';
        radio.name = `radioConsumer`;
        // radio.checked = currentLayer == undefined ?
        //     (i === stats.length-1) :
        //     (i === currentLayer);

        radio.onchange = () => {
            var desc= {};
            desc["id"]=trackid;

            var radioButtons = document.getElementsByName("radioConsumer");
            for(var i = 0; i < radioButtons.length; i++) {
                if (radioButtons[i].checked == true) {
                    desc["data"]={"spatialLayer":i};
                    sendMessage({
                        room: room,
                        from: peerID,
                        type: 'setPreferredLayers',
                        desc: desc
                    });


                }
            }

        };
        // let bitrate = Math.floor(s.bitrate / 1000);
        label.innerHTML = s.rid + " " + ( (s.scaleResolutionDownBy != null) ? s.scaleResolutionDownBy:'full') ;
        div.appendChild(radio);
        div.appendChild(label);
        //container.appendChild(div);
        $('#TRSubscribe').append(div);

    }
}

function addConumerVideoAudio(ms) {

    var store={};

    var tracks = ms.getTracks();

    var divStore = document.createElement('div');

    var statButton;

    for( const tno in tracks)
    {
        var track = tracks[tno];
        store [track.kind] = track.id

        let pause = document.createElement('span'),
        checkbox = document.createElement('input'),
        label = document.createElement('label');
        pause.classList = 'nowrap';
        checkbox.type = 'checkbox';
        checkbox.id=track.id;
        checkbox.checked = false;
        checkbox.onchange = async () => {
            if (checkbox.checked) {
                await btn_subscribe_pause (checkbox.id);
            } else {
                await btn_subscribe_resume(checkbox.id);
            }

        }
        label.id = `consumer-stats-${track.id}`;
        label.innerHTML = "Pause " + track.kind;


        if(track.kind === 'video') {
            statButton = document.createElement('button');
            statButton.id=track.id;
            statButton.innerHTML += 'video Stats';
            statButton.onclick = function(){
               // alert('here be dragons');return false;
                btn_subscribe_stats(statButton.id);
                return false;
            };


        }


        // if (consumer.paused) {
        //     label.innerHTML = '[consumer paused]'
        // } else {
        //     let stats = lastPollSyncData[myPeerId].stats[consumer.id],
        //         bitrate = '-';
        //     if (stats) {
        //         bitrate = Math.floor(stats.bitrate / 1000.0);
        //     }
        //     label.innerHTML = `[consumer playing ${bitrate} kb/s]`;
        // }
        pause.appendChild(checkbox);
        pause.appendChild(label);
       // pause.appendChild(checkbox);
        divStore.appendChild(pause);

    }

    if(statButton)
    divStore.appendChild(statButton);

let el = document.createElement("video");
// set some attributes on our audio and video elements to make
// mobile Safari happy. note that for audio to play you need to be
// capturing from the mic/camera
el.setAttribute('playsinline', true);
el.setAttribute('autoplay', true);


var div = document.createElement('div');
div.textContent = ms.id;
div.potato= store;

div.appendChild(el);
var td = document.createElement('td');

td.appendChild(div);
td.appendChild(divStore);

$('#TRSubscribe').append(td);


subscribe_simulcast(store.video);

el.srcObject = ms;
el.play()
    .then(()=>{})
    .catch((e) => {
        err(e);
    });




return true;

}

function onCreateSessionDescriptionError(error) {
  //log('Failed to create session description: ' + error.toString());
  console.log('Failed to create session description: ' + error.toString());
  
}



async function getUserMedia1( isWebcam) {


  let stream;
  try {

  //stream =  await navigator.mediaDevices.getUserMedia({ audio: true, video: true });

   stream = isWebcam ?
      await navigator.mediaDevices.getUserMedia({ audio: true, video: true }) :
      await navigator.mediaDevices.getDisplayMedia({ audio: true, video: true });
  } catch (err) {
    console.error('getUserMedia() failed:', err.message);
    throw err;
  }
  return stream;
}

/////////////////////////////////////////////////////////////////////////////
async function btn_subscribe_resume(consumerid ) {

    sendMessage ({
          room: room,
          from: peerID,
          type: "subscribe-resume",
          desc: consumerid
        });
}

async function btn_subscribe_pause(consumerid ) {

    sendMessage ({
          room: room,
          from: peerID,
          type: "subscribe-pause",
          desc: consumerid
        });
}


const CAM_VIDEO_SIMULCAST_ENCODINGS =
[
    {rid: 'q', scaleResolutionDownBy: 4.0},
    {rid: 'h', scaleResolutionDownBy: 2.0},
    {rid: 'f'}
];
///////////////////////////////////////////////////////////////////////////

//var pc1
async function publish(isWebcam)
{
  // const isWebcam = (e.target.id === 'btn_webcam');
  // $txtPublish = isWebcam ? $txtWebcam : $txtScreen;

  var parser = new URL(window.location.href); 
  var istcp = parser.searchParams;
  const tcpValue = istcp.get('forceTcp') ? true : false;


   let stream;

   stream =  await getUserMedia1(isWebcam);
   let  videotrack;
   let audiotrack;



   if ($('#chk_video').prop('checked'))
   videotrack = stream.getVideoTracks()[0];

   if ($('#chk_audio').prop('checked'))
    audiotrack = stream.getAudioTracks()[0];

   //const params = { track };
//////////////////////////////////////////////////////////////////////////

// var objTo = document.getElementById('container');
//     var divtest = document.createElement("div");
//     divtest.innerHTML = "new div";
//     objTo.appendChild(divtest);


//     const $chkSimulcast = $('#chk_simulcast');
//     const chkSimulcast1 = $('#chk_simulcast');
//
//     if (chkSimulcast1.prop('checked')
// )   {
//          var x = 1;
//     }



  //
  // let el = document.createElement("video");
  // // set some attributes on our audio and video elements to make
  // // mobile Safari happy. note that for audio to play you need to be
  // // capturing from the mic/camera
  // el.setAttribute('playsinline', true);
  // el.setAttribute('autoplay', true);
  //
  // $('#local_video').append(el);
  // $('#divPeerid').text(peerID);

   // videoEl.appendChild(el);

 // el.srcObject = new MediaStream([ consumer.track.clone() ]);
 // el.consumer = consumer;
  // let's "yield" and return before playing, rather than awaiting on
  // play() succeeding. play() will not succeed on a producer-paused
  // track until the producer unpauses.
  // el.play()
  //   .then(()=>{})
  //   .catch((e) => {
  //     err(e);
  //   });




/////////////////////////////////////////////////////////////////////////////////


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

                    // const streamV = new MediaStream();
                    // streamV.addTrack(videotrack);
                    //
                    // el.srcObject = streamV;
                    //
                    // el.play()
                    //     .then(()=>{})
                    //     .catch((e) => {
                    //         err(e);
                    //     });


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

        //var encodings;
        var _stream = new MediaStream();


        if(audiotrack) {
            var transceiver1 = pc1.addTransceiver(
                audiotrack,
                {
                    direction: 'sendonly',
                    streams: [_stream]
                });
        }
         
         //firefox
        // var parameters = transceiver.sender.getParameters();
        // console.log("simulcast parameters %o", parameters);

        //  if (!parameters.encodings) {
        //  parameters.encodings = [{}];
        //  }

        // var encodings = [
        //           { rid: 'r0', maxBitrate: 100000 },
        //           { rid: 'r1', maxBitrate: 500000 }
        //       ];
        // parameters.encodings = encodings;
        // transceiver.sender.setParameters(parameters);


          // Mormal without simulcast
         if(videotrack ) {
             var checkBox = document.getElementById("chk_simulcast");
             if (checkBox.checked) {

                 var transceiver = pc1.addTransceiver(videotrack, {
                     direction: 'sendonly',
                     streams: [_stream],
                     sendEncodings:CAM_VIDEO_SIMULCAST_ENCODINGS
                 });


             } else {
                 var transceiver = pc1.addTransceiver(
                     videotrack,
                     {
                         direction: 'sendonly',
                         streams: [_stream]
                     });
             }
         }

         

        var offer = await pc1.createOffer();

        console.log( "publish offer: %o", offer);

         await pc1.setLocalDescription(offer);

        // We can now get the transceiver.mid.
        //const localId = transceiver.mid;

        //console.log("arvind transceiver.mid " + transceiver.mid);

        isStarted = true;

        //document.querySelector('#local_video').srcObject = stream;


        sendMessage ({
          room: room,
          from: peerID,
          //to: remotePeerID,
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
          type: "subscribe",
        });

 
}//end subscribe 



// async function pollit() 
// {


//       // super-simple signaling: let's poll at 1-second intervals
//       pollingInterval = setInterval(async () => {
//         let { error } = await pollAndUpdate();
//         if (error) {
//           clearInterval(pollingInterval);
//           err(error);
//         }
//       }, 1000);


// }//end simulcast 

async function btn_audio_level_start()
{

  sendMessage ({
          room: room,
          from: peerID,
          to: remotePeerID,
          type: "rtpObserver_addProducer",
        });
}

async function btn_audio_level_stop()
{

  sendMessage ({
          room: room,
          from: peerID,
          to: remotePeerID,
          type: "rtpObserver_removeProducer",
        });
}


async function btn_producer_stats(producerid)
{

  sendMessage ({
          room: room,
          from: peerID,
          type: "producer_getStats",
          desc: producerid
        });

}

async function btn_subscribe_stats(consumerid)
{

  sendMessage ({
          room: room,
          from: peerID,
          type: "consumer_getStats",
          desc: consumerid
        });

}


function soundlevel(level)
{
  //console.log(level);

  sound_level.innerHTML = JSON.stringify(level, undefined, 4); 
}

function prodstats(desc)
{
  //console.log(level);

  prod_stat.innerHTML = JSON.stringify(desc, undefined, 4); 
}

function constats(desc)
{
  //console.log(level);

  cons_stat.innerHTML = JSON.stringify(desc, undefined, 4); 
}


