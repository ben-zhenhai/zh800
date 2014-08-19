window.onload = function() {
	
	var socket=io.connect();
	
function addMessage(form, text)
{
	var li = document.createElement('li');
	li.innerHTML = '<b>'+ form + '</b> : ' + text;
	
}



 function log (message, options) {
    var $el = $('<li>').addClass('log').text(message);
    addMessageElement($el, options);
}

 function addMessageElement (el, options) {
    var $el = $(el);

    // Setup default options
    if (!options) {
      options = {};
    }
    if (typeof options.fade === 'undefined') {
      options.fade = true;
    }
    if (typeof options.prepend === 'undefined') {
      options.prepend = false;
    }

    // Apply options
    if (options.fade) {
      $el.hide().fadeIn(FADE_TIME);
    }
    if (options.prepend) {
      $messages.prepend($el);
    } else {
      $messages.append($el);
    }
    $messages[0].scrollTop = $messages[0].scrollHeight;
  }