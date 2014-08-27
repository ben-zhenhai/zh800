// example: 
// session.pingHost ('192.168.0.148', function (error, target) {
//     if (error)
//         if (error instanceof ping.RequestTimedOutError)
//             console.log (target + ": Not alive");
//         else
//             console.log (target + ": " + error.toString ());
//     else
//         console.log (target + ": Alive");
// });

// console.log('just'+justPing('192.168.0.148'));


var ping = require ("net-ping");
var session = ping.createSession ();
var pingSet={};
var http = require('http');
var fs = require('fs');
var express = require('express');
var connect = require("connect");
var socketio = require("socket.io");
var morgan = require('morgan');
var bodyParser = require('body-parser');
var app = express();
var routes = require('./routes')(app);
var io;
var server = http.createServer(app);
fs.unlinkSync('ip.log');

var iniRead = require('./iniRead.js')(function(iniData){
	console.log('ready:'+iniData);
	console.log('ready::'+iniData.pingTime);
	initPingSet(pingSet,iniData.ipRange);
	setInterval(checkIP,iniData.pingTime,pingSet);
	setInterval(checkIPfail, iniData.checkTime,pingSet,addIpNotice,iniData.checkTimes);
});
io = socketio.listen(server);
io.on("connection", function(socket) {
	socket.on("echo", function(data) {
		console.log('--message');
	});
});


app.set('view engine', 'jade');
app.set('views', './views');
app.use(require('stylus').middleware({
     src: './views',
     compress: true
}));
app.use(express.static('./public'));
app.use(express.static('./javascript'));
app.use(express.static('./files'));
app.use(express.static('./images'));
app.use(express.static('./views'));
app.use(express.static('./stylesheets'));
app.use(morgan('tiny',{
  stream : fs.createWriteStream('app.log', {'flags':'w'})
}));
app.use(morgan('tiny'));
app.use(bodyParser.json());
app.use(bodyParser.urlencoded());
app.post('/addIpMaping', function(req, res) {
     var name = req.body.IP;
     var source = req.body.name;
     console.log('Searching for: ' + name);
     console.log('From: ' + source);
     res.json(req.body);
     //res.send(name + ' : ' + source);
});


server.listen(3000, function(){
     console.log('Express server listening on port ' + 3000);
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
	console.log(ipTable);
	fs.appendFile('ip.log', JSON.stringify(ipTable)+'\r\n',function(args){
		// body
	});
	io.on("connection", function(socket) {
  	//for(var i in ipTable)
  	{
  		console.log('--'+ipTable+'--');
  		socket.emit("echo", ipTable);	
  	}
    
  });
	
//	console.log(ipTable);
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


// ---- MongoDB START ----
var mongoose = require('mongoose')
var conn_mongo = mongoose.createConnection('mongodb://localhost/zhenhai') 
var Data = require(__dirname + '/data_model').getModel(conn_mongo)

var net = require('net')
var server_net = net.createServer()

var array = []

server_net.on('connection', function(client) {
    client.setEncoding('utf8')

    client.on('data', function(data) {
        var tmp = data.replace(/(\r\n|\n|\r)/gm,'')
        //array = tmp.toString().split(/\t+/g)
        array = tmp.toString().split(',')

        console.log('size: ' + array.length)
        array.forEach(function(data) {
            console.log('hi: ' + data)
        })

        var data = new Data({
           order_type: array[0],
           lot_no: array[1],
           work_qty: array[2],
           count_qty: array[3],
           emb_date: array[4],
           bad_qty: array[5],
           mach_ip: array[6],
           defact_id: array[7],
           mach_id: array[8],
           work_id: array[9],
           CX: array[10],
           DX: array[11],
           LC: array[12],
           mach_status: array[13]
        })
        data.save(function(error) {
            if (error) console.error(error)
            console.log('add data ok.')
        })
    })

    client.on('close', function() {
        console.log('close')
    })

    client.on('end', function() {
        console.log('end')
    })
})

server_net.listen(5566)
console.log('listen 5566 port')
// ---- MongoDB END ----
