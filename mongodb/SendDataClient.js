var net = require('net')
var tcpClient =  net.Socket()

var PORT = 5566
var HOST = '192.168.0.181'

var args = process.argv.slice(2)
var fs = require('fs')

var remaining = ''
var line = ''
var index = 0
var last = 0
var hihi = 0

function xyz(input, func) {

    console.log('HI archer');
    input.on('data', function(data) {
        remaining += data;
        readLine(input, func)
    });
    input.on('end', function() {
        if (remaining.length > 0) {
            func_two(remaining);
        }
    })
}


function readLine(input, func) {
    //var remaining = ''

        //remaining += data;
        console.log('remaining:::' + remaining)
        index = remaining.indexOf('\n');
        //var index = remaining.indexOf('\n');
        last  = 0;
        //var last  = 0;
        //while (index > -1) {
        if (index > -1) {
            //var line = remaining.substring(last, index);
            console.log('hihi: ' +hihi++)
            line = remaining.substring(last, index);
            last = index + 1;
            func(line);
            index = remaining.indexOf('\n', last);
        }
        //}
        remaining = remaining.substring(last);
   // });

}

function func(data) {
    console.log('HI' + data);
    tcpClient.connect(
        PORT, HOST,
        function() {
            console.log('Connected to ' + HOST + ':' + PORT)
            console.log('Start to send data: ' + data)
            tcpClient.write(data,function(){
                console.log('callback~~~~~~~')
                console.log('inputFile: ' + inputFile)
                readLine(inputFile, func)
            })
            tcpClient.destroy()
            console.log('destroy')
        }
    )
}
tcpClient.on('data', function() {
        console.log('data')
    }
)

tcpClient.on('end', function() {
        console.log('test')
    }
)


function func_two(data) {
    //console.log(data)
}

var inputFile = fs.createReadStream(args.toString())
//readLine(inputFile, func)
xyz(inputFile, func)
/*
tcpClient.connect(
    PORT, HOST, 
    function() {
        console.log('Connected to ' + HOST + ':' + PORT)
        console.log('Start to send data: ' + args)
        tcpClient.write(args.toString())
        tcpClient.destroy()
    }
)
/*

tcpClient.on(
    'data', 
    function(data) {
        console.log('Received: ' + data.toString())
    }
)


tcpClient.on(
    'close', 
    function() {
        console.log('Connection closed')
        //reConnect()
    }
)

tcpClient.on(
    'error', 
    function(e) {
       if (e.code == 'ECONNREFUSED') {
            console.log('error code: ' + e.code)
            reConnect()
        }
    }
)

function reConnect() {
    tcpClient.setTimeout(1000,
        function() {
            tcpClient.connect(PORT, HOST,
                function() {
                    console.log('Connected to ' + HOST + ':' + PORT)
                }
            )
        }
    )

    console.log('will try to reconnect to ' + HOST + ':' + PORT)
}
*/
