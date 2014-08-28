var ping = require ("net-ping");
var session = ping.createSession ();
var pingSet={};
var fs = require('fs');
//var cp = require("child_process");

process.on('message',function(m){
	console.log('child got message: ',m);

})


if (fs.existsSync('ip.log')) {
    fs.unlinkSync('ip.log');
}






var iniRead = require('./iniRead.js')(function(iniData){
	//console.log('ready:'+iniData);
	//console.log('ready::'+iniData.pingTime);
	initPingSet(pingSet,iniData.ipRange);
	setInterval(checkIP,iniData.pingTime,pingSet);
	setInterval(checkIPfail, iniData.checkTime,pingSet,addIpNotice,iniData.checkTimes);
});


function asyncPing(ip,cb)
{
	session.pingHost(ip, function (error, target){
		if(error)
		{
			//errorAction(ip);	//detect ping Fail
		}
		else
		{
			errorAction(ip);	//detect ping success
		}
	});
}


// notice ping is blocking function , but nodejs is non-blocking function.
function justPing(ip)
{
	var returnValue=false;
	session.pingHost(ip, function (error, target){
		if (error)
		{
			returnValue= false;
		}
		else
		{
			returnValue= true;
		}
	});
	return returnValue;
}

function errorAction(ip)
{
	//console.log(ip+'='+pingSet[ip]+':'+Date());
	pingSet[ip]=pingSet[ip]+1;
	//console.log('eeerroorr  --- '+ip);
}
function addIpNotice(ipTable)
{
	//console.log(ipTable);
	fs.appendFile('ip.log','---'+'\n'+JSON.stringify(ipTable)+'\r\n',function(args){
		// body
	});
	process.send(ipTable);
	//console.log(ipTable);
}
function checkIPfail(pingSet,cb ,checkTimes)
{
	var failIpTable={};
	var cbRun=0;
	for(var ip in pingSet)
	{
		if(pingSet[ip]>checkTimes)
		{
			failIpTable[ip]=Date();
			pingSet[ip]=0;
			cbRun=1;
			
		}
	}
		if(cbRun==1){
			cb(failIpTable);
			cbRun=0;
		}	
		//console.log(failIpTable);
	
}



function initPingSet(pingSet,ipRange)//,pingSetValue)
{
	for(var i=1;i<256;i++)
	{
		
		pingSet[ipRange+i]=0;
	}
}



function checkIP(pingSet)
{
	for(var ip in pingSet)
	{	
		
		asyncPing(ip);
	}
}

