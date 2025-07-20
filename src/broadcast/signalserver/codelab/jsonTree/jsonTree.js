
// function tree_click_add_event()
// {
//     var toggler = document.getElementsByClassName("caret");
//     var i;

//     for (i = 0; i < toggler.length; i++) {
//       toggler[i].addEventListener("click", function() {
//         this.parentElement.querySelector(".nested").classList.toggle("active");
//         this.classList.toggle("caret-down");
//       });
//     }
// }



function dragEvenListner(item) 
{
    item.addEventListener('dragstart', handleDragStart, false);
    item.addEventListener('dragenter', handleDragEnter, false);
    item.addEventListener('dragover', handleDragOver, false);
    item.addEventListener('dragleave', handleDragLeave, false);
    item.addEventListener('drop', handleDrop, false);
    item.addEventListener('dragend', handleDragEnd, false);
}



var createSublist = function(container, lst) {

  var ul = document.createElement('ul');   ul.setAttribute('class','nested');

  for (let x in lst) 
  {

    let  el =  lst[x];
    // if( type == "Audio")
    // {

    //    if( lst[x].audio)
    //         el = lst[x].audio {
    // }

    //var row = el[];

    
   
    var li = document.createElement('li');
   

    if( el && !el.length)
    {
        var span = document.createElement('span'); span.setAttribute('class','caret');
        span.innerText = x;
        li.appendChild(span);

        span.addEventListener("click", function() {
        this.parentElement.querySelector(".nested").classList.toggle("active");
        this.classList.toggle("caret-down");
       });


        createSublist(li, el);
    }
    else
    {
        li.innerText = x;
        li.setAttribute('draggable','true');
        li.setAttribute('class','drag');
        li.setAttribute('id', el);

    }


  

    

    // var nodes = row.nodes;

    // if(nodes && nodes.length) {

    //   createSublist(li, nodes);

    // }

    

    ul.appendChild(li);

  }

  

  container.appendChild(ul);

};


  
function handleDragStart(e) {
    this.style.opacity = '0.4';

    if( this.tagName != "LI")
    { 
       return;
    }

    e.dataTransfer.effectAllowed = 'move';
    e.dataTransfer.setData('type', this.innerHTML);
    e.dataTransfer.setData('id', this.id);

  }

function handleDragOver(e) {
  if (e.preventDefault) {
    e.preventDefault();
  }

  e.dataTransfer.dropEffect = 'move';
  
  return false;
}

function handleDragEnter(e) {
  this.classList.add('over');
}

function handleDragLeave(e) {
  this.classList.remove('over');
}

function handleDrop(e) {
  if (e.stopPropagation) {
    e.stopPropagation(); // stops the browser from redirecting.
  }
  
   
  if( this.tagName == "DIV")
  {

    const id = e.dataTransfer.getData('id');

    this.innerHTML = e.dataTransfer.getData('type') +  id;


    if( id.indexOf("/") >= 0)
    playRecording(id, this);
    else
    addCamera(id, this);
  }
  
  return false;
}

function handleDragEnd(e) {
  this.style.opacity = '1';
  
  // items.forEach(function (item) {
  //   item.classList.remove('over');
  // });
}

