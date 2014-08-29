var net = require('net')
var tcpClient = new net.Socket()

var PORT = 5566
var HOST = 'mongo_server'

var args = process.argv.slice(2)

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
*/

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
