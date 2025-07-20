

function tree_expand_cam(id, node)
{
    var toggler = document.getElementById(id);

    if(toggler)
    {


      const ulElem = toggler.parentElement.children;
   
      if(ulElem.length > 1)
      {
          toggler.parentElement.removeChild(ulElem[1] );
      }


      createSublist( toggler.parentElement, node);
     // tree_click_add_event();

      toggler.parentElement.querySelector(".nested").classList.toggle("active");
      toggler.classList.toggle("caret-down");


      createTable_cam(2,2); 
    }

}   
  



function createTable_cam(row, column)
{
    var output = '<div>'+'<table width=100% border="1" style="border-color: #ccc;" cellspacing="0" cellpadding="5" class="table">'

    var cellSizd = 100/(row*column);

    for(var i = 1; i <= row; i++){
        output += '<tr>'
        for(var j = 1;j <= column; j++){

            var cs = 'liveS'+i+j;

            output += '<td id=' + cs + ' width=' + cellSizd + '% bgcolor="red" style="aspect-ratio: 16/9;" >'+'<div class="drag" style="aspect-ratio: 16/9;" >Drag and Drop Camera</div>'+'</td>'
        }
        output += '</tr>'
    }
    output += '</div>'+'</table>'
    document.getElementById('container').innerHTML=output;


    let items = document.querySelectorAll('.drag');
    items.forEach(item => dragEvenListner(item));


}   
  



var url = location.protocol + '//' + location.hostname  + ':' + 8080 ;

function getCamTree( )
{


   fetch(url + "/api/camtree", {
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
      // Handle the response from the server
      //printCamList(lstCam=data)
        tree_expand_cam("rootLiveCam", data);

    })
    .catch(error => {
      // Handle any errors that occur during form submission
      console.error(error);
      //alert('enter right userid and password.');

      //document.getElementById("msgCam").innerHTML = error;

    });
 

}

getCamTree();

//tree_expand("root", test.rtsp );

//createTable(2,2); 
var submitBut = document.getElementById('submit');

if(submitBut)
submitBut.onclick=function(){
      var row = document.getElementById('row').value;
      var column = document.getElementById('column').value;
      if(row == '' || row == 'null' ){
          alert('Enter row value');
      }
      if(column == '' || column == 'null' ){
          alert('Enter column value');
      }
     
      createTable(row,column); 
};


        //Enter Only Numeric Value
function isNumberKey(my_event){
    var charCode = (my_event.which) ? my_event.which : event.keyCode
    if (charCode > 31 && (charCode < 48 || charCode > 57))
        {
            alert("Enter Only Numbers");
            return false;
        }
    return true;
}



function testCam1()
{

   var divAdd  =     document.getElementById("liveS11").children[0];

   addCamera(2, divAdd);
}
