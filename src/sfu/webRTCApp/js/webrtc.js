'use strict';

  var dataChannelLabel = "chat";


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

let channelSnd =null;
let channelRec =null;
//var localStream;
//var track;

//var remoteStream;
//var turnReady;

var roomId = document.getElementById("roomCtrl").options[document.getElementById("roomCtrl").selectedIndex].text;

function roomCtrlOnChange() {
  var x = document.getElementById("roomCtrl").value;
  roomId = x;
}

//var  remotePeerID;
var  peerID;
var  remotePeerName;
var  peerName;


let pc1 =null;
let pc2 = null;

var flipSoundLevel = 0;

var socket = io.connect();


socket.on('created', function(room) {
  console.log('Created room ' + room);
  isInitiator = true;
});

socket.on('full', function(room) {
  console.log('Room ' + room + ' is full');
});

socket.on('join', function (room, id, numClients){
  //console.log('Another peer made a request to join room ' + room);
  console.log('This peer is the initiator of room ' + room + '!' +" client id " + id);
  isChannelReady = true;
  document.getElementById("divParticipants").innerHTML = numClients;

  // ////////////////////
  //
  //   sendMessage ({
  //       room: room,
  //       
  //       to: remotePeerID,
  //       type: "subscribe",
  //       desc: id
  //   });
  // ////////////////////

});

socket.on('leave', function (room, id, numClients){
  //console.log('Another peer made a request to join room ' + room);
  if( id == -1 && numClients -1 )
  {

     var str = "SFU server is down.";
     var result = str.fontcolor("red");
    document.getElementById("divStatus").innerHTML = result;
  }
  else
  {
      console.log("Disconnect client id " + id);
      isChannelReady = true;
      document.getElementById("divParticipants").innerHTML = numClients;
  }
});



socket.on('joined', function(room, id, numClients) {
 console.log('joined: ' + room + ' with peerID: ' + id);
 //log('joined: ' + room + ' with peerID: ' + id);
  isChannelReady = true;
  peerID = id;

   initPC();
    // Handle RTCPeerConnection connection status.
  // var str = numClients;
   //var result = str.fontcolor("green");
   document.getElementById("divParticipants").innerHTML = numClients;

});

 socket.on('disconnect', function () {

});


function initPC()
{
    pc2 = new RTCPeerConnection(
        {
            iceServers         : [],
            iceTransportPolicy : 'all',
            bundlePolicy       : 'max-bundle',
            rtcpMuxPolicy      : 'require',
            sdpSemantics       : 'unified-plan'
        });




    channelRec = pc2.createDataChannel(dataChannelLabel);
    channelRec.binaryType = 'arraybuffer';
    setupRecDataChannel(channelRec);
    
 
    pc2.ondatachannel = function (event) {
          if (event.channel.label == dataChannelLabel) {
            dataChannel = event.channel;
            console.log("DataChannel received");
            setupDataChannel(event.channel);
          } else {
            console.log("Unknown CataChannel label: " + event.channel.label);
          }
        }
       
    pc2.ontrack = ({transceiver, streams: [stream]}) => {


    if(transceiver.direction != 'inactive' && transceiver.currentDirection != 'inactive')
    {   
    var track = transceiver.receiver.track;
    console.log("pc.ontrack with transceiver and streams " + track.kind);


    if (track.kind === 'video' || track.kind === 'audio') {

        let el = document.createElement(track.kind);

        el.setAttribute('playsinline', true);
        el.setAttribute('autoplay', true);
        el.setAttribute('id', `va-${track.id}`);
        el.controls= true;
        //videoPlayerElement.load();
        //el.id = `video-${track.id}`;


        // set some attributes on our audio and video elements to make
        // mobile Safari happy. note that for audio to play you need to be


        var div = document.createElement('div');

        var para = document.createElement("P");
        para.innerHTML = "<span> <small> trackid:" +  track.id  + "<br>"+  "peerID:" +  stream.id  + "<br>" +   "</small> </span>";
        div.appendChild(para);
        

        //div.textContent = `streamid-${stream.id}`
       // div.potato= store;

        div.appendChild(el);

        div.id = `consumer-div-${track.id.substring(0, 36)}`;


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
        
        if(track.kind === 'video') {
        label.innerHTML = "Pause " + track.kind;
        }
        else if(track.kind === 'audio') {
        label.innerHTML = "Mute " + track.kind;

        }

        var divStore = document.createElement('fieldset');

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


            var labelName = document.createElement("label");
            labelName.innerHTML =  track.id.substring(41);
            //divStore.className="divTableRow";
            divStore.appendChild(labelName);

    	}


        pause.appendChild(checkbox);
        pause.appendChild(label);
  

        // pause.appendChild(checkbox);
        divStore.appendChild(pause);

        if(statButton)
    	divStore.appendChild(statButton);


    
        if(track.kind === 'video')
        {
            var td = document.createElement('div');
            td.className ="box";
            td.id = `box-${track.id}`;
            td.appendChild(div);
            
            td.appendChild(divStore);   
            //objJson[track.id] = td;

            //changePage(current_page);
             document.getElementById("traddCtrl2").append(td);
           
        }

        if (track.kind === 'audio') {

            var trExt = document.createElement('tr');
            trExt.id = `ConAudiostream-${track.id}`;

            var tr = document.createElement('tr');


            var divLevel = document.createElement('hr');
            divLevel.className = "new4";
            //divLevel.id=`consoundLevel-${track.id}`;
            divLevel.id=`consoundLevel-${track.id.substring(0, 36)}`;
            // tr.appendChild(divLevel);

            var labelName = document.createElement("label");
            //labelName.id = `conNameAud-${stream.id}`;
            labelName.innerHTML =  track.id.substring(41)
            

            var tr = document.createElement('fieldset');
            // var td = document.createElement('td');

            //var trImg = document.createElement('img');
           // trImg.src ="speaker.png"
            //tr.appendChild(trImg);
            
            tr.appendChild(labelName);
            tr.appendChild(divLevel);
            tr.appendChild(div);

            // var para = document.createElement("P");
            // para.innerHTML = "<span> <small> trackid:" +  track.id  + "<br>"+  "peerID:" +  stream.id  + "<br>" +   "</small> </span>";
  
            
            tr.appendChild(divStore);

            trExt.appendChild(tr);

            //trExt.id = 'constd' + track.id;
           // trExt.class='tr';
            //trExt.style.width = "200px";

            document.getElementById("traddCtrl0").append(trExt);

        }
        // else if(track.kind === 'video')
        // $('#traddCtrl2').append(td);
    }



    stream.onaddtrack = (event) =>{ 
    console.log("stream.onaddtrack " + event.track.kind)
    return;
    }



    }//if(transceiver
    stream.onremovetrack = (event) =>{

     console.log("stream.onremovetrack " + track.id);

    var parentVideo = document.getElementById("traddCtrl2");
    var childVideo = document.getElementById(`box-${track.id}`);
    if (parentVideo != null && childVideo != null) {
        parentVideo.removeChild(childVideo);
    }


    var len1 = Object.keys(objJson).length

    if (objJson.hasOwnProperty(track.id))
    {
         console.log("found it");
         delete objJson[track.id];
    }


    var len2 = Object.keys(objJson).length


    var parentAudio = document.getElementById("traddCtrl0");
    var childAudio = document.getElementById(`ConAudiostream-${track.id}`);
    if (parentAudio != null && childAudio != null) {
        parentAudio.removeChild(childAudio);
    }




   
    }
    transceiver.receiver.track.onmute = () => console.log("transceiver.receiver.track.onmute " + track.id);
    transceiver.receiver.track.onended = () => {


         var parentVideo = document.getElementById("traddCtrl2");
        var childVideo = document.getElementById(`box-${track.id}`);
        if (parentVideo != null && childVideo != null) {
            parentVideo.removeChild(childVideo);
        }

        var parentAudio = document.getElementById("traddCtrl0");
        var childAudio = document.getElementById(`ConAudiostream-${track.id}`);
        if (parentAudio != null && childAudio != null) {
            parentAudio.removeChild(childAudio);
        }


        console.log("transceiver.receiver.track.onended " + track.id)
    };
    transceiver.receiver.track.onunmute = (event) => {
    console.log("transceiver.receiver.track.onunmute " + track.id);
    var elVideo = document.getElementById(`va-${track.id}`); 
    if(elVideo != null)  
    elVideo.srcObject = new MediaStream([ track.clone() ]);

    };


    };




    pc2.addEventListener('iceconnectionstatechange', () =>
    {
        switch (pc2.iceConnectionState)
        {
            case 'checking':
                console.log( 'PC2 subscribing...');
                break;
            case 'connected':

              console.log( 'PC2 subscribed & connected');
               break;

            case 'completed':

                console.log( 'PC2 subscribed & completed');
                break;
            case 'failed':
               // pc2.close();

                console.log( 'PC2 failed...');
                break;
            case 'disconnected':
               // pc2.close();
                console.log( 'PC2 disconnected...');
                break;
            case 'closed':
                //pc2.close();
                console.log( 'PC2 closed...');
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


     pc1.ondatachannel = function (event) {
          if (event.channel.label == dataChannelLabel) {
            dataChannel = event.channel;
            console.log("DataChannel received");
            setupDataChannel(event.channel);
          } else {
            console.log("Unknown CataChannel label: " + event.channel.label);
          }
    }


    
    

    // channelSnd.onopen = function(event)
    // {
    //     //channelSnd.send('Hi you!');
    // }
    
    // channelSnd.onmessage = function(event)
    // {
    //     console.log(event.data);

    //     wirtechanneldata(event.data);
    // }


    // Handle RTCPeerConnection connection status.
    pc1.addEventListener('iceconnectionstatechange', () =>
    {
        switch (pc1.iceConnectionState)
        {
            case 'checking':
                console.log( 'PC1 publishing...');
                break;
            case 'connected':

              console.log( 'PC1 published & connected');
               break;

            case 'completed':

                console.log( 'PC1 published & completed');
                break;

            case 'failed':
               // pc1.close();
                console.log( 'PC1 failed...');
                break;
            case 'disconnected':
               // pc1.close();
                console.log( 'PC1 disconnected...');
                break;
            case 'closed':
               // pc1.close();
                 console.log( 'PC1 closed...');
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

    console.log( " Pc2 offers %o", sdp.sdp);

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
  
  //console.log('Client received message:', message.type);
  if (message.type === 'offer') {
     console.log('Client received message:', message.type);
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

            const myNodeAudio = document.getElementById("traddCtrl0");
            while (myNodeAudio.firstChild) {
                myNodeAudio.removeChild(myNodeAudio.lastChild);
            }

            
            addProducerVideoAudio();
                        // publish_simulcast(mss[ts]);
                 


            }, function (error) {

            console.error(error);

        });

  } else if (message.type === 'bye' && isStarted) {
    handleRemoteHangup();
  } else if (message.type === 'soundlevel' && isStarted) {
    soundlevel(message.desc);
  } else if (message.type === 'user' && isStarted) {
    //setName(message);
  } else if (message.type === 'chat' ) {
    message.to = socket.id;
    displayMessage(message);
  }else if (message.type === 'score' && isStarted) {
    score(message.desc);
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

   
        console.log('Sending answer to peer.');
        sendMessage({
            room: roomId,
            type: answer.type,
            desc: answer
        });



    console.log("answer %o", answer);



    //$('#traddCtrl2').remove();
/*
    const myNode = document.getElementById("traddCtrl2");
    while (myNode.firstChild) {
        myNode.removeChild(myNode.lastChild);
    }


    var transceivers = pc2.getTransceivers();

    for (var tsn in transceivers) {

        if(transceivers[tsn].direction != 'inactive' && transceivers[tsn].currentDirection != 'inactive')
            addConumerVideoAudio(transceivers[tsn]);
    }
  */  

    return  true;
}

// function removeElement(elementId) {
//     // Removes an element from the document
//     var element = document.getElementById(elementId);
//     element.parentNode.removeChild(element);
// }

function addProducerVideoAudio() {

    var transceivers = pc1.getTransceivers();

    //var store={};
 

    for (var ts in transceivers) {
        if( transceivers[ts].currentDirection != 'inactive' &&  transceivers[ts].direction != 'inactive') {
            var track = transceivers[ts].sender.track;
            //store [track.kind] = track.id;

            if (track.kind === 'video') {

                var divtd = document.createElement('div');
                divtd.className="box";


                var divStore = document.createElement('div');
                divStore.className="divTableRow";

                let el = document.createElement("video");

                el.setAttribute('playsinline', true);
                el.setAttribute('autoplay', true);

                //el.class='video';

                el.style.width = "100px";
                el.style.hight = "100px";

                var div = document.createElement('div');
                //div.textContent = track.id;
                //div.potato= store;
                var name = document.createElement("label");
                //name.type = "text";
                //name.id = `proName-${peerID}`;
                name.innerHTML = "me";
       

                var para = document.createElement("P");
                para.innerHTML = "<span> <small> videotrackid:" +  track.id  + "<br>"+  "peerID:" +  peerID  + "<br>" +   "</small> </span>";
                div.appendChild(para);

                div.appendChild(el);


                //div.style.width = "200px";

                divStore.appendChild(div);


                var closeButton = document.createElement('button');
                    closeButton.innerHTML += 'close';
                    closeButton.id=track.id;
                    closeButton.onclick = async function(){
                    var mss = pc1.getTransceivers();
                    for (var ts in mss) {
                        let ltrack = mss[ts].sender.track;

                        if ( ltrack && this.id === ltrack.id) {
                            console.log("close track " + ltrack.id);
                            mss[ts].sender.replaceTrack(null);

                            pc1.removeTrack(mss[ts].sender);

                            mss[ts].direction = "inactive";
                            if ( ts > 0) {
                                let strack = mss[ts-1].sender.track;
                                if ( strack && strack.kind == "audio") {
                                    console.log("close track " + strack.id);
                                    mss[ts-1].sender.replaceTrack(null);

                                    pc1.removeTrack(mss[ts-1].sender);

                                    mss[ts-1].direction = "inactive";

                                }

                                /////////////
                                document.getElementById("btn_webcam").disabled = true;
                                document.getElementById("btn_screen").disabled = true;
                                ////////////
                            }

                            //  this._remoteSdp.closeMediaSection(transceiver.mid)
                        }
                    }

                    var offer1 = await pc1.createOffer();

                    console.log("after close offer %o ", offer1.sdp);

                    await pc1.setLocalDescription(offer1);

                    sendMessage ({
                        room: roomId,
                        //to: remotePeerID,
                        type: offer1.type,
                        desc: offer1
                    });



                    // removeElement('producertd' );
                    return false;

                };


                divStore.appendChild(name);

                

                let pause = document.createElement('span'),
                checkbox = document.createElement('input'),
                label = document.createElement('label');
                pause.classList = 'nowrap';
                checkbox.type = 'checkbox';
                checkbox.id=track.id;
                checkbox.checked = false;
                checkbox.onchange = async () => {
                if (checkbox.checked) {
                    await btn_publisher_pause (checkbox.id);
                } else {
                    await btn_publisher_resume(checkbox.id);
                }

                }
                label.id = `video-check-${track.id}`;
                label.innerHTML = "Pause " + track.kind;

                divtd.appendChild(divStore);

                divtd.appendChild(closeButton);

                divtd.appendChild(checkbox);
                divtd.appendChild(label);
                divtd.appendChild(pause);

                ///////////////////////////////
                // let statButton;
              
                // statButton = document.createElement('button');
                // statButton.id=track.id;
                // statButton.innerHTML += 'video Stats';
                // statButton.onclick = function(){
                //     // alert('here be dragons');return false;
                //     btn_producer_stats(statButton.id);
                //     return false;
                // };

                // if(statButton)
                // divStore.appendChild(statButton);
                //////////////////////////////

                


                const streamV = new MediaStream();
                streamV.addTrack(track);

                el.srcObject = streamV;

                el.play()
                .then(()=>{})
                .catch((e) => {
                   console.log("play eror %o ", e);

                });


                divtd.id = 'td' + track.id;

                document.getElementById("traddCtrl1").append(divtd);
               
            }
            else if (track.kind === 'audio') {

                var trExt = document.createElement('tr');

                var divLevel = document.createElement('div');
                divLevel.className = "mystyle";
                //divLevel.id=`consoundLevel-${track.id}`;
               // tr.appendChild(divLevel);
                
                var labelName = document.createElement("label");
                labelName.id = `prodNameAud-${peerID}`;
                labelName.innerHTML =  "me";


                var tr = document.createElement('fieldset');
               // var td = document.createElement('td');

                var trImg = document.createElement('img');
                trImg.src ="speaker.png"
                tr.appendChild(trImg);
                tr.appendChild(labelName);
                tr.appendChild(divLevel);
                //tr.appendChild(td);

                var divStore = document.createElement('div');

                var div = document.createElement('div');

                var para = document.createElement("P");
                para.innerHTML = "<span> <small> audiotrackid:" +  track.id  + "<br>"+  "peerID:" +  peerID  + "<br>" +   "</small> </span>";
                div.appendChild(para);

               // div.style.width = "200px";
                divStore.appendChild(div);

  
                let pause = document.createElement('span'),
                checkbox = document.createElement('input'),
                label = document.createElement('label');
                pause.classList = 'nowrap';
                checkbox.type = 'checkbox';
                checkbox.id=track.id;
                checkbox.checked = false;
                checkbox.onchange = async () => {
                if (checkbox.checked) {
                    await btn_publisher_pause (checkbox.id);
                } else {
                    await btn_publisher_resume(checkbox.id);
                }

                }
                label.id = `audio-check-${track.id}`;
                label.innerHTML = "Mute " + track.kind;

                pause.appendChild(checkbox);
                pause.appendChild(label);
                divStore.appendChild(pause);

                tr.appendChild(divStore);

                trExt.appendChild(tr);

                trExt.id = 'prodtd' + track.id;
                trExt.class='tr';
                //trExt.style.width = "200px";

                document.getElementById("traddCtrl0").append(trExt);

            }


        }//end if active 
    } // end for transcievers

   // var track = ms;


 

    return true;

}


async function getUserMedia1( isWebcam) {


  let stream;
  try {

  //stream =  await navigator.mediaDevices.getUserMedia({ audio: true, video: true });

   stream = isWebcam ?
      await navigator.mediaDevices.getUserMedia({ audio: true, video: {width: {ideal: 320}, height: {ideal: 240}} }) :
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
           type: "subscribe-resume",
           desc: consumerid
         });
}

async function btn_subscribe_pause(consumerid ) {

     sendMessage ({
           room: roomId,
           type: "subscribe-pause",
           desc: consumerid
         });
}

async function btn_publisher_resume(producerid ) {

     sendMessage ({
           room: roomId,
           type: "publisher-resume",
           desc: producerid
         });
}

async function btn_publisher_pause(producerid ) {

     sendMessage ({
           room: roomId,
           type: "publisher-pause",
           desc: producerid
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

  if(channelSnd == null)
  {
      channelSnd = pc1.createDataChannel(dataChannelLabel);
      setupSendDataChannel(channelSnd);
  }

  if(pc1 == null)
  {

    var str = "Not connected yet, press connect button first";
    var result = str.fontcolor("red");
    document.getElementById("divStatus").innerHTML = result;

    return ;
  }
  else
    document.getElementById("divStatus").innerHTML = "publishing";

  var parser = new URL(window.location.href); 
  var istcp = parser.searchParams;
  const tcpValue = istcp.get('forceTcp') ? true : false;


   let stream;

   stream =  await getUserMedia1(isWebcam);
   let  videotrack;
   let audiotrack;

  

   if (document.getElementById("chk_video").checked)
   videotrack = stream.getVideoTracks()[0];

   if ( document.getElementById("chk_audio").checked )
    audiotrack = stream.getAudioTracks()[0];




        //var encodings;
        var _stream = new MediaStream();

        if (pc1.addTransceiver){

        }
        else
        {
             console.log(" pc.addTransceiver is not supported in this broswer, do pc.addTrack")
            /// else do pc.addTrack
        }

        if(audiotrack) {
            var transceiver1 = pc1.addTransceiver(
                audiotrack,
                {
                    direction: 'sendonly',
                    streams: [_stream]
                });

            //transceiver1.sender.setStreams(_stream);
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

        console.log( 'PC1 offer made:%o', offer.sdp ); 
  
         await pc1.setLocalDescription(offer);

        // We can now get the transceiver.mid.
        //const localId = transceiver.mid;

        //console.log("arvind transceiver.mid " + transceiver.mid);

        isStarted = true;

        //document.querySelector('#local_video').srcObject = stream;


        sendMessage ({
          room: roomId,
          //to: remotePeerID,
          type: pc1.localDescription.type,
          desc: pc1.localDescription
        });




 
}
/////////////////////////////End Publish


async function subscribe() {


  if(pc2 == null)
  {

    var str = "Not connected yet, press connect button first";
    var result = str.fontcolor("red");
    document.getElementById("divStatus").innerHTML = result;

    return ;
  }
  else
    document.getElementById("divStatus").innerHTML = "subscribed";

  var parser = new URL(window.location.href); 
  var istcp = parser.searchParams;
  const tcpValue = istcp.get('forceTcp') ? true : false;


  sendMessage ({
          room: roomId,
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
          type: "rtpObserver_addProducer",
        });
}

async function btn_audio_level_stop()
{

  sendMessage ({
          room: roomId,
          type: "rtpObserver_removeProducer",
        });
}




async function btn_producer_stats(producerid)
{

  // sendMessage ({
  //         room: roomId,
  //         type: "producer_getStats",
  //         desc: producerid
  //       });

}

async function btn_subscribe_stats(consumerid)
{

  // sendMessage ({
  //         room: roomId,
  //         type: "consumer_getStats",
  //         desc: consumerid
  //       });

}

function score(message)
{
    //console.log(message);

    document.getElementById("divProducerScore").innerHTML = message.producerScore;
    document.getElementById("divConsumerScore").innerHTML = message.score;
}


// function setName(message)
// {
   
//     //console.log("setName %o" + message.from);
//    // alert(message);

//    var nameElV = document.getElementById(`conNameVid-${message.from}`); 
//    if(nameElV != null)
//    {  
//         nameElV.innerHTML =  message.desc;
//    }


//    var nameElA = document.getElementById(`conNameAud-${message.from}`); 
//    if(nameElA != null)
//    {  
//         nameElA.innerHTML =  message.desc;
//    }
   
// }

function soundlevel(message)
{
    //console.log(message);

  
    flipSoundLevel = (++flipSoundLevel)%2;


    for (const element of message) 
    {
       //console.log(element.producerId);

       var nameEl = document.getElementById(`consumer-div-${element.producerId}`); 
       if(nameEl != null)
       {    
           if(flipSoundLevel==1)
            nameEl.style.border = "thick dashed #00FF00   ";
           else
            nameEl.style.border = "thick dashed #00FFFF   ";

       }

       var audLevel = document.getElementById(`consoundLevel-${element.producerId}`); 
       if(audLevel != null)
       {    
           //console.log(`${ 70 + element.volume}px`);
           audLevel.style.width =  `${ 70 + element.volume}px` ;
       }
       
   }


  sound_level.innerHTML = JSON.stringify(message, undefined, 4); 

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


////////////////////////// chat start ////////////////////////////////////////
function btn_chat()
{  
       
        if(pc2 == null)
        {

            var str = "Not connected yet, press connect button first";
            var result = str.fontcolor("red");
            document.getElementById("divStatus").innerHTML = result;

            return ;
        }
        else
        {
            document.getElementById("divStatus").innerHTML = "Chatting";

        }

        var send = {
                        room: roomId,
                        type: 'chat',
                        desc: document.getElementById("chatInput").value
                    };

        socket.emit('postAppMessage', send);

        document.getElementById("chatInput").value = "";
}




function displayMessage(data) {
      console.log( "chat %o", data);
      console.log( "chat text " + data.desc);

      let divClass = "";
      //verify that the user ID and the message sent ID is similar 
      if (data.to === data.from) {
          console.log("This person has sent a message")
        divClass = "myDiv";
      } else {
        divClass = "yourDiv";
      }
      const div = document.createElement("div");
      div.className = divClass;
      const li = document.createElement("li");
      //const p = document.createElement("p");
     // p.className = "time";
      var dd = new Date();
      dd = dd.getHours() + ":" + dd.getMinutes();
      div.innerHTML =
        '<p class="chatmessage"> ' + data.user + "[" + dd + "] " + 
        data.desc +
        "</p>";
      //div.appendChild(p);
      li.appendChild(div);

      document.getElementById("chatmessages").appendChild(li);

      var myDiv = document.getElementById("chatDiv");
      myDiv.scrollTop = myDiv.scrollHeight;


    }


    //////////////////////////////chat end ////////////////////////////////////


// function btn_senddata()
// {
//     var sendme = document.getElementById("dataInput").value ;
//     channelSnd.send(sendme);
// }





// function wirtechanneldata(x) {
//   var node = document.createElement("LI");
//   var textnode = document.createTextNode(x);
//   node.appendChild(textnode);
//   document.getElementById("dataList").appendChild(node);
// }





/**
* Add the various callback handlers to the DataChannel.
* Shared between both clients.
*/
// var setupDataChannel = function (dataChannel) {
// dataChannel.onopen = function (e) {
//   console.log("DataChannel open and ready to be used");

//   // $("#send_datachannel_msg").click(function () {
//   //   var msg = $("#datachannel_msg").val();
//   //   console.log("Sending message: " + msg);
//   //   dataChannel.send(msg);
//   // });
// };

// dataChannel.onclose = function () {
//   console.log("DataChannel closed");
// };

// dataChannel.onerror = function (e) {
//   console.log("DataChannel error: " + e.message);
//   console.log(e);
// };

// dataChannel.onmessage = function (e) {
//   console.log("Received message: " + e.data);
//   // if (sourceBuffer != null) {
//   //   sourceBuffer.appendBuffer(e.data);
//   // } else {
//   //   console.log("Got data but sourceBuffer is null");
//   // }
// };
// };














////////////////////////////////////////////// file transfer//////////////////////////////////////////////////////////


/*
 * https://raw.githubusercontent.com/webrtc/samples/gh-pages/src/content/datachannel/filetransfer/js/main.js
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
'use strict';


//let channelSnd;
//let channelRec;
let fileReader;
const bitrateDiv = document.querySelector('div#bitrate');
const fileInput = document.querySelector('input#fileInput');
const abortButton = document.querySelector('button#abortButton');
const downloadAnchor = document.querySelector('a#download');
const sendProgress = document.querySelector('progress#sendProgress');
const receiveProgress = document.querySelector('progress#receiveProgress');
const statusMessage = document.querySelector('span#status');
const sendFileButton = document.querySelector('button#sendFile');

let receiveBuffer = [];
let receivedSize = 0;

let bytesPrev = 0;
let timestampPrev = 0;
let timestampStart;
let statsInterval = null;
let bitrateMax = 0;

sendFileButton.addEventListener('click', () => createConnection());
fileInput.addEventListener('change', handleFileInputChange, false);
abortButton.addEventListener('click', () => {
  if (fileReader && fileReader.readyState === 1) {
    console.log('Abort read!');
    fileReader.abort();
  }
});

async function handleFileInputChange() {


  initSendDataChannel();


  const file = fileInput.files[0];
  if (!file) {
    console.log('No file chosen');
  } else {
    sendFileButton.disabled = false;
  }
}

async function createConnection() {
  abortButton.disabled = false;
 // sendFileButton.disabled = true;


  //pc1 = new RTCPeerConnection();
  //console.log('Created local peer connection object pc1');

  // channelSnd = pc1.createDataChannel('sendDataChannel');
  // channelSnd.binaryType = 'arraybuffer';
  // console.log('Created send data channel');

  // channelSnd.addEventListener('open', onSendChannelStateChange);
  // channelSnd.addEventListener('close', onSendChannelStateChange);
  // channelSnd.addEventListener('error', onError);

  // pc1.addEventListener('icecandidate', async event => {
  //   console.log('Local ICE candidate: ', event.candidate);
  //   await pc2.addIceCandidate(event.candidate);
  // });

  // pc2 = new RTCPeerConnection();
  // console.log('Created remote peer connection object pc2');

  // pc2.addEventListener('icecandidate', async event => {
  //   console.log('Remote ICE candidate: ', event.candidate);
  //   await pc1.addIceCandidate(event.candidate);
  // });
  // pc2.addEventListener('datachannel', receiveChannelCallback);

  // try {
  //   const offer = await pc1.createOffer();
  //   await gotLocalDescription(offer);
  // } catch (e) {
  //   console.log('Failed to create session description: ', e);
  // }


  onSendChannelStateChange();

  //channelSnd.send("ravind");

  fileInput.disabled = true;
}

async function initSendDataChannel()
{
    if(channelSnd)
        return;

      pc1.ondatachannel = function (event) {
          if (event.channel.label == dataChannelLabel) {
            dataChannel = event.channel;
            console.log("DataChannel received");
            setupDataChannel(event.channel);
          } else {
            console.log("Unknown CataChannel label: " + event.channel.label);
          }
    }


    channelSnd = pc1.createDataChannel(dataChannelLabel);
    channelSnd.binaryType = 'arraybuffer';
    setupSendDataChannel(channelSnd);
    

     var offer = await pc1.createOffer();

        console.log( 'PC1 offer made:%o', offer.sdp ); 
  
         await pc1.setLocalDescription(offer);

        // We can now get the transceiver.mid.
        //const localId = transceiver.mid;

        //console.log("arvind transceiver.mid " + transceiver.mid);

        isStarted = true;

        //document.querySelector('#local_video').srcObject = stream;


        sendMessage ({
          room: roomId,
          //to: remotePeerID,
          type: pc1.localDescription.type,
          desc: pc1.localDescription
        });

}

function sendData() {
  const file = fileInput.files[0];
  console.log(`File is ${[file.name, file.size, file.type, file.lastModified].join(' ')}`);

  // Handle 0 size files.
  statusMessage.textContent = '';
  downloadAnchor.textContent = '';
  if (file.size === 0) {
    bitrateDiv.innerHTML = '';
    statusMessage.textContent = 'File is empty, please select a non-empty file';
    //closeDataChannels();
    return;
  }
  sendProgress.max = file.size;
  receiveProgress.max = file.size;
  const chunkSize = 16384;
  fileReader = new FileReader();
  let offset = 0;
  fileReader.addEventListener('error', error => console.error('Error reading file:', error));
  fileReader.addEventListener('abort', event => console.log('File reading aborted:', event));
  fileReader.addEventListener('load', e => {
    console.log('FileRead.onload ', e);
    channelSnd.send(e.target.result);
    offset += e.target.result.byteLength;
    sendProgress.value = offset;
    if (offset < file.size) {
      readSlice(offset);
    }
  });
  const readSlice = o => {
    console.log('readSlice ', o);
    const slice = file.slice(offset, o + chunkSize);
    fileReader.readAsArrayBuffer(slice);
  };
  readSlice(0);
}

function closeDataChannels() {

  if(channelSnd)
  {
    console.log('Closing data channels');
    channelSnd.close();
    console.log(`Closed data channel with label: ${channelSnd.label}`);
    channelSnd = null;
    console.log('Closed peer connections');
 }
  // if (channelRec) {
  //   channelRec.close();
  //   console.log(`Closed data channel with label: ${channelRec.label}`);
  //   channelRec = null;
  // }
  // pc1.close();
  // pc2.close();
  // pc1 = null;
  // pc2 = null;


  // re-enable the file select
  fileInput.disabled = false;
  abortButton.disabled = true;
  sendFileButton.disabled = false;
}

// async function gotLocalDescription(desc) {
//   await pc1.setLocalDescription(desc);
//   console.log(`Offer from pc1\n ${desc.sdp}`);
//   await pc2.setRemoteDescription(desc);
//   try {
//     const answer = await pc2.createAnswer();
//     await gotRemoteDescription(answer);
//   } catch (e) {
//     console.log('Failed to create session description: ', e);
//   }
// }

// async function gotRemoteDescription(desc) {
//   await pc2.setLocalDescription(desc);
//   console.log(`Answer from pc2\n ${desc.sdp}`);
//   await pc1.setRemoteDescription(desc);
// }

// function receiveChannelCallback(event) {
//   console.log('Receive Channel Callback');
//   channelRec = event.channel;
//   channelRec.binaryType = 'arraybuffer';
//   channelRec.onmessage = onReceiveMessageCallback;
//   channelRec.onopen = onReceiveChannelStateChange;
//   channelRec.onclose = onReceiveChannelStateChange;

//   receivedSize = 0;
//   bitrateMax = 0;
//   downloadAnchor.textContent = '';
//   downloadAnchor.removeAttribute('download');
//   if (downloadAnchor.href) {
//     URL.revokeObjectURL(downloadAnchor.href);
//     downloadAnchor.removeAttribute('href');
//   }
// }

function onReceiveMessageCallback(event) {
  console.log(`Received Message ${event.data.byteLength}`);
  receiveBuffer.push(event.data);
  receivedSize += event.data.byteLength;
  receiveProgress.value = receivedSize;

  // we are assuming that our signaling protocol told
  // about the expected file size (and name, hash, etc).
  //const file = fileInput.files[0];
  //if (receivedSize === file.size)
   var filename = channelRec.label;
   {
    const received = new Blob(receiveBuffer);
    receiveBuffer = [];

    downloadAnchor.href = URL.createObjectURL(received);
    downloadAnchor.download = filename;
    downloadAnchor.textContent =
      `Click to download '${filename}' (${receivedSize} bytes)`;
    downloadAnchor.style.display = 'block';

    const bitrate = Math.round(receivedSize * 8 /
      ((new Date()).getTime() - timestampStart));
    bitrateDiv.innerHTML =
      `<strong>Average Bitrate:</strong> ${bitrate} kbits/sec (max: ${bitrateMax} kbits/sec)`;

    // if (statsInterval) {
    //   clearInterval(statsInterval);
    //   statsInterval = null;
    // }

    //closeDataChannels();
  }
}

function onSendChannelStateChange() {
  if (channelSnd) {
    const {readyState} = channelSnd;
    console.log(`Send channel state is: ${readyState}`);
    if (readyState === 'open') {
      sendData();

      // if (statsInterval) 
      // {
      //   clearInterval(statsInterval);
      //   statsInterval = null;
      // }


    }
  }
}

function onError(error) {
  /*if (channelSnd) {
    console.error('Error in channelSnd:', error);
    return;
  }*/
  console.log('Error in channelSnd which is already closed:', error);
}

async function onReceiveChannelStateChange() {
  if (channelRec) {
    const readyState = channelRec.readyState;
    console.log(`Receive channel state is: ${readyState}`);
    if (readyState === 'open') {
      timestampStart = (new Date()).getTime();
      timestampPrev = timestampStart;
      // statsInterval = setInterval(displayStats, 500);
      // await displayStats();
    }
  }
}

// display bitrate statistics.
async function displayStats() {
  if (pc2 && pc2.iceConnectionState === 'connected') {
    const stats = await pc2.getStats();
    let activeCandidatePair;
    stats.forEach(report => {
      if (report.type === 'transport') {
        activeCandidatePair = stats.get(report.selectedCandidatePairId);
      }
    });
    if (activeCandidatePair) {
      if (timestampPrev === activeCandidatePair.timestamp) {
        return;
      }
      // calculate current bitrate
      const bytesNow = activeCandidatePair.bytesReceived;
      const bitrate = Math.round((bytesNow - bytesPrev) * 8 /
        (activeCandidatePair.timestamp - timestampPrev));
      bitrateDiv.innerHTML = `<strong>Current Bitrate:</strong> ${bitrate} kbits/sec`;
      timestampPrev = activeCandidatePair.timestamp;
      bytesPrev = bytesNow;
      if (bitrate > bitrateMax) {
        bitrateMax = bitrate;
      }
    }
  }
}





/**
* Add the various callback handlers to the DataChannel.
* Shared between both clients.
*/
var setupSendDataChannel = function (dataChannel) {
dataChannel.onopen = function (e) {
  console.log("DataChannel open and ready to be used for sending file");

  // $("#send_datachannel_msg").click(function () {
  //   var msg = $("#datachannel_msg").val();
  //   console.log("Sending message: " + msg);
  //   dataChannel.send(msg);
  // });

    

};

dataChannel.onclose = function () {

  onSendChannelStateChange();
  console.log("DataChannel closed");
};

dataChannel.onerror = function (e) {
  console.log("DataChannel error: " + e.message);

  onError();
  console.log(e);
};

dataChannel.onmessage = function (e) {
  console.log("Received message: " + e.data);
  // if (sourceBuffer != null) {
  //   sourceBuffer.appendBuffer(e.data);
  // } else {
  //   console.log("Got data but sourceBuffer is null");
  // }
};
};





/**
* Add the various callback handlers to the DataChannel.
* Shared between both clients.
*/



var setupRecDataChannel = function (dataChannel) {
dataChannel.onopen = function (e) {
  console.log("DataChannel open and ready to be used for receiving files");

  // $("#send_datachannel_msg").click(function () {
  //   var msg = $("#datachannel_msg").val();
  //   console.log("Sending message: " + msg);
  //   dataChannel.send(msg);
  // });


  receivedSize = 0;
  bitrateMax = 0;
  downloadAnchor.textContent = '';
  downloadAnchor.removeAttribute('download');
  if (downloadAnchor.href) {
    URL.revokeObjectURL(downloadAnchor.href);
    downloadAnchor.removeAttribute('href');
  }

  onReceiveChannelStateChange();


};

dataChannel.onclose = function () {
  onReceiveChannelStateChange();
  console.log("DataChannel closed");
};

dataChannel.onerror = function (e) {
  console.log("DataChannel error: " + e.message);

  onError();
  console.log(e);
};

dataChannel.onmessage = function (e) {
  console.log("Received message: " + e.data);

  onReceiveMessageCallback(e);

  // if (sourceBuffer != null) {
  //   sourceBuffer.appendBuffer(e.data);
  // } else {
  //   console.log("Got data but sourceBuffer is null");
  // }
};
};


