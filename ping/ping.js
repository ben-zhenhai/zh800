var ping = require ("net-ping");
var session = ping.createSession ();
var pingSet={};
var fs = require('fs');

process.on('message',function(m){
  console.log('child got message: ',m);
})

if(fs.existsSync('ip.log')) {
  fs.unlinkSync('ip.log');
}

var iniRead = require('./iniRead.js')(function(iniData) {
  initPingSet(pingSet, iniData.ipRange);
  initPingSet(pingSet, iniData.ipRange2);
  setInterval(checkIP, iniData.pingTime,pingSet);
  setInterval(checkIPfail, iniData.checkTime, pingSet, addIpNotice, iniData.checkTimes);
});

function asyncPing(ip,cb) {
  session.pingHost(ip, function (error, target) {
    //console.log('~~~>' + ip);
    if (error) {
      errorAction(ip); //detect ping Fail
    } else {
      //errorAction(ip); //detect ping success
    }
  });
}

// notice ping is blocking function , but nodejs is non-blocking function.
function justPing(ip) {
  var returnValue=false;
  session.pingHost(ip, function (error, target) {
    if (error) {
      returnValue = false;
    } else {
      returnValue = true;
    }
  });
  return returnValue;
}

function errorAction(ip) {
  //console.log(ip+'='+pingSet[ip]+':'+Date());
  pingSet[ip] = pingSet[ip] + 1;
  //console.log('eeerroorr  --- '+ip);
}

function addIpNotice(ipTable) {
  //console.log('iptable: ' + ipTable);
  fs.appendFile('ip.log', '---' + '\n' + JSON.stringify(ipTable) + '\r\n', function(args) {
    // body
  });
  process.send(ipTable);
  //console.log(ipTable);
}

function checkIPfail(pingSet,cb ,checkTimes) {
  var failIpTable = []; // an array to store all data
  var cbRun = 0;
  for(var ip in pingSet) {
    if(pingSet[ip] > checkTimes) {
      var data = {};
      data["IP"] = ip;
      data["DATE"] = Date();
      //data[ip] = Date();
      failIpTable.push(data); // a new key:value pair, and put it in failIpTable array.
      pingSet[ip] = 0;
      cbRun = 1;			
    }
  }

  if(cbRun == 1) {
    cb(failIpTable);
    cbRun = 0;
  }
  //console.log(failIpTable);	
}

function initPingSet(pingSet,ipRange) {
  for(var i = 1; i < 256; i++) {		
    pingSet[ipRange+i] = 0;
  }
}

function checkIP(pingSet) {
  for(var ip in pingSet) {	
    asyncPing(ip);
  }
}
