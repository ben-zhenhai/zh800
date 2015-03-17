var express = require('express');
var app = express();
//var app = require('express')();
var http = require('http').Server(app);
var io = require('socket.io')(http);
var child_process = require('child_process');
var n = child_process.fork('./ping_2.js');
var fs = require('fs');
var dailyCount = child_process.fork('./dailySum.js');

console.log("Alive server start.");

n.on('message',function(m){
  io.emit('alive', JSON.stringify(m));
  console.log("-------->> get alive data!");
  //for (var i = 0; i < m.length; i++) {
  //  console.log(m[i]);
  //}
  //console.log(JSON.stringify(m));
});

dailyCount.on('message', function(m) {
  console.log("-------->> get daily count data!");
  io.emit('dailyCount', JSON.stringify(m));
  console.log(m);
});

app.use(express.static(__dirname + '/javascript'));

app.get('/', function(req, res) {
  res.sendfile('boxStatus.html');
});

app.get('/pic', function(req, res) {
  res.sendfile('boxStatusPic.html');
});

io.on('connection', function(socket) {
  console.log('a user connected');

  //n.send('init');
  //dailyCount.send('init');

  socket.on('boxStatusPic', function(msg) {
    n.send('init');
    dailyCount.send('init');
    console.log('get boxStatusPic.html msg:' + msg);
  });
  socket.on('disconnect', function() {
    console.log('user disconnected');
  });
});

http.listen(8080, function() {
  console.log('lostening on 8080');
});
