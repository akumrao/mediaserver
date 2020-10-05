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

var roomId = 'foo'; /*think as a group  peerName@room */
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

                pc2Connected = true;
                console.log( 'subscribed...');

                break;
            case 'failed':
                pc2.close();

                console.log( 'failed...');
                break;
            case 'disconnected':
                pc2.close();
                console.log( 'Peerconnection disconnected...');
                break;
            case 'closed':
                pc2.close();
                console.log( 'failed...');
                break;
        }
    });



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

                console.log( 'published...');
                break;
            case 'failed':
                pc1.close();
                console.log( 'failed...');
                break;
            case 'disconnected':
                pc1.close();
                console.log( 'failed...');
                break;
            case 'closed':
                pc1.close();
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

  console.log('sending message: ', message.type);
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
function subscribe_simulcast(trackid)
{
    for (let s of CAM_VIDEO_SIMULCAST_ENCODINGS) {

        var div = document.createElement('div');
        var radio = document.createElement('input');
        var label = document.createElement('label');
        radio.type = 'radio';
        radio.name = `radioConsumer`;


        radio.onchange = () => {
            var desc= {};
            desc["id"]=trackid;

            var radioButtons = document.getElementsByName("radioConsumer");
            for(var i = 0; i < radioButtons.length; i++) {
                if (radioButtons[i].checked == true) {
                    desc["data"]={"spatialLayer":i};
                    sendMessage({
                        room: roomId,
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
  console.log('Client received message:', message.type);

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
            subscribe();
           //////


            const myNode = document.getElementById("traddCtrl1");
            while (myNode.firstChild) {
                myNode.removeChild(myNode.lastChild);
            }

            var transceivers = pc1.getTransceivers();

            var store={};

            for (var ts in transceivers) {
                if( transceivers[ts].currentDirection != 'inactive' &&  transceivers[ts].direction != 'inactive') {
                    var track = transceivers[ts].sender.track;
                    store [track.kind] = track.id;
                    if (track.kind === 'video') {

                        addProducerVideoAudio(track, store);
                        // publish_simulcast(mss[ts]);
                        store={};
                    }
                }
            }


            }, function (error) {

            console.error(error);

        });

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

//var trackNo= -1;

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
            room: roomId,
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


    console.log("answer %o", answer);



    //$('#traddCtrl2').remove();

    const myNode = document.getElementById("traddCtrl2");
    while (myNode.firstChild) {
        myNode.removeChild(myNode.lastChild);
    }


    var transceivers = pc2.getTransceivers();

    for (var tsn in transceivers) {

        if(transceivers[tsn].direction != 'inactive' && transceivers[tsn].currentDirection != 'inactive')
            addConumerVideoAudio(transceivers[tsn]);
    }

    return  true;
}

function removeElement(elementId) {
    // Removes an element from the document
    var element = document.getElementById(elementId);
    element.parentNode.removeChild(element);
}

function addProducerVideoAudio(track, store) {


   // var track = ms;

    var divStore = document.createElement('div');



    let el = document.createElement("video");

    el.setAttribute('playsinline', true);
    el.setAttribute('autoplay', true);

    el.class='video';

    el.style.width = "200px";
    el.style.hight = "200px";

    var div = document.createElement('div');
   // div.textContent = track.id;
    div.potato= store;

    var para = document.createElement("P");
    para.innerHTML = "<span> <small> trackid:" +  track.id  + "<br>"+  "peerID:" +  peerID  + "<br>" +   "</small> </span>";
    div.appendChild(para);

    div.appendChild(el);
    div.style.width = "200px";


    divStore.appendChild(div);

    let statButton;

    if(track.kind === 'video') {
        statButton = document.createElement('button');
        statButton.id=track.id;
        statButton.innerHTML += 'video Stats';
        statButton.onclick = function(){
            // alert('here be dragons');return false;
            btn_producer_stats(statButton.id);
            return false;
        };
    }

    if(statButton)
        divStore.appendChild(statButton);

    var closeButton = document.createElement('button');
    closeButton.innerHTML += 'close';
    closeButton.onclick = async function(){

        var mss = pc1.getTransceivers();

        // console.log( "ravind1 : %o", mss);

        for (var ts in mss) {


            let ltrack = mss[ts].sender.track;

            if ( ltrack && store[ltrack.kind] === ltrack.id) {

                mss[ts].sender.replaceTrack(null);

                pc1.removeTrack(mss[ts].sender);

                mss[ts].direction = "inactive";

                //  this._remoteSdp.closeMediaSection(transceiver.mid)


            }
        }

        var offer1 = await pc1.createOffer();


        console.log("after close offer %o ", offer1);

        await pc1.setLocalDescription(offer1);

        sendMessage ({
            room: roomId,
            from: peerID,
            //to: remotePeerID,
            type: offer1.type,
            desc: offer1
        });





        btn_producer_close(store);
        // removeElement('producertd' );
        return false;

    };

    divStore.appendChild(closeButton);

    var td = document.createElement('td');
    td.appendChild(divStore);




    td.id = 'td' + track.id;
    td.class='td';
    td.style.width = "200px";

    $('#traddCtrl1').append(td);


    const streamV = new MediaStream();
    streamV.addTrack(track);

    el.srcObject = streamV;

    el.play()
        .then(()=>{})
        .catch((e) => {
            err(e);
        });


    return true;

}

function addConumerVideoAudio(transiver) {


    var store={};

    var track = transiver.receiver.track;

    if(track.kind === 'audio')
        return ;


    var mss = pc2.getRemoteStreams();
    let ms;
    for( var msn in mss   )
    {
       var lms = mss[msn];

       var tracks = lms.getTracks();
       for(var tc of tracks )
       {
           if ( tc ==track)
           {
               ms = lms;
           }
       }

       if(ms)
           break;
    }

    store [track.kind] = track.id;


    let el = document.createElement(track.kind);
    // set some attributes on our audio and video elements to make
    // mobile Safari happy. note that for audio to play you need to be
    // capturing from the mic/camera
    el.setAttribute('playsinline', true);
    el.setAttribute('autoplay', true);


    var div = document.createElement('div');
    div.textContent = ms.id;
    div.potato= store;

    div.appendChild(el);



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

    let statButton;
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


        pause.appendChild(checkbox);
        pause.appendChild(label);


    var divStore = document.createElement('div');

   // pause.appendChild(checkbox);
    divStore.appendChild(pause);

    if(statButton)
        divStore.appendChild(statButton);


    var td = document.createElement('td');
    td.appendChild(div);
    td.appendChild(divStore);

    $('#traddCtrl2').append(td);


    el.srcObject = new MediaStream([ track.clone() ]);
    // let's "yield" and return before playing, rather than awaiting on
    // play() succeeding. play() will not succeed on a producer-paused
    // track until the producer unpauses.
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
          room: roomId,
          from: peerID,
          type: "subscribe-resume",
          desc: consumerid
        });
}

async function btn_subscribe_pause(consumerid ) {

    sendMessage ({
          room: roomId,
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




        //var encodings;
        var _stream = new MediaStream();


        if(audiotrack) {
            var transceiver1 = pc1.addTransceiver(
                audiotrack,
                {
                    direction: 'sendonly',
                    streams: [_stream]
                });

            transceiver1.sender.setStreams(_stream);
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
             let checkBox = document.getElementById("chk_simulcast");
             if ( checkBox && checkBox.checked && isWebcam ) {

                 var transceiver = pc1.addTransceiver(videotrack, {
                     direction: 'sendonly',
                     streams: [_stream],
                     sendEncodings:CAM_VIDEO_SIMULCAST_ENCODINGS
                 });
                // transceiver.sender.setStreams(_stream);

             } else {
                 var transceiver = pc1.addTransceiver(
                     videotrack,
                     {
                         direction: 'sendonly',
                         streams: [_stream]
                     });
                 //transceiver.sender.setStreams(_stream);
             }
         }

         

        var offer = await pc1.createOffer();

        console.log( "PC1 offer made: %o", offer);

         await pc1.setLocalDescription(offer);

        // We can now get the transceiver.mid.
        //const localId = transceiver.mid;

        //console.log("arvind transceiver.mid " + transceiver.mid);

        isStarted = true;

        //document.querySelector('#local_video').srcObject = stream;


        sendMessage ({
          room: roomId,
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
          room: roomId,
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
          room: roomId,
          from: peerID,
          to: remotePeerID,
          type: "rtpObserver_addProducer",
        });
}

async function btn_audio_level_stop()
{

  sendMessage ({
          room: roomId,
          from: peerID,
          to: remotePeerID,
          type: "rtpObserver_removeProducer",
        });
}


async function btn_producer_close(producerids)
{

    sendMessage ({
        room: roomId,
        from: peerID,
        type: "producer_close",
        desc: producerids
    });

}

async function btn_producer_stats(producerid)
{

  sendMessage ({
          room: roomId,
          from: peerID,
          type: "producer_getStats",
          desc: producerid
        });

}

async function btn_subscribe_stats(consumerid)
{

  sendMessage ({
          room: roomId,
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


