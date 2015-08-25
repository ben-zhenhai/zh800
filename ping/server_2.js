var express = require('express');
var app = express();
// var app = require('express')();
var http = require('http').Server(app);
var io = require('socket.io')(http);
var child_process = require('child_process');
var n = child_process.fork('./ping_2.js');
var fs = require('fs');
var email = require('emailjs');
var dailyCount = child_process.fork('./dailySum.js');
var dbConn = child_process.fork('./dbConnect.js');
// var checkError = child_process.fork('./sendMail.js');
var errorRate = 0.0;
var errerAlert = 0.30;
var mailServer = email.server.connect({
  user: "freeman@zhenhai.com.tw",
  password: "aaaa9881#",
  host: "www.zhenhai.com.tw",
  ssl: false
});
var resultArray = {};

console.log("Alive server start.");

n.on('message',function(m){
  io.emit('alive', JSON.stringify(m));
  console.log("-------->> get alive data!");
  //for (var i = 0; i < m.length; i++) {
  //  console.log(m[i]);
  //}
  //console.log(JSON.stringify(m));
  dbConn.send('init');
});

dailyCount.on('message', function(m) {
  console.log("-------->> get daily count data!");
  io.emit('dailyCount', JSON.stringify(m));
  //console.log(m);
});

dbConn.on('message', function(m) {
  console.log("-------->> get machine status data!");
  io.emit('dbConnect', JSON.stringify(m)); 
  //console.log(m);
});

app.use(express.static(__dirname + '/javascript'));
app.use(express.static(__dirname + '/images'));

app.get('/', function(req, res) {
  res.sendfile('boxStatus.html');
});

//app.get('/pic2', function(req, res) {
//  res.sendfile('boxStatusPic.html');
//});

app.get('/pic', function(req, res) {
  res.sendfile('boxStatusPic2.html');
});

io.on('connection', function(socket) {
  console.log('a user connected');

  //n.send('init');
  //dailyCount.send('init');

  socket.on('boxStatusPic', function(msg) {
    n.send('init');
    dailyCount.send('init');
    //dbConn.send('init');
    console.log('get boxStatusPic.html msg:' + msg);
  });

  socket.on('errorRate', function(msg) {
    errorRate = msg;
    console.log("errorRate: " + errorRate);
  });

  socket.on('disconnect', function() {
    console.log('user disconnected');
  });
});

http.listen(8080, function() {
  console.log('listening on 8080');
});

function sendMail(msg) {
  mailServer.send(
    {
      text:msg,
      from: "ZhenHai 機台存活率檢查 <freeman@zhenhai.com.tw>",
      to: "z_h_e_n_h_a_i@mailinator.com",
      subject: "台容 謝崗廠 機台存活率過低"
    },
    function(err, msg) {
      console.log("err: " + err || msg);
    }
  );
}

function checkErrorRate() {
  if (errorRate > errerAlert) {
    sendMail("出錯機台 " + (errorRate* 100) + " %");
  }
}

setInterval(checkErrorRate, 300000);
