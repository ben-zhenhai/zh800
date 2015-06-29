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
var ipMappingFile = 'ipMapping_2.json';
var ipMappingData = {};

fs.readFile(ipMappingFile, 'utf8', function(err, data) {
  if(err) {
    console.log('read ipMappingFile error');
    return;
  }
  ipMappingData = JSON.parse(data);
  
  //console.log(ipMappingData);
  //var tmp = ipMappingData.filter(function(obj) {
  //  return obj.IP === '192.168.10.2';
  //});
  //console.log('~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~>>>>' + tmp[0].ID);
});

function initIpArray(ipRange, upper, lower, ignoreIPs) {
  for (var x = parseInt(lower); x <= parseInt(upper); x++) {
    pingSet[ipRange+x] = 0;
  }
  for(var x = 0; x < ignoreIPs.length; x++) {
    delete pingSet[ipRange+ignoreIPs[x]];
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
    // console.log(ip + '  ' + pingSet[ip] + ' ' + errorTimes);
    if(pingSet[ip] >= errorTimes) { 
      var tmp = ipMappingData.filter(function(obj) {
        return obj.IP === ip;
      });
      var data = {};
      data["ID"] = tmp[0].ID;
      //console.log('~~~~~~~~>>>' + data["ID"]);
      data["TYPE"] = tmp[0].TYPE;
      //console.log('~~~~~~~~>>>' + data["TYPE"]);
      data["IP"] = ip;
      //console.log('~~~~~~~~>>>' + data["IP"]);
      data["NOTE"] = tmp[0].NOTE;
      //console.log('~~~~~~~~>>>' + data["NOTE"]);
      data["DATE"] = Date();
      //console.log('~~~~~~~~>>>' + data["DATE"]);
      failedIp.push(data);
      pingSet[ip] = 5;
      //console.log('~~~~~~~~>>>' + pingSet[ip]);
    }
  }
  console.log("ping_2.js send failedIp result");
  process.send(failedIp);
}

process.on('message', function(msg) {
  console.log('ping_2.js get process msg: ' + msg);
  if(msg === 'init') {
    checkAliveResult(1);
  }
});

var iniRead = require('./iniRead.js')(function(iniData) {
  initIpArray(iniData.ipRange1.range, iniData.ipRange1.upper, iniData.ipRange1.lower, iniData.ipRange1.ignore);
  initIpArray(iniData.ipRange2.range, iniData.ipRange2.upper, iniData.ipRange2.lower, iniData.ipRange2.ignore);
  setInterval(checkAlive, iniData.pingTime);
  setInterval(checkAliveResult, iniData.checkTime, iniData.errorTimes);
});
