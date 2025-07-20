///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

            // *** USER PARAMETERS ***
    

            let bothAV;
            
            //var stream_live; // the HTMLMediaElement (i.e. <video> element)
            var ws; // websocket
            var seeked = false; // have have seeked manually once ..
            var cc = 0;
            
            var source_buffer = null; // source_buffer instance
            
            var pass = 0;

            let videoObj;
            
            let saveVideo;
            var ClipPlay = false;

      
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
               
                log('buffered mp4: ', memview.byteLength);


                if( ClipPlay)
                {
                    if(saveVideo)
                    {
                        saveVideo = concatTypedArrays(saveVideo, memview );
                    }
                    else
                    {
                        saveVideo = memview;
                    }
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

                var md = number.split("_");

                if(md[0] ="MD")
                    ClipPlay = true;



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
                        log('Client message: ', event.data);
                       
                        
                        if(event.data == "close" )
                        {
                            ws.close();
                           // reSet();
                        }
                        
                    }

                    
                };
                
                ws.onopen = function() 
                {
                    console.log('WebSocket Client Connected');
                    ws.send(number.toString());

                    log("press Save option to save to file" );

                };

                ws.onclose = function () {
                    console.log("closed");
                };

                ws.onerror = function (e) {
                    console.log("error: " + e.message);
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



function downloadMp4() {

    console.log("download");

   if(ClipPlay)
   {
     var blob = new Blob([saveVideo]);
     saveAs(blob, "dump.mp4");
   }  

}


function concatTypedArrays(buffer1, buffer2) 
{
  var tmp = new Uint8Array(buffer1.byteLength + buffer2.byteLength);
  tmp.set(buffer1, 0);
  tmp.set(buffer2, buffer1.byteLength);
  return tmp;
}

