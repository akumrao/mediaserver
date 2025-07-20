

function tree_expand_recording(id, node)
{
    var toggler = document.getElementById(id);

    const ulElem = toggler.parentElement.children;
   
    if(ulElem.length > 1)
    {
        toggler.parentElement.removeChild(ulElem[1] );
    }


    createSublist( toggler.parentElement, node);
   // tree_click_add_event();

    toggler.parentElement.querySelector(".nested").classList.toggle("active");
    toggler.classList.toggle("caret-down");

    createTable_recording(1,1); 

   // test1();
}   
  



function createTable_recording(row, column)
{
    var output = '<div>'+'<table width=100% border="1" style="border-color: #ccc;" cellspacing="0" cellpadding="5" class="table">'

    var cellSizd = 100/(row*column);

    for(var i = 1; i <= row; i++){
        output += '<tr>'
        for(var j = 1;j <= column; j++){

            var cs = 'recod'+i+j;

            output += '<td id=' + cs + ' width=' + cellSizd + '% bgcolor="red" style="aspect-ratio: 16/9;" >'+'<div class="drag" style="aspect-ratio: 16/9;" >Drag and Drop Camera</div>'+'</td>'
        }
        output += '</tr>'
    }
    output += '</div>'+'</table>'
    document.getElementById('reccontainer').innerHTML=output;


    let items = document.querySelectorAll('.drag');
    items.forEach(item => dragEvenListner(item));


} 
  


var url = location.protocol + '//' + location.hostname  + ':' + 8080 ;

function getRecordingCam()
{

   var tabid04 =  document.getElementById('id04');

   if(tabid04)
   tabid04.style.display='block';



   fetch(url + "/api/recordcam", {
    method: 'GET',
    headers: {
      'Content-type': 'application/json'
    },
    
    })
    .then(response => 
    {
       if (!response.ok) {
          // make the promise be rejected if we didn't get a 2xx response
          return Promise.reject( response.status + ":"+ response.statusText );

       } else {
           return response.json();
       }
 
     }
     )
    .then(data => {

          if(!data)
          { 
            document.getElementById("msgRecord").innerHTML = "Please press Setting Menu option and enable recording";

            return;
          }

          var  selectBox = document.getElementById("cameraSel");

          var length = selectBox.options.length;
          for (i = length-1; i >= 0; i--) {
            selectBox.options[i] = null;
          }

          for (let i = 0; i < data.length; i++) {

            myOption = document.createElement("option");
            myOption.text = data[i];
            myOption.value = data[i];
            selectBox.appendChild(myOption);
            

          }

        


        if( selectBox.options.length)
        {
          let slected = selectBox.options[0].value;  
          getRecordingTree(slected);
        }

 


    })
    .catch(error => {
      // Handle any errors that occur during form submission
      console.error(error);
      //alert('enter right userid and password.');

      document.getElementById("msgRecord").innerHTML = error;

    });
 
}



function getRecordingTree( camID)
{


   fetch(url + "/api/recordtree", {
    method: 'GET',
    headers: {
      cam: camID,
      'Content-type': 'application/json'
    },
    
    })
    .then(response => 
    {
       if (!response.ok) {
          // make the promise be rejected if we didn't get a 2xx response
          return Promise.reject( response.status + ":"+ response.statusText );

       } else {
           return response.json();
       }
 
     }
     )
    .then(data => {
      // Handle the response from the server
      //printCamList(lstCam=data)
        tree_expand_recording("RecRoot", data);


    })
    .catch(error => {
      // Handle any errors that occur during form submission
      console.error(error);
      //alert('enter right userid and password.');

      document.getElementById("msgRecord").innerHTML = error;

    });
 
}






function test1()
{

  playRecording("1/2023-08-30/14/14-42-31_783/");

}



//getRecordingCam();



function doSomething( el)
{
    getRecordingTree(el.value);
}


function playRecording(recordPath )
{

 // console.log(recid);


  var parentRec  =  document.getElementById("recod11");
  var parentChild = parentRec.children[0];

  parentRec.removeChild(parentChild);


  let el = document.createElement("video");

  el.setAttribute('playsinline', true);
  el.setAttribute('autoplay', true);
  el.muted = true;
  //el.id = "remoteVideo";
  el.id = `remoteVideo`;


  el.controls = false;

  parentRec.appendChild(el);


  startup(recordPath);

}


function stop_recording()
{
   // ms_closed();

    reSet();

    var parentRec  =  document.getElementById("recod11");
    var parentChild = parentRec.children[0];

    parentRec.removeChild(parentChild);


    var divDrag = document.createElement('div');
    divDrag.innerHTML="Drag and Drop Camera";
    divDrag.className = "drag";
    divDrag.style.aspectRatio="16/9";
    parentRec.appendChild(divDrag);
    dragEvenListner(divDrag);

   // test1();

}


function close_recording()
{
   stop_recording();
   
   var id04 = document.getElementById('id04');
   if(id04)
   id04.style.display='none';


}
