var net = require('net')

var PORT = 5566
var HOST = 'localhost'

var args = process.argv.slice(2)
var fs = require('fs')

var remaining = ''
var line = ''
var index = 0
var last = 0
var lineCount = 0;


function setupListener(input, func) {

    input.on('data', function(data) {
        remaining += data;
        readLine(input, func)
    });

    input.on('end', function() {})
}


function readLine(input, func) {
    index = remaining.indexOf('\n');
    last  = 0;

    if (index > -1) {
        line = remaining.substring(last, index);
        last = index + 1;
        func(line);
        index = remaining.indexOf('\n', last);
    } else {
        sendSaveCommand();
    }

    remaining = remaining.substring(last);
}

function sendSaveCommand() {

    var tcpClient =  net.Socket()

    tcpClient.connect(
        PORT, HOST,
        function() {
            tcpClient.write("saveData",function(){
                tcpClient.destroy()
            })
        }
    )
}


function func(data) {

    var tcpClient =  net.Socket()

    tcpClient.connect(
        PORT, HOST,
        function() {
            console.log('Start to send data: ' + data)
            tcpClient.write(data,function(){
                readLine(inputFile, func)
                tcpClient.destroy()
            })
        }
    )

}


var inputFile = fs.createReadStream(args.toString())

setupListener(inputFile, func)
