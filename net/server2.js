var net=require('net');
var count=1;
var users={};

var conn = net.connect(3000,'localhost');

conn.on('connect',function(){
	console.log('connect');
});

conn.on('close',function(d){
	console.log('close'+d);
});
conn.on('data',function(d){
	console.log('data'+d);
});
conn.on('error',function(d){
	console.log('error:'+d);
});