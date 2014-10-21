var app = require('express')();
var http = require('http').Server(app);
var io = require('socket.io')(http);
var child_process = require('child_process');
var n = child_process.fork('./ping.js');

/*
wsServer.on('connection', function(ws) {
  wsServer.on('message', function(message) {
    console.log('received: %s', message);
  });
});


wsServer.broadcast = function(data) {
  for(var i in this.clients) {
    this.clients[i].send(data);
  }
};
*/

n.on('message',function(m){
  io.emit('freeman', JSON.stringify(m));
  console.log('hihi');
});

app.get('/', function(req, res) {
  res.sendfile('boxStatus.html');
});

io.on('connection', function(socket) {
  console.log('a user connected');
  socket.on('disconnect', function() {
    console.log('user disconnected');
  });
});

http.listen(8080, function() {
  console.log('lostening on 8080');
});
