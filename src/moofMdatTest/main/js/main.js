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
                        //console.log("got that special moof");

                         LogMessage ({
                          type: 'candidate',
                          desc: 'streaming'
                        });

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
                        //console.log(event.data);
                        //log('Client sending message: ', event.data);

                         LogMessage ({
                          type: 'error',
                          desc: event.data
                        });

                        ws.close();
                        


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

                       LogMessage ({
                          type: 'candidate',
                          desc: 'connected'
                        });

                };

                ws.onclose = function () {
                   // console.log("DataChannel closed");

                        LogMessage ({
                          type: 'bye',
                          desc: 'Connection close'
                        });

                };

                ws.onerror = function (e) {
                    //console.log("DataChannel error: " + e.message);
                    //console.log(e);

                         LogMessage ({
                          type: 'error',
                          desc: e.message
                        });

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




function LogMessage(message) {
  console.log('message: %s, %o', message.type,  message.desc );
  log('client Message', message.type,  message.desc );

    if (typeof message.type !== 'undefined' && message.type == 'error') 
    {
        document.getElementById("hlsStreamSection").innerHTML = '<div id="errorId" class="liveview_col text-center">We are having trouble connecting to this camera. Please try again. If issue persists, please report to <a target="_blank" href="mailto: customer.support@pro-vigil.com">customer.support@pro-vigil.com</a></div>'; 
        document.getElementById("videoLoader").innerHTML =  '';  
    }
    if (typeof message.type !== 'undefined' && message.type == 'bye') 
    {
        document.getElementById("hlsStreamSection").innerHTML = '<div id="errorId" class="liveview_col text-center">We are having trouble connecting to this camera. Please try again. If issue persists, please report to <a target="_blank" href="mailto: customer.support@pro-vigil.com">customer.support@pro-vigil.com</a></div>';
        document.getElementById("videoLoader").innerHTML =  '';    
    }
}

actionButtons();
function actionButtons() {
	/* predefine zoom and rotate */
	var zoom = 1,
		rotate = 0;
	/* Grab the necessary DOM elements */
	var hlsStreamSection = document.getElementById('hlsStreamSection'),
		v = document.getElementsByTagName('video')[0],
		controls = document.getElementById('controls');
	/* Array of possible browser specific settings for transformation */
	var properties = ['transform', 'WebkitTransform', 'MozTransform', 'msTransform', 'OTransform'],
		prop = properties[0];
	/* Iterators and stuff */
	var i, j, t;
	/* Find out which CSS transform the browser supports */
	/*   for(i=0,j=properties.length;i<j;i++){
	    if (properties[i] in v.style) {
	      prop = properties[i];
	      break;
	    }
	  } */
	/* Position video */
	v.style.left = 0;
	v.style.top = 0;
	/* If there is a controls element, add the player buttons */
	if(controls) {
		controls.innerHTML = '<div id="change">' + '<button title="Reset" class="reset"><svg class="reset" xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink"  width="45" height="45" viewBox="0 0 512 512" xml:space="preserve"><g class="reset" transform="matrix(1.48 0 0 1.48 256 256)"><path class="reset" style="stroke: rgb(0,0,0); stroke-width: 0; stroke-dasharray: none; stroke-linecap: butt; stroke-dashoffset: 0; stroke-linejoin: miter; stroke-miterlimit: 4; fill: rgb(77,77,79); fill-rule: nonzero; opacity: 1;" vector-effect="non-scaling-stroke"  transform=" translate(-256, -244.27)" d="M 256 121.07 L 122.88 234.35 L 170.70999999999998 234.35 L 170.70999999999998 347.81 C 170.7980015432894 358.71166102152233 179.6780348828916 367.4933749180086 190.57999999999998 367.46 L 230.27999999999997 367.46 L 230.27999999999997 298.08 L 283.18999999999994 298.08 L 283.18999999999994 367.46 L 321.42999999999995 367.46 C 332.3297231322432 367.4878975083077 341.2065463470775 358.7093581941913 341.29999999999995 347.81 L 341.29999999999995 234.35 L 389.12999999999994 234.35 Z" stroke-linecap="round" /></g></svg></button>'+'<button title="Zoom In" class="zoomin"><svg class="zoomin" viewBox="0 0 512 512"><path class="zoomin" d="M505.75,475.58,378.42,348.25a212.3,212.3,0,0,0,48.25-134.92C426.67,95.7,331,0,213.33,0S0,95.7,0,213.33,95.7,426.67,213.33,426.67a212.3,212.3,0,0,0,134.92-48.25L475.58,505.75a21.33,21.33,0,1,0,30.17-30.17ZM277.33,234.67H234.67v42.67a21.33,21.33,0,1,1-42.67,0V234.67H149.33a21.33,21.33,0,0,1,0-42.67H192V149.33a21.33,21.33,0,0,1,42.67,0V192h42.67a21.33,21.33,0,1,1,0,42.67Z" fill="#5b5b5f"/></svg></button>' + '<button title="Zoom Out" class="zoomout"><svg class="zoomout" viewBox="0 0 512 512"><path class="zoomout" d="M505.75,475.58,378.42,348.25a212.3,212.3,0,0,0,48.25-134.92C426.67,95.7,331,0,213.33,0S0,95.7,0,213.33,95.7,426.67,213.33,426.67a212.3,212.3,0,0,0,134.92-48.25L475.58,505.75a21.33,21.33,0,1,0,30.17-30.17ZM277.33,234.67h-128a21.33,21.33,0,0,1,0-42.67h128a21.33,21.33,0,1,1,0,42.67Z" fill="#5b5b5f"/></svg></button>' + '<button title="Move Left" class="right"><svg class="right" viewBox="0 0 512 512"><polygon class="right" points="91.13 256 255.42 382.35 255.42 302.45 405.63 302.45 405.63 209.56 255.44 209.56 255.44 129.65 91.13 256" fill="#5b5b5f"/></svg></button>' + '<button title="Move Right" class="left"><svg class="left" viewBox="0 0 512 512"><polygon class="left" points="420.88 256 256.57 129.66 256.57 209.57 106.37 209.57 106.37 302.45 256.57 302.45 256.57 382.35 420.88 256" fill="#5b5b5f"/></svg></button>' + '<button title="Move Down" class="up"><svg class="up" viewBox="0 0 512 512"><polygon class="up" points="256 420.89 382.35 256.58 302.44 256.58 302.44 106.38 209.55 106.38 209.55 256.57 129.65 256.57 256 420.89" fill="#5b5b5f"/></svg></button>' + '<button title="Move Up" class="down"><svg class="down" viewBox="0 0 512 512"><polygon class="down" points="256.02 91.13 129.66 255.43 209.57 255.43 209.57 405.62 302.45 405.62 302.45 255.43 382.36 255.43 256.02 91.13" fill="#5b5b5f"/></svg></button>' + '</div>';
	}
	/* If a button was clicked (uses event delegation)...*/
	controls.addEventListener('click', function(e) {
		t = e.target;
		if(t.nodeName.toLowerCase() === 'button' || t.nodeName.toLowerCase() == 'svg' || t.nodeName.toLowerCase() == 'polygon' || t.nodeName.toLowerCase() == 'path') {
			
			var classValue = typeof t.className.baseVal === "undefined" ? t.className : t.className.baseVal;
			/* Check the class name of the button and act accordingly */
			switch (classValue) {
				/* Increase zoom and set the transformation */
				case 'zoomin':
					if(zoom < 6) {
						zoom = zoom + 0.5;
						v.style.transform = 'scale(' + zoom + ') rotate(' + rotate + 'deg)';
					}
					break;
					/* Decrease zoom and set the transformation */
				case 'zoomout':
					if(zoom > 1) {
						zoom = zoom - 0.5;
						v.style.transform = 'scale(' + zoom + ') rotate(' + rotate + 'deg)';
					}
					break;
				case 'left':
					if(zoom > 1) {
						v.style.left = (parseInt(v.style.left, 10) - 25) + 'px';
					}
					break;
				case 'right':
					if(zoom > 1) {
						v.style.left = (parseInt(v.style.left, 10) + 25) + 'px';
					}
					break;
				case 'up':
					if(zoom > 1) {
						v.style.top = (parseInt(v.style.top, 10) - 25) + 'px';
					}
					break;
				case 'down':
					if(zoom > 1) {
						v.style.top = (parseInt(v.style.top, 10) + 25) + 'px';
					}
					break;
					/* Reset all to default */
				case 'reset':
					zoom = 1;
					rotate = 0;
					v.style.top = 0 + 'px';
					v.style.left = 0 + 'px';
					v.style.transform = 'rotate(' + rotate + 'deg) scale(' + zoom + ')';
					break;
			}
			e.preventDefault();
		}
	}, false);
	document.addEventListener("keydown", function(event) {
		if ((event.keyCode == 187 || event.keyCode == 61) && zoom < 6) {
			zoom = zoom + 0.5;
			v.style.transform = 'scale(' + zoom + ') rotate(' + rotate + 'deg)';
		}
		if ((event.keyCode == 189 || event.keyCode == 173) && zoom > 1) {
			zoom = zoom - 0.5;
			v.style.transform = 'scale(' + zoom + ') rotate(' + rotate + 'deg)';
		}
		if (event.keyCode == 39 && zoom > 1) {
			v.style.left = (parseInt(v.style.left, 10) - 25) + 'px';
		}
		if (event.keyCode == 37 && zoom > 1) {
			v.style.left = (parseInt(v.style.left, 10) + 25) + 'px';
		}
		if (event.keyCode == 40 && zoom > 1) {
			v.style.top = (parseInt(v.style.top, 10) - 25) + 'px';
		}
		if (event.keyCode == 38 && zoom > 1) {
			v.style.top = (parseInt(v.style.top, 10) + 25) + 'px';
		}
		if (event.keyCode == 27) {
			zoom = 1;
			rotate = 0;
			v.style.top = 0 + 'px';
			v.style.left = 0 + 'px';
			v.style.transform = 'rotate(' + rotate + 'deg) scale(' + zoom + ')';
		}
	});
};



