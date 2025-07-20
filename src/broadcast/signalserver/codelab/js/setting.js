'use strict';

const newidDef = 345246;
let  newid = newidDef;

var url = location.protocol + '//' + location.hostname  + ':' + 8080 ;

let lstCam;

function setting()
{


  var tabid03 =  document.getElementById('id03');

  if(tabid03)
  tabid03.style.display='block';

  var token = localStorage.getItem("token");

  getCamList( token);
  
}

function getCamList( token)
{


   fetch(url + "/api/camlist", {
    method: 'GET',
    headers: {
      'Content-type': 'application/json',
       token : localStorage.getItem("token")
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
      printCamList(lstCam=data)

    })
    .catch(error => {
      // Handle any errors that occur during form submission
      console.error(error);
      //alert('enter right userid and password.');

      document.getElementById("msgCam").innerHTML = error;

    });

}


function deleteCam( token, cam)
{

    var myHeaders = new Headers();
    myHeaders.append("token", token);
    myHeaders.append("Content-Type", "application/json");

    var tmp = {};
    tmp[cam] = lstCam[cam];

    var raw  = JSON.stringify(tmp);

     var requestOptions = {
    method: 'DELETE',
    headers: myHeaders,
    body: raw
    };


    fetch(url, requestOptions)
     .then(response => response.text())
     .then((result) => {    delete lstCam[cam]; printCamList(lstCam); ++newid; })
     .catch(error => {
      // Handle any errors that occur during form submission
      console.error(error);
      //alert('enter right userid and password.');

      document.getElementById("msgCam").innerHTML = error;

    });
}

function saveCam( token, cam)
{

    var myHeaders = new Headers();
    myHeaders.append("token", token);
    //myHeaders.append("perm", "w");
    myHeaders.append("Content-Type", "application/json");

    var tmp = {};
    tmp[cam] = lstCam[cam];

    var raw  = JSON.stringify(tmp);

     var requestOptions = {
    method: 'PUT',
    headers: myHeaders,
    body: raw
    };


    fetch(url + "/api/put", requestOptions)
     .then(response => response.text())
     .then((result) => {   printCamList(lstCam) })
     .catch(error => {
      // Handle any errors that occur during form submission
      console.error(error);
      //alert('enter right userid and password.');

      document.getElementById("msgCam").innerHTML = error;

    });

}

 function printCamList( lst)
 {

  var type = document.getElementById("streamType").value;
  
  let text = "<table id='tabSetting' border='1' width=100%>"
  
  for (let x in lst) 
  {

    let  el =  lst[x];
    if( type == "Audio")
    {

       if( lst[x].audio)
            el = lst[x].audio
        else
          continue;
    }
    else if( type == "Analytics")
      continue;

     var ch ;

     if(el.recording)
     {
        ch = (el.recording =="on") ?'checked':'';
     }
     else
     {
        lst[x].recording = "off";
     }

     text += "<tr>";
         
     text += "<td>" + "<button type='button' id=del" + x  +  " onClick='cb_delete(this)'>Delete </button> </td>";

     text +=  "<td>" +  x + "</td>" ;


     text +=  "<td width='80'> <input type='text' name='txt' size='80' oninput='cb_txtchange(this)' value =" + el.rtsp + " id=txt" + x  + " > </td>" ;

     text +=  "<td>" +  (((el.state != undefined) && el.state.length) >  20 ? el.state.slice(0,19) : el.state)   + "</td>";

     text +=  "<td>" + "Record: <input "+ ch + " type='checkbox' id=rec" + x  +  " onClick='cb_record(this)' ></td>";

     text +=  "<td>" + "<button type='button' id=sav" + x  +  " onClick='cb_save(this)'>Save </button> </td>"+
       
     "</tr>";
  }
  
  text += "</table>"    

  document.getElementById("camlist").innerHTML = text;

 }




  function cb_record(elem)
  {

    var id =  elem.id.slice(3);

    if(elem.checked)
       lstCam[id].recording = "on";
    else
       lstCam[id].recording = "off";
  }


  function cb_delete(elem)
  {
    
     var id =  elem.id.slice(3);

    deleteCam(localStorage.getItem("token"), id);
    
  }


 function cb_save(elem)
{
   
    var id =  elem.id.slice(3);
    saveCam(localStorage.getItem("token"), id);
  
}



function cb_add()
{
   
  

 //  lstCam[size].rtsp = elem.value;


  var table = document.getElementById("tabSetting");

  // Create an empty <tr> element and add it to the 1st position of the table:
  var row = table.insertRow(0);


  // Insert new cells (<td> elements) at the 1st and 2nd position of the "new" <tr> element:
  var cell1 = row.insertCell(0);
  var cell2 = row.insertCell(1);
  var cell3 = row.insertCell(2);
  var cell4 = row.insertCell(3);
  var cell5 = row.insertCell(4);
  var cell6 = row.insertCell(5);
 
  // Add some text to the new cells:
  cell1.innerHTML = "XXXXX";
  cell2.innerHTML = "<input type='text' name='txt' size='20' " + " id=newiid" + newid  + " >";
  cell3.innerHTML = "<input type='text' name='txt' size='80'  value =rtsp://localhost/test.264" + " id=newtxt" + newid  + " >";
  cell4.innerHTML = "XXXXXX";
  cell5.innerHTML = "XXXXXX";
  cell6.innerHTML = "<button type='button' id=newsav" + newid  +  " onClick='cb_newsave(this)'>Save </button>";

  newid = newid +1;
  
}



function cb_txtchange(elem)
{
   
   var id =  elem.id.slice(3);
   lstCam[id].rtsp = elem.value;
}




function cb_newsave(elem)
{
   
  var id =  elem.id.slice(6);


  
  var newiid =  document.getElementById("newiid" + id );

  if( !newiid.value.length || lstCam[newiid.value] )
  {
     alert("Please enter unique camera Id. Id exist: " + newiid.value );
     return;
  } 

  var newtxt =  document.getElementById("newtxt" + id );
  
  if( newiid &&  newtxt )
  {

    lstCam[newiid.value]={};
    lstCam[newiid.value].rtsp  = newtxt.value;
    lstCam[newiid.value].recording  = "off";
    saveCam(localStorage.getItem("token"), newiid.value);
    printCamList( lstCam );

  }


}



function onStreamTypeChange()
{
    printCamList( lstCam )

}


function close_setting()
{
  document.getElementById('id03').style.display='none';
  
  if(   newid != newidDef )
  {
     //getCamTree();  this will cause issue. Instead add and delete individual element to camera tree
     newid = newidDef;
  }

}
