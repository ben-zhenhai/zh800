var net=require('net');
var count=1;
var users={};
//users['server']=1;
var server = net.createServer(function(conn){
	conn.setEncoding('utf8');
	
	var nickname;
	console.log('\033[90m new connection ! \033[39m');
	conn.write('\033[92m welcome hello message \033[39m'
		+ '\033[97m'+ count + '\033[39m' 
		+ '\033[92m connections \033[39m');
		count++;
		

	conn.on('close',function(d){
		console.log('close'+d);	
	});
	
	conn.on('data',function(data){
		
		
		data=data.replace('\r\n','');
		if(!nickname){
			
			if(users[data]){
			//	console.log('1'+data+'1'+users[data]);
				conn.write('\033[93m> nickname already in use, try again: \033[39m');
				console.log('\033[93m> nickname already in use, try again: \033[39m');
				return;
			}else{
				nickname=data;
				
				users[nickname] = conn;
				for(var i in users){
					users[i].write('\033[90m >'+nickname+' joined the room \033[39m\n');
					console.log('\033[90m >'+nickname+' joined the room \033[39m\n');
				}
			}

	//		console.log('if(!nickname=!',data);
		}else{
	//		console.log('nickname='+nickname);
			console.log('\033[96m >'+nickname+': \033[39m'+data);
			for(var i in users){
				//if (i!=nickname)
				{	
					users[i].write('\033[96m >'+nickname+': \033[39m'+data+ '\n');
				}
			}
		}
	});
	conn.on('close',function(){
		count--;
		
		
	});
});

server.listen(3000,function(){
	console.log('\033[96m server listen on *:3000\033[39m');
})