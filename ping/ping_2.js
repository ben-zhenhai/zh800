var ping = require('net-ping');
var options = {
  networkProtocol: ping.NetworkProtocol.IPv4,
  packetSize: 12,
  retries: 0,
  sessionId: (process.pid % 65535),
  timeout: 500,
  ttl: 128
};
var session = ping.createSession(options);
var pingSet = {};
var fs = require('fs');

process.on('message', function(msg) {
  console.log('child get ' + msg);
  if(msg == 'firstInit') {
    checkAliveResult(10);
  }
});

function initIpArray(ipRange) {
  for (var x = 0; x < 256; x++) {
    pingSet[ipRange+x] = 0;
  }
}

function queryHost(host) {
  session.pingHost(host, function(err, ip) {
    if(err) {
      pingSet[ip] = pingSet[ip] + 1;
    } else {
      pingSet[ip] = 0;
    }
  });
}

function checkAlive() {
  for(var ip in pingSet) {
    setTimeout(queryHost(ip), 5000);
  }
}

function checkAliveResult(errorTimes) {
  var failedIp = [];
  for(var ip in pingSet) {
    console.log(ip + '  ' + pingSet[ip] + ' ' + errorTimes);
    if(pingSet[ip] >= errorTimes) {
      var data = {};
      data["IP"] = ip;
      data["DATE"] = Date();
      failedIp.push(data);
      pingSet[ip] = parseInt(errorTimes);
    }
  }
  process.send(failedIp);
}

var iniRead = require('./iniRead.js')(function(iniData) {
  initIpArray(iniData.ipRange);
  initIpArray(iniData.ipRange2);
  setInterval(checkAlive, iniData.pingTime);
  setInterval(checkAliveResult, iniData.checkTime, iniData.errorTimes);
});
