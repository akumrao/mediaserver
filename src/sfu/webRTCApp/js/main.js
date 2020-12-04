

var records_per_page = 2;
var current_page = 1;

function RecordPerPage() {
  var x = document.getElementById("myInput").value;
  document.getElementById("demo").innerHTML = "You wrote: " + x;
  records_per_page =x;
  changePage(current_page);
}



var objJson = {}; // Can be obtained from another source, such as your objJson variable

// var obj0 = videoDisplay(0);
// var obj1 = videoDisplay(1);
// var obj2 = videoDisplay(2);
// var obj3 = videoDisplay(3);

// objJson['0']= obj0;
// objJson['1']= obj1;
// objJson['2']= obj2;
// objJson['3']= obj3;


var len = Object.keys(objJson).length

if (objJson.hasOwnProperty(2))
{
     console.log("found it");
}

//delete  objJson[1];


for (var key in objJson) {
    console.log("key " + key , "value = " + objJson[key]);
}

function videoDisplay( count)
{


        let el = document.createElement('video');

        el.setAttribute('playsinline', true);
        el.setAttribute('autoplay', true);
        el.setAttribute('id', `va-${count}`);
        //el.id = `video-${count}`;


        // set some attributes on our audio and video elements to make
        // mobile Safari happy. note that for audio to play you need to be


        var div = document.createElement('div');

        var para = document.createElement("P");
        para.innerHTML = "<span> <small> trackid:" +  count  + "<br>"+  "peerID:" +  count + "<br>" +   "</small> </span>";
        div.appendChild(para);

        

        //div.textContent = `streamid-${count}`
       // div.potato= store;
    
        div.appendChild(el);
        
        div.id = `consumer-div-${count}`;


        let pause = document.createElement('span'),
        checkbox = document.createElement('input'),
        label = document.createElement('label');

        pause.classList = 'nowrap';
        checkbox.type = 'checkbox';
        checkbox.id=count;
        checkbox.checked = false;
        checkbox.onchange = async () => {
        if (checkbox.checked) {
            await btn_subscribe_pause (checkbox.id);
        } else {
            await btn_subscribe_resume(checkbox.id);
        }

        }
        label.id = `consumer-stats-${count}`;
        

        label.innerHTML = "Pause Video";
        

        var divStore = document.createElement('div');

        

        var labelName = document.createElement("label");
        labelName.id = `conNameVid-${count}`;
        //labelName.innerHTML =  count.substring(0, 6);
        divStore.appendChild(labelName);


        pause.appendChild(checkbox);
        pause.appendChild(label);


        

    // pause.appendChild(checkbox);
        divStore.appendChild(pause);

             
        var td = document.createElement('td');
        td.id = `stream-${count}`;
        td.appendChild(div);

        td.appendChild(divStore);

       console.log(td);

       console.log("arvind %o", td);
       

       return td;
}

function prevPage()
{
    if (current_page > 1) {
        current_page--;
        changePage(current_page);
    }
}

function nextPage()
{
    if (current_page < numPages()) {
        current_page++;
        changePage(current_page);
    }
}
    
function changePage(page)
{
    var btn_next = document.getElementById("btn_next");
    var btn_prev = document.getElementById("btn_prev");
    var listing_table = document.getElementById("traddCtrl2");
    var page_span = document.getElementById("page");
 
    // Validate page
    if (page < 1) page = 1;
    if (page > numPages()) page = numPages();

    listing_table.innerHTML = "";

    // for (var i = (page-1) * records_per_page; i < (page * records_per_page); i++) {
    //     listing_table.innerHTML += "<td><div>" + objJson[i].innerHTML + "</div></td>";
    // }
    for (const [i, [key, value]] of Object.entries(Object.entries(objJson))) {
      console.log(`${i}: ${key} = ${value}`);

       if (  i >= (page-1) * records_per_page && i < (page * records_per_page)) {
            console.log("key=" + key + " i=" + i);
            listing_table.innerHTML += "<td><div>" + objJson[key].innerHTML + "</div></td>";
        }
    }

     
   

    page_span.innerHTML = page;

    if (page == 1) {
        btn_prev.style.visibility = "hidden";
    } else {
        btn_prev.style.visibility = "visible";
    }

    if (page == numPages()) {
        btn_next.style.visibility = "hidden";
    } else {
        btn_next.style.visibility = "visible";
    }
}

function numPages()
{
    return Math.ceil(Object.keys(objJson).length / records_per_page);
}

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

   $("#btn_connect").attr("disabled", true);

    console.log("connect");
          $("#div1").html("<h2>btn_connect!</h2>");
   socket.emit('create or join', roomId);

    var str = "Connected";
    var result = str.fontcolor("green");
    document.getElementById("divStatus").innerHTML = result;

 // const data = await socket.request('getRouterRtpCapabilities');

  });


  $("#btn_webcam").click(function(){
    $("#btn_webcam").attr("disabled", true);
    console.log("btn_webcam click");

   publish(true);
  });

  $("#btn_screen").click(function(){

    $("#btn_screen").attr("disabled", true);

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
