// (c) 2023 Johnson Controls.  All Rights reserved.

'use strict';

var isChannelReady = true;
var isInitiator = false;
var isStarted = false;
var pc;
var remoteStream;
var turnReady;

var roomId = 'VideoEdgeWebRTC'; /*think as a group  peerName@room */
var peerID;
var peerName;

var encType;

let ai = false;

var pcConfig = {
  'iceServers': [{
    'urls': 'stun:stun.l.google.com:19302'
  }]
};

var browserName = (function(agent) {
    switch (true) {
        case agent.indexOf("edge") > -1:
            return "Edge";
        case agent.indexOf("edg/") > -1:
            return "Edge ( chromium based)";
        case agent.indexOf("opr") > -1 && !!window.opr:
            return "Opera";
        case agent.indexOf("chrome") > -1 && !!window.chrome:
            return "Chrome";
        case agent.indexOf("trident") > -1:
            return "MS IE";
        case agent.indexOf("firefox") > -1:
            return "Firefox";
        case agent.indexOf("safari") > -1:
            return "Safari";
        default:
            return "other";
    }
})(window.navigator.userAgent.toLowerCase());

if (browserName == "Firefox")
    document.getElementById("encoder").value = "VP9"

let trackarr = [];
var socket = io.connect();
socket.on('created', function(room) {
    console.log('Created room ' + room);
    isInitiator = true;
});

socket.on('join', function(room, id, numClients) {
    console.log('New peer, room: ' + room + ', ' + " client id: " + id);
    isChannelReady = true;
});

socket.on('joined', function(room, id, numClients) {
    console.log('joined: ' + room + ' with peerID: ' + id);
    log('joined: ' + room + ' with peerID: ' + id);
    isChannelReady = true;
    peerID = id;

    let urlVars = getQueryParameters();

    let camid = urlVars["camera"];
    if (!camid) {
        return;
    }

    let startTime = urlVars["start"] ?? 0;
    let endTime = urlVars["end"]?? 0;
    let width = urlVars["width"]?? 0;
    let height = urlVars["height"]?? 0;
    let scale = urlVars["scale"]?? 1;
    let speed = urlVars["speed"]?? 1;
    let encoder = urlVars["encoder"]?? "NVIDIA";
    
    if (isInitiator) {
        offermsg(camid, startTime, endTime, width, height, speed, scale, encoder, ai);
    }
});

socket.on('log', function(array) {
    console.log.apply(console, array);
});


function getQueryParameters() {
    var queryParameters = {};

    function captureQueryParam(match, name, value)
    {
        queryParameters[name] = value;
    }

    window.location.href.replace(/[?&]+([^=&]+)=([^&]*)/gi, captureQueryParam);
    return queryParameters;
}


function sendMessage(message) {
    console.log('Client sending message: ', message);
    log('Client sending message: ', message);
    socket.emit('messageToWebrtc', message);
}

var text, parser, xmlDoc;
parser = new DOMParser();

function draw(text, cv, ctx) {
    xmlDoc = parser.parseFromString(text, "text/xml");

    ctx.clearRect(0, 0, cv.width, cv.height);

    let ObjectBox = xmlDoc.getElementsByTagName("Object");
    if (!ObjectBox) {
        return;
    }

    for (var ncount = 0; ncount < ObjectBox.length; ++ncount) {

        let boundingBox = ObjectBox[ncount].getElementsByTagName("BoundingBox")[0];

        if (!boundingBox) {
            continue;
        }

        var x0 = parseFloat(boundingBox.attributes["X0"].nodeValue);
        var y0 = parseFloat(boundingBox.attributes["Y0"].nodeValue);
        var x1 = parseFloat(boundingBox.attributes["X1"].nodeValue);
        var y1 = parseFloat(boundingBox.attributes["Y1"].nodeValue);

        let label = ObjectBox[ncount].getElementsByTagName("Classification")[0];
        if (label) {
            name = label.attributes["Label"].nodeValue;
        }

        let attrbute = ObjectBox[ncount].getElementsByTagName("Attribute")[0];
        if (attrbute) {
            var colorName = attrbute.attributes["Name"].nodeValue;
            var colorValue = attrbute.attributes["Value"].nodeValue;
            name += " " + colorName + " " + colorValue;
        }

        x0 = x0 * cv.width;
        y0 = y0 * cv.height;

        x1 = x1 * cv.width;
        y1 = y1 * cv.height;

        var w = x1 - x0;
        var h = y1 - y0;

        ctx.beginPath();
        ctx.rect(x0, y0, w, h);

        if (label) {
            ctx.fillText(name, x0 + 2, y0 + 10);
        }

        ctx.stroke();
    }
}

let SDPUtils;
socket.on('message', function(message) {
    console.log('Client received message:', message);
    log('Client received message:', message);

    if (message === 'got user media') {
        maybeStart();
    } else if (message.type === 'offer') {
        if (!isInitiator && !isStarted) {
            maybeStart();
        }

        var des = new RTCSessionDescription(message.desc);
        let mediaSections = SDPUtils.getMediaSections(des.sdp);
        const mediaSection = mediaSections[mediaSections.length - 1];
        const rtpParameters = SDPUtils.parseRtpParameters(mediaSection);

        let statsText = '';
        for (var i = 0; i < rtpParameters.codecs.length; i++) {
            var codecName = rtpParameters.codecs[i].name;
            var tmpObj = {};

            tmpObj["MaxEnc"] = rtpParameters.codecs[i].parameters["MaxEnc"];
            if (codecName == "VP9")
                tmpObj["SwEnc"] = rtpParameters.codecs[i].parameters["PresentEncIns"];
            else if (codecName == "H264") {
                encType = rtpParameters.codecs[i].parameters["Enc"];
                tmpObj[encType] = rtpParameters.codecs[i].parameters["PresentEncIns"];
            }
            if (codecName == "VP9" || codecName == "H264") {
                statsText += `<div>Encoder: ${ JSON.stringify(tmpObj)}</div>`;
            }
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
    } else if (message.type === 'error') {
        console.log('Camera state', message.desc);
        log('Camera state:', message.desc);
        hangup();
    }
});

var remoteVideo = document.querySelector('#remoteVideo');

isInitiator = true;

function maybeStart() {
    console.log('>>>>>>> maybeStart() ', isStarted, isChannelReady);
    if (!isStarted && isChannelReady) {
        console.log('>>>>>> creating peer connection');
        createPeerConnection();
        isStarted = true;
        console.log('isInitiator', isInitiator);

        if (roomId !== '') {
            socket.emit('create or join', roomId);
            console.log('Attempted to create or  join room', roomId);
        }
    }
}

window.onbeforeunload = function() {
    sendMessage({
        room: roomId,
        type: 'bye'
    });
};

var enc = new TextDecoder("utf-8");
if (!("TextDecoder" in window))
    alert("Sorry, this browser does not support TextDecoder...");

function videoAnalyzer(encodedFrame, controller, cv, ctx) {
    const view = new DataView(encodedFrame.data);
    const metasize = view.getUint32(encodedFrame.data.byteLength - 4);
    if (metasize < encodedFrame.data.byteLength && metasize < 2000) {
        const tag = view.getUint32(encodedFrame.data.byteLength - 8 - metasize);
        const metaData = encodedFrame.data.slice(encodedFrame.data.byteLength - 4 - metasize, encodedFrame.data.byteLength - 4);

        if (view.getUint32(0) == 1) { //  h264 start code '0001' 
            encodedFrame.data = encodedFrame.data.slice(0, encodedFrame.data.byteLength - 8 - metasize);

            var xmlDec = enc.decode(metaData);

            try {
                var xmlFrame = atob(xmlDec);
            } catch (err) {
                console.log("Metadata xml parser fails." + err);
                return;
            }

            draw(xmlFrame, cv, ctx);
        }
    }

    controller.enqueue(encodedFrame);
}

function gotRemoteTrack(receiver, cv) {
    console.log('pc2 received remote stream');

    var ctx = cv.getContext("2d");

    const frameStreams = receiver.createEncodedStreams();
    frameStreams.readable.pipeThrough(new TransformStream({
            transform: async function(encodedFrame, controller) {
                videoAnalyzer(encodedFrame, controller, cv, ctx);
            }
        }))
        .pipeTo(frameStreams.writable);
}

async function createPeerConnection() {
    try {
        // For debug, you can create a peer connection as follows.
        // pc = new RTCPeerConnection(null);
        if (ai)
            pc = new RTCPeerConnection({
                encodedInsertableStreams: true,
                iceServers: [{'urls': 'stun:stun.l.google.com:19302'}],
                iceTransportPolicy: 'all',
                bundlePolicy: 'max-bundle',
                rtcpMuxPolicy: 'require',
                sdpSemantics: 'unified-plan'
            });
        else
            pc = new RTCPeerConnection({
                iceServers: [{'urls': 'stun:stun.l.google.com:19302'}],
                iceTransportPolicy: 'all',
                bundlePolicy: 'max-bundle',
                rtcpMuxPolicy: 'require',
                sdpSemantics: 'unified-plan'
            });

        pc.onicecandidate = handleIceCandidate;
        pc.ontrack = ontrack;
        pc.addEventListener('iceconnectionstatechange', e => onIceStateChange(pc, e));
        console.log('Created RTCPeerConnnection');
    } catch (e) {
        console.log('Failed to create PeerConnection, exception: ' + e.message);
        alert('Cannot create RTCPeerConnection object.');
    }
}

function handleIceCandidate(event) {
    console.log('icecandidate event: ', event);
    if (event.candidate) {
        sendMessage({
            room: roomId,
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
    // for changing bandwidth,bitrate and audio stereo/mono
    // sessionDescription.sdp = sessionDescription.sdp.replace("useinbandfec=1", "useinbandfec=1; minptime=10; cbr=1; stereo=1; sprop-stereo=1; maxaveragebitrate=510000");
    // sessionDescription.sdp = sessionDescription.sdp.replace("useinbandfec=1", "useinbandfec=1; minptime=10; stereo=1; maxaveragebitrate=510000");

    sessionDescription.sdp = sessionDescription.sdp.replaceAll("level-asymmetry-allowed=1", "level-asymmetry-allowed=1; Enc=" + encType);
    pc.setLocalDescription(sessionDescription);
    console.log('setLocalAndSendMessage sending message', sessionDescription);

    sendMessage({
        room: roomId,
        type: sessionDescription.type,
        desc: sessionDescription
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
        type: 'bye'
    });
}

function handleRemoteHangup() {
    console.log('Session terminated.');
    stop();
}

function stop() {
    isStarted = false;
    pc.close();
    pc = null;
}

let streamV = new Map();

function ontrack({
    transceiver,
    receiver,
    streams: [stream]
}) {
    var track = transceiver.receiver.track;
    var trackid = stream.id;
    var tns =  transceiver;

    if (transceiver.direction != 'inactive' && transceiver.currentDirection != 'inactive' && track.kind == "video") {
        console.log("transceiver.receiver.track.onunmute " + trackid);
        var divtd = document.createElement('div');
        divtd.className = "box";

        var divStore = document.createElement('div');
        divStore.className = "divTableRow";

        let el = document.createElement("video");

        el.setAttribute('playsinline', true);
        el.setAttribute('autoplay', true);
        el.muted = true;
        el.id = `vd-${trackid}`;

        var width = document.getElementById("widthVideo").value;

        el.style.maxWidth = width + 'px';
        el.width = width + 'px';
        el.controls = false;

        var div = document.createElement('div');
        var name = document.createElement("label");

        name.innerHTML = "<span> <small> videotrackid:" + trackid + "<br>" + "peerID:" + peerID + "<br>" + "</small> </span>";

        var camid = document.getElementById("camId").value;

        let cv;
        if (ai) {
            cv = document.createElement("canvas");
            cv.id = "cv1";
            gotRemoteTrack(receiver, cv);
            cv.className = "canvas";
            div.appendChild(cv);
        }

        div.appendChild(el);

        divtd.onclick = async function() {
            if (divtd.style.backgroundColor == "red") {
                divtd.style.backgroundColor = "";

                const index = trackarr.indexOf(trackid);
                if (index > -1) { 
                    trackarr.splice(index, 1); 
                }
            } else {
                divtd.style.backgroundColor = "red";
                trackarr.push(trackid);
            }
        };

        divStore.appendChild(div);

        var closeButton = document.createElement('button');
        closeButton.innerHTML += 'close';
        closeButton.id = trackid;
        closeButton.onclick = async function() {

            streamV.delete(trackid);

            sendMessage({
                room: roomId,
                type: 'command',
                desc: 'close',
                trackids: [trackid],
                act: true
            });

            document.getElementById("traddCtrl1").removeChild(divtd);
            return false;
        };

        divStore.appendChild(name);

        let pause = document.createElement('span'),
            checkbox = document.createElement('input'),
            label = document.createElement('label');
        pause.classList = 'nowrap';
        checkbox.type = 'checkbox';
        checkbox.id = trackid;
        checkbox.checked = false;
        checkbox.onchange = async () => {
            sendMessage({
                room: roomId,
                type: 'command',
                desc: 'mute',
                trackids: [trackid],
                act: checkbox.checked
            });
        }
        label.id = `video-check-${trackid}`;
        label.innerHTML = "Pause " + track.kind;

        divtd.appendChild(divStore);
        divtd.appendChild(closeButton);
        divtd.appendChild(checkbox);
        divtd.appendChild(label);
        divtd.appendChild(pause);

        var trackk = streamV.get(trackid);
        // var audt = trackk.getAudioTracks();  // enable when both audio and vidoeo present

        // if (audt.length) {
        //     let pause1 = document.createElement('span'),
        //         checkbox1 = document.createElement('input'),
        //         label1 = document.createElement('label');
        //     pause1.classList = 'nowrap';
        //     checkbox1.type = 'checkbox';
        //     checkbox1.id = trackid + "_aud";
        //     checkbox1.checked = true;
        //     checkbox1.onchange = async () => {

        //         el.muted = (checkbox1.checked == true);

        //         sendMessage({
        //             room: roomId,
        //             type: 'command',
        //             desc: 'muteaudio',
        //             trackids: [trackid + "_aud"],
        //             act: checkbox1.checked
        //         });
        //     }
        //     label1.id = `audio-check-${trackid}`;
        //     label1.innerHTML = "Pause " + "audio";

        //     divtd.appendChild(checkbox1);
        //     divtd.appendChild(label1);
        //     divtd.appendChild(pause1);
        // }

        if (!streamV.has(trackid)) {
            streamV.set(trackid, new MediaStream());
        }

        streamV.get(trackid).addTrack(track);
        el.srcObject = streamV.get(trackid);

        el.play()
            .then(() => {
                if (cv) {
                    cv.width = el.offsetWidth;;
                    cv.height = el.offsetHeight
                }
            })
            .catch((e) => {
                console.log("play eror %o ", e);
            });

        divtd.id = 'td' + trackid;

        document.getElementById("traddCtrl1").append(divtd);

    } else if (transceiver.direction != 'inactive' && transceiver.currentDirection != 'inactive' && track.kind == "audio") {
        if (!streamV.has(trackid)) {
            streamV.set(trackid, new MediaStream());
        }
        streamV.get(trackid).addTrack(track);
    }

    stream.onaddtrack = () => console.log("stream.onaddtrack");
    stream.onremovetrack = () => console.log("stream.onremovetrack");
    transceiver.receiver.track.onmute = () => {

        console.log("transceiver.receiver.track.onmute " + trackid);

    }
    transceiver.receiver.track.onended = () => console.log("transceiver.receiver.track.onended " + trackid);
    transceiver.receiver.track.onunmute = () => {


        console.log("transceiver.receiver.track.onunmute " + trackid)


    };
}

function onIceStateChange(pc, event) {
    switch (pc.iceConnectionState) {
        case 'checking': {
            start();
            setupWebRtcPlayer(pc);
            onWebRtcAnswer();

            console.log('checking...');
        }
        break;
        case 'connected':
            console.log('connected...');
            break;
        case 'completed':
            console.log('completed...');
            break;
        case 'failed':
            console.log('failed...');
            break;
        case 'disconnected':
            console.log('Peerconnection disconnected...');
            break;
        case 'closed':
            console.log('failed...');
            break;
    }
}

function onMuteClick() {
    // we  might enable this code in future
    // var checkBox = document.getElementById("checkmute");
    // if (checkBox.checked == true) {
    //     text.style.display = "block";
    // } else {
    //     text.style.display = "none";
    // }

    var camids = document.getElementById("camId").value;

    if (!trackarr.length) {
        checkBox.checked = false;
        alert("Please click and select elements to pause");
        return;
    }

    sendMessage({
        room: roomId,
        type: 'command',
        desc: 'mute',
        trackids: trackarr,
        act: checkBox.checked
    });
}

function addAICamera() {
    let aiTmp = getQueryParameters()["ai"];
    if (aiTmp) {
        ai = true;
    }

    if (!ai) {
        alert("please add ?ai=true \n https://localhost:9093/?ai=true ");
        return;
    }

    if (isInitiator) {
        maybeStart();
    }

    var camid = document.getElementById("camId").value;
    var startTime = document.getElementById("startTime").value;

    var endTime = 0;

    var width = document.getElementById("widthVideo").value;
    var height = document.getElementById("heightVideo").value;
    var speed = document.getElementById("speed").value;

    if (startTime == "0" && speed != "1") {
        alert("Please enter start time for Speed > 1")
        document.getElementById("speed").value = 1;
        return;
    }

    var scale = document.getElementById("scale").value;
    var encoder = document.getElementById("encoder").value;

    offermsg(camid, startTime, endTime, width, height, speed, scale, encoder, ai);
}

function addCamera() {
    if (isInitiator) {
        maybeStart();
    }

    var camid = document.getElementById("camId").value;
    var startTime = document.getElementById("startTime").value;

    var endTime = 0;

    var width = document.getElementById("widthVideo").value;
    var height = document.getElementById("heightVideo").value;
    var speed = document.getElementById("speed").value;

    if (startTime == "0" && speed != "1") {
        alert("Please enter start time for Speed > 1")
        document.getElementById("speed").value = 1;
        return;
    }

    var scale = document.getElementById("scale").value;
    var encoder = document.getElementById("encoder").value;

    offermsg(camid, startTime, endTime, width, height, speed, scale, encoder, ai);
}

function applyCamera() {
    var camids = document.getElementById("camId").value;

    var endTime = 0;

    var width = document.getElementById("widthVideo").value;
    var height = document.getElementById("heightVideo").value;

    var speed = document.getElementById("speed").value;
    var scale = document.getElementById("scale").value;

    var startTime = document.getElementById("startTime").value;

    if (!trackarr.length) {
        alert("Please click and select elements to reverseplay or change resolution");
        return;
    }

    for (var x = 0; x < trackarr.length; ++x) {
        let el = document.getElementById(`vd-${trackarr[x]}`);

        el.style.maxWidth = width + 'px';
        el.width = width + 'px';
    }

    if (startTime == "0" && speed != "1") {
        alert("Please enter starttime for Speed > 1")
        document.getElementById("speed").value = 1;
        return;
    }

    sendMessage({
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
        act: true
    });
}

function forward() {
    document.getElementById("scale").value = 1;
    applyCamera();
}

function backward() {
    document.getElementById("scale").value = -1;
    applyCamera();
}

function offermsg(camid, startTime, endTime, width, height, speed, scale, encoder, ai) {
    sendMessage({
        room: roomId,
        cam: camid.toString(),
        start: startTime.toString(),
        end: endTime.toString(),
        width: width.toString(),
        height: height.toString(),
        speed: speed.toString(),
        scale: scale.toString(),
        encoder: encoder.toString(),
        ai: ai,
        type: 'offer',
    });
}
