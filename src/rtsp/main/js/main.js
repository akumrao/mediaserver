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
            var codecs = "avc1.4D401F"; // https://wiki.whatwg.org/wiki/Video_type_parameters
            // if your stream has audio, remember to include it in these definitions.. otherwise your mse goes sour

            var codecs = "mp4a.40.2";

            // var codecs = "avc1.4D401F,mp4a.40.2";
            var codecPars = mimeType+';codecs="'+codecs+'"';
            
            //var stream_started = false; // is the source_buffer updateend callback active nor not
            
            // create media source instance
            var ms = null;// = new MediaSource();
            
            // queue for incoming media packets
            var queue = [];

            let bothAV;
            
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

            function getSubBox(arr, box_name, _ofs, ifs)
            {
                var i = _ofs;
                res = getBox(arr, i);
                main_length = res[0]; name = res[1]; // this boxes length and name
            
                 console.log("subb: " + name + " len " + main_length);

                i = i + 8 + ifs;
            
                var sub_box = [];

           
            
                while (i < main_length + _ofs) {
                   res = getBox(arr, i);
                   l = res[0];  name = res[1];
                
                   console.log("  inner: " + name + " l " + main_length);
                
                    if (box_name == name) {
                        var sb = arr.slice(i, i+l);
                        sub_box.push(sb);
                    }
                    i = i + l;
                }
                return sub_box;
            }



            function hasFirstSampleFlag(arr, _ofs){ // input Uint8Array
                // [moof [mfhd] [traf [tfhd] [tfdt] [trun]]]
                var traf = getSubBox(arr, "traf", _ofs, 0);
                if (!traf.length) { console.log("no traf in moof"); return false; }
                
                var trun = getSubBox(traf[0], "trun", 0, 0);
                if (!trun.length) { console.log("no trun in traf"); return false; }
                
                // ISO/IEC 14496-12:2012(E) .. pages 5 and 57
                // bytes: (size 4), (name 4), (version 1 + tr_flags 3)
                var flags = trun[0].slice(10,13); // console.log(flags);
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
                    

                    let code;
                    var sbt = getSubBox(memview, "trak", 0, 0);
                    if (!sbt.length)
                        return;
                    if(sbt.length == 2)
                        bothAV= true;
                    else
                        bothAV =false;

                    //"moov.trak.mdia.minf.stbl.stsd.mp4a.pinf\""
                    for (var i = 0; i < sbt.length; i++) 
                    {

                        sb = getSubBox(sbt[i], "mdia", 0, 0);
                        if (!sb.length)
                            return;
                        sb = getSubBox(sb[0], "minf", 0, 0);
                        if (!sb.length)
                            return;
                        sb = getSubBox(sb[0], "stbl", 0, 0);
                        if (!sb.length)
                            return;
                        var stsd = getSubBox(sb[0], "stsd", 0, 0);
                        if (!stsd.length)
                            return;

                        sb = getSubBox(stsd[0], "avc1", 0, 8);
                        if (!sb.length)
                        {

                            sb = getSubBox(stsd[0], "mp4a", 0, 8);
                            if (!sb.length)
                            return;

                            if(code)
                            code += ",mp4a.40.2";
                            else
                            code = "mp4a.40.2";

                        }
                        else
                        {
                           sb = getSubBox(sb[0], "avcC", 0, 78);
                           if (!sb.length)
                            return;

                           code="avc1." + dtox(sb[0][9]) + dtox(sb[0][10]) + dtox(sb[0][11]) ;

                        }

                    }



                    var cdars = mimeType+';codecs="'+code+'"';

                   

                    if (!MediaSource.isTypeSupported(cdars)) {
                        console.log("Mimetype " + cdars +
                                " not supported");
                       // ws.close();
                    } else
                    {
                        codecPars = cdars ;
                        console.log("Mimetype " + codecPars +
                            " supported");
                    }




                      reOpen();
                }
                else if ((name=="moof") && (pass==2)) {
                    if (hasFirstSampleFlag(memview, 0)) {
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

                data = arr;
                if (queue.length == 0 && source_buffer && !source_buffer.updating) {

                    try {

                        //console.log("direct, len " + data.byteLength);
                       // console.log(hexdump(data, 16));

                        //source_buffer.timestampOffset = ms.duration;
                        source_buffer.appendBuffer(data);
                        cc = cc + 1;

                    } catch (exc) {
                        console.log("exception: source_buffer.appendBuffer " + exc);
                        ws.close();
                        return;
                    };

                    //     console.log(hexdump(arr, 16));

                    return;
                }

                queue.push(data); // add to the end
                //console.log("queue push:" + queue.length + ", len: " + data.byteLength);
                  
               
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
                

                if (ms && ms.readyState === 'open')
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
                ms = new MediaSource();
              

                videoObj = document.getElementById('remoteVideo');


               
                if (videoObj)
                {
                    
                // get reference to video
                   //stream_live = document.getElementById('streamingVideo');
            
                // set mediasource as source of video
                    videoObj.src = window.URL.createObjectURL(ms);
                }

                //ms.addEventListener('sourceopen',opened,false);

                ms.addEventListener('sourceopen',ms_opened, false);
                ms.addEventListener('sourceclosed', ms_closed, false);
                ms.addEventListener('sourceended', ms_ended, false);
                 

                //if (shouldShowPlayOverlay)
                //{
                //  showPlayOverlay();
                  //resizePlayerStyle();
                  //ws.send(JSON.stringify({ type: 'gettings' }));
                //}
            }
            
            function ms_opened() { // MediaSource object is ready to go
                // https://developer.mozilla.org/en-US/docs/Web/API/MediaSource/duration
                 if(ms && ms.readyState == 'open')
                 {
                    ms.duration = buffering_sec;
                    source_buffer = ms.addSourceBuffer(codecPars);
                    
                    // https://developer.mozilla.org/en-US/docs/Web/API/source_buffer/mode
                    var myMode = source_buffer.mode;
                    source_buffer.mode = 'sequence';
                    // source_buffer.mode = 'segments';

                    console.log(ms.readyState);

                    source_buffer.addEventListener("updateend",loadPacket);
                    //source_buffer.addEventListener("update",loadPacket);
                    if (queue.length) {
                     console.log("ms_opened: loading queued pkt");
                     loadPacket();
                   }
                }
           

            }

            function ms_closed() {
                console.log("mediasource closed()");
               ws.close();
            }
  
            function ms_ended() {
                 console.log("mediasource ended()");
                 ws.close();
            }

           var _appendBuffer = function(buffer1, buffer2) {
                var tmp = new Uint8Array(buffer1.byteLength + buffer2.byteLength);
                tmp.set(new Uint8Array(buffer1), 0);
                tmp.set(new Uint8Array(buffer2), buffer1.byteLength);
               return tmp.buffer;
            };


            function hexdump(buffer, blockSize) {
                if (typeof buffer === 'string') {
                } else if (buffer instanceof ArrayBuffer && buffer.byteLength !== undefined) {
                    buffer = String.fromCharCode.apply(String,
                            [].slice.call(new Uint8Array(buffer)));
                } else if (Array.isArray(buffer)) {
                    buffer = String.fromCharCode.apply(String, buffer);
                } else if (buffer.constructor === Uint8Array) {
                    buffer = String.fromCharCode.apply(String,
                            [].slice.call(buffer));
                } else {
                    console.log("Error: buffer is unknown...");
                    return false;
                }

                blockSize = blockSize || 16;
                var lines = [];
                var hex = "0123456789ABCDEF";
                for (var b = 0; b < buffer.length; b += blockSize) {
                    var block = buffer.slice(b, Math.min(b + blockSize, buffer.length));
                    var addr = ("0000" + b.toString(16)).slice(-4);
                    var codes = block.split('').map(function(ch){
                        var code = ch.charCodeAt(0);
                        return " " + hex[(0xF0 & code) >> 4] + hex[0x0F & code];
                    }).join("");
                    codes += "   ".repeat(blockSize - block.length);
                    var chars = block.replace(/[\x00-\x1F\x20]/g, '.');
                    chars += " ".repeat(blockSize - block.length);
                    lines.push(addr + " " + codes + "  " + chars);
                }
                return lines.join("\n");
            }

            function dtox(d, padding) {
                var hex = Number(d).toString(16);

                padding = typeof (padding) === "undefined" ||
                        padding === null ? padding = 2 : padding;

                while (hex.length < padding)
                    hex = "0" + hex;

                return hex;
            }



            function loadPacket() 
            {

                if (source_buffer && !source_buffer.updating) { // really, really ready
                    if (queue.length>0) 
                    {

                        inp = queue.shift();

                        //console.log("loadPacket " + ms.readyState + ", dur " + videoObj.duration + ", ms dur " + ms.duration + ", len " + inp.byteLength);

                        if (verbose) { console.log("queue pop:", queue.length); }
                        //console.log(hexdump(inp, 16));
                        if (verbose)
                        {
                            var memview = new Uint8Array(inp);
                            res = getBox(memview, 0);
                            console.log(res[1]);
                                 //console.log(" ==> writing buffer with", memview[0], memview[1], memview[2], memview[3]);
                        }
                        
                        try
                        {
                    
                            //inp =  queue[0];
  
                            source_buffer.appendBuffer(inp);
                            //queue.shift();
                         } catch (e) 
                         {
                            console.log("sourceBuffer.appendBuffer = " + e.toString())
                             ws.close();
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

