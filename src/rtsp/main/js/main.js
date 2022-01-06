///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

            // *** USER PARAMETERS ***
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

            // var codecs = "mp4a.40.2";

            var codecs = "avc1.4D401F,mp4a.40.2";
            var codecPars = mimeType+';codecs="'+codecs+'"'; //Rate in Hz, 0xBB80 for 48 KHz, and 0XAC44 for 44.1 KHz
            
            //var stream_started = false; // is the source_buffer updateend callback active nor not
            
            // create media source instance
            var ms = new MediaSource();
            
            // queue for incoming media packets
            var queue = [];
            
            //var stream_live; // the HTMLMediaElement (i.e. <video> element)
            var ws; // websocket
            var seeked = false; // have have seeked manually once ..
            var cc = 0;
            
            var source_buffer = null; // source_buffer instance
            
            var pass = 0;

            let videoObj;
            
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
                    
                    
                    // var arv="";
                    // for( var i = 0 ; i < arr.byteLength; ++i)
                    // {
                    //      if (memview[i] == 0x67) 
                    //      {
                    //             var x = 0;
                    //      }

                    //      if (memview[i] ==  0xBB && memview[i+1] == 0x80 ) 
                    //      {
                    //             var xx = 0;
                    //      }


                    //      if (memview[i] ==  0xAC && memview[i+1] == 0x44 ) 
                    //      {
                    //             var xx = 0;
                    //      }


                    //     arv += memview[i].toString(16);
                    // }

                    // console.log("got moov" + arv);

                    if (memview[491] == 0x67) 
                    { 
                       var codecPars1 =
                      'video/mp4; codecs="avc1.' +
                      memview[492].toString(16) +
                      memview[493].toString(16) +
                      memview[494].toString(16) +
                      '"'
                      console.log("Video actual codec:'" + codecPars1 + "'")
                      codecs = "avc1.4D401F";
                    }


                    if ((memview[731] == 0xBB  &&  memview[732] == 0x80)  ||  (memview[731] == 0xAC  &&  memview[732] == 0x44))
                    { 
                        codecs += ",mp4a.40.2";
                        
                    }
                     
                    codecPars = mimeType+';codecs="'+codecs+'"';
                    reOpen();
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
                    console.log("got frame " + name );
                    return;
                }


/*
                if ((name=="moof") ) {
                    if (hasFirstSampleFlag(memview)) {
                       
                        console.log("got that special moof");

                         if (queue.length  >  35 ) 
                          {
                         
                             queue = [];

                           }
                    }
                    
                }
*/

//                console.log("show frame " + name );

                
                // keep the latency to minimum
                if(videoObj && source_buffer)
                {
                    let latest = videoObj.duration;
                    if ((videoObj.duration >= buffering_sec) && 
                        ((latest - videoObj.currentTime) > buffering_sec_seek)) {
                        console.log("seek from ", videoObj.currentTime, " to ", latest);
                        df = (videoObj.duration - videoObj.currentTime); // this much away from the last available frame
                        if ((df > buffering_sec_seek)) {
                            seek_to = videoObj.duration - buffering_sec_seek_distance;
                            videoObj.currentTime = seek_to;
                            }
                    }
                }

                queue.push(arr); // add to the end
                if (verbose) { console.log("queue push:", queue.length); }

                // data = arr;
                
                // if (!stream_started) {
                //     if (verbose) {console.log("Streaming started: ", memview[0], memview[1], memview[2], memview[3], memview[4]);}
                //     source_buffer.appendBuffer(data);
                //     stream_started = true;
                //     cc = cc + 1;
                //     return;
                // }
                if(source_buffer)
                loadPacket();

                      
               
            }
            
            function reSet()
            {
               // console.log("reSet");
                pass = -1;
                stream_started = false; 
                queue = [];
                seeked = false; 
                 cc = 0;
                

                //videoObj.stop();
                //  if (videoObj)
                // {
                //     videoObj.pause();
                //   videoObj.removeAttribute('src'); 
                //     videoObj.remove();
                //     videoObj = null;
                // }
                

                if (ms.readyState === 'open')
                {
                    try {
                        ms.removeSourceBuffer(source_buffer);
                        ms.endOfStream();

                    } catch (error) 
                    {
                       console.log( error.message);
                    }
                }  
                
                ms =null;
                source_buffer = null;
               
                ms = new MediaSource();          
                pass = 0; 
            }

            function reOpen()
            {
                console.log("Open");
              

                videoObj = document.getElementById('remoteVideo');


               
                if (videoObj)
                {
                    
                // get reference to video
                   //stream_live = document.getElementById('streamingVideo');
            
                // set mediasource as source of video
                    videoObj.src = window.URL.createObjectURL(ms);
                }

                //ms.addEventListener('sourceopen',opened,false);

                 ms.addEventListener('sourceopen',opened);

                //if (shouldShowPlayOverlay)
                //{
                //  showPlayOverlay();
                  //resizePlayerStyle();
                  //ws.send(JSON.stringify({ type: 'gettings' }));
                //}
            }
            
            function opened() { // MediaSource object is ready to go
                // https://developer.mozilla.org/en-US/docs/Web/API/MediaSource/duration
                 if(ms && ms.readyState == 'open')
                 {
                    ms.duration = buffering_sec;
                    source_buffer = ms.addSourceBuffer(codecPars);
                    
                    // https://developer.mozilla.org/en-US/docs/Web/API/source_buffer/mode
                    var myMode = source_buffer.mode;
                    source_buffer.mode = 'sequence';
                    // source_buffer.mode = 'segments';

                    //source_buffer.addEventListener("updateend",loadPacket);
                    source_buffer.addEventListener("update",loadPacket);
                }
           

            }

             function loadPacket() 
            {

                if (!source_buffer.updating) { // really, really ready
                    if (queue.length>0) 
                    {
                         try
                         {
                    
                            inp =  queue[0];
                            if (verbose) { console.log("queue pop:", queue.length); }
                    
                            if (verbose)
                            {
                               var memview = new Uint8Array(inp);
                                  res = getBox(memview, 0);
                                     console.log(res[1]);
                                 //console.log(" ==> writing buffer with", memview[0], memview[1], memview[2], memview[3]);
                             }
                        
                            source_buffer.appendBuffer(inp);
                            queue.shift();
                         } catch (e) 
                         {
                            console.log("sourceBuffer.appendBuffer = " + e.toString())
                            reSet();
                         }

                        cc = cc + 1;
                        }
                    else { // the queue runs empty, so the next packet is fed directly
                       // stream_started = false;
                    }
                }
                else { // so it was not?
                }

            }

            function startup() 
            {
                window.WebSocket = window.WebSocket || window.MozWebSocket;

                if (!window.WebSocket) {
                  alert('Your browser doesn\'t support WebSocket');
                  return;
                }




                 


                let number = getUrlVars()["cam"];
                   if ( !number ) {
                     number =0;
                    }





                var pth = window.location.href.replace('http://', 'ws://').replace('https://', 'wss://') ;

                ws = new WebSocket(pth);


                //ws = new WebSocket("ws://localhost:1111/ws/");
                ws.binaryType = "arraybuffer";
                ws.onmessage = function (event) {

                   /* if (document.hidden) {
                 
                        return;
                    }*/

                    if(event.data instanceof ArrayBuffer) {
                        // binary frame
                       // const view = new DataView(event.data);
                       // console.log(view.getInt32(0));
                       if(pass > -1 )
                       putPacket(event.data);
                     //source_buffer.appendBuffer(event.data);

                    } 
                    else 
                    {
                        // text frame
                        console.log(event.data);
                        log('Client sending message: ', event.data);
                        // if(event.data == "reset" )
                        // {
                        //     document.getElementById("parStats").innerHTML = "";
                        //     reSet();
                        // }
                        // else
                        // {
                        //     document.getElementById("settings-button").disabled = false;
                        //     document.getElementById("parStats").innerHTML = event.data ;
                        // }
                    }

                    
                };
                
                ws.onopen = function() 
                {
                    console.log('WebSocket Client Connected');
                    ws.send(number.toString());

                       // if (shouldShowPlayOverlay)
                       // {
                       //    showPlayOverlay();
                       //    resizePlayerStyle();
                       //    //ws.send(JSON.stringify({ type: 'gettings' }));
                       //  }

                };

                ws.onclose = function () {
                    console.log("DataChannel closed");
                };

                ws.onerror = function (e) {
                    console.log("DataChannel error: " + e.message);
                    console.log(e);
                };

          
            }







//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


function getUrlVars() {
    var vars = {};
    var parts = window.location.href.replace(/[?&]+([^=&]+)=([^&]*)/gi, function(m,key,value) {
        vars[key] = value;
    });
    return vars;
}



function load() {

    startup();
}










//log('Client sending message: ', message);









// Set up audio and video regardless of what devices are present.
/*var sdpConstraints = {
  offerToReceiveAudio: true,
  offerToReceiveVideo: true
};*/

/////////////////////////////////////////////


// Could prompt for room name:
// room = prompt('Enter room name:');

