var socket = io.connect();
    
    socket.on("echo", function(data) {
      console.log(data);
      document.write(data);
    });