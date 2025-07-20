'use strict';


// Get the modal
var modal1 = document.getElementById('id01');
var modal2 = document.getElementById('id02');
var modal3 = document.getElementById('id03');
var modal4 = document.getElementById('id04');
var modal5 = document.getElementById('id05');

// When the user clicks anywhere outside of the modal, close it
window.onclick = function(event) {
    if (event.target == modal1 || event.target == modal2 || event.target == modal3 || event.target == modal4 || event.target == modal5) {
        modal1.style.display = "none";
        modal2.style.display = "none";
        // modal3.style.display = "none";
        //modal4.style.display = "none";
        modal5.style.display = "none";
    }
}


var url = location.protocol + '//' + location.hostname  + ':' + 8080 ;
// function gettocken1(callback, cam='')
// {

//   var myHeaders = new Headers();
//   myHeaders.append("key", "admin@passsword");
//   myHeaders.append("exp", "360");
//   myHeaders.append("perm", "w");
//   myHeaders.append("Content-Type", "application/json");

//   var raw = "";

//   var requestOptions = {
//   method: 'OPTIONS',
//   headers: myHeaders,
//   body: raw
//   };


//   fetch(url, requestOptions)
//   .then(response => response.text())
//   .then((result) => { console.log(result); } )
//   .catch(error => console.log( error));


// }

const loginform = document.getElementById("loginform");
loginform.addEventListener('submit', function(event) {
    event.preventDefault();


    const inputs = loginform.elements;
    var  user = inputs[0].value;
    const pass = inputs[1].value;
    //const rem = inputs[3].checked;



    fetch(url + "/api/login", {
    method: 'POST',
    body: JSON.stringify({ 'uname':user,'psw':pass }),
    headers: {
      'Content-type': 'application/json',
    },
  })
    .then(response => 
    {
       if (!response.ok) {
          // make the promise be rejected if we didn't get a 2xx response
          return Promise.reject( response.status + ":"+ response.statusText );

       } else {
           return response.text();
       }
 
     }
     )
    .then(data => {
      // Handle the response from the server
      localStorage.setItem("token", data);

       console.log(data);
      loginform.parentElement.style.display = "none";
      document.getElementById("bt02").disabled=false;
      document.getElementById("bt03").disabled=false;
      document.getElementById("bt04").disabled=false;

    })
    .catch(error => {
      // Handle any errors that occur during form submission
      console.error(error);
      //alert('enter right userid and password.');

      document.getElementById("forget").innerHTML = error;

    });




});

const accountform = document.getElementById("accountform");
accountform.addEventListener('submit', function(event) {
    event.preventDefault();


    const inputs = accountform.elements;
    var  user = inputs[0].value;
    const pass = inputs[1].value;
    const pass1 = inputs[2].value;
    //const rem = inputs[3].checked;

    if( pass != pass1)
    {
        document.getElementById("frmerr").innerHTML = "password does not match";
        return ;
    }

    fetch(url + "/api/account", {
    method: 'POST',
    body: JSON.stringify({ 'uname':user,'psw':pass }),
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
           return response.text();
       }
 
     }
     )
    .then(data => {
      // Handle the response from the server
      localStorage.setItem("token", data);
      console.log(data);
      accountform.parentElement.style.display = "none";;
    })
    .catch(error => {
      // Handle any errors that occur during form submission
      console.error(error);
      //alert('enter right userid and password.');

      document.getElementById("frmerr").innerHTML = error;

    });




});
