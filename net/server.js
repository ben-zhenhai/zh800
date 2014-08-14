var net=require('net');
var count=1;
var users={};
//users['server']=1;
var server = net.createServer(function(conn){
	conn.setEncoding('utf8');
	console.log('\033[90m new connection ! \033[39m');
	conn.write('\033[92m welcome hello message \033[39m'
		+ '\033[97m'+ count + '\033[39m' 
		+ '\033[92m connections \033[39m');
		count++;
	var d =new Date();	
	console.log(d);
	var nickname=conn.remoteAddress;
	if(users[nickname]){	
		conn.write('\033[93m'+d.getTime()+'>'+ nickname +' already in use, try again: \033[39m');
		console.log('\033[93m'+d.getTime()+'>'+ nickname +' already in use, try again: \033[39m');
		for(var j=1;;j++)
		{
			nickname=nickname+'-'+j;
			if(users[nickname])
				continue;
			else
			{
				users[nickname] = conn;
				console.log('\033[90m'+d.getTime()+'>'+nickname+' joined the room \033[39m\n');
				for(var i in users){
					users[i].write('\033[90m '+d.getTime()+'>'+nickname+' joined the room \033[39m\n');
					
				}
				break;
			}
		}
	}else{
		//nickname=data;
		
		users[nickname] = conn;
		for(var i in users){
			users[i].write('\033[90m '+d.getTime()+'>'+nickname+' joined the room \033[39m\n');
			console.log('\033[90m '+d.getTime()+'>'+nickname+' joined the room \033[39m\n');
		}
	}
	
		
	
	conn.on('close',function(d){
		console.log('close'+d);	
	});

	conn.on('data',function(data){
		var d =new Date();
		data=data.replace('\r\n','');
			console.log('\033[96m '+d.getTime()+'>'+nickname+': \033[39m'+data);
			for(var i in users){
					users[i].write('\033[96m '+d.getTime()+'>'+nickname+': \033[39m'+data+ '\n');
			}		
	});

	conn.on('close',function(){
		count--;
	});
});

server.listen(3000,function(){
	console.log('\033[96m server listen on *:3000\033[39m');
})