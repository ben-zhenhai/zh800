var net = require('net')
var recordCount = 0;
var BATCH_LIMIT = 10000;

var child_process = require("child_process")
var rabbitSender = child_process.fork('./rabbitSender.js');

function startServer(messageQueue) {

    console.log("START SERVER...")
    var server = net.createServer()
    var count = 0;
    server.on('connection', function(client) {
        client.setEncoding('utf8')
        client.on('data', function(data) {
          if (data != "saveData") {
            console.log("Received data [%d]: %s", count, data)
            rabbitSender.send(data)
            count++;
          }
        })
    })
    
    server.listen(5566)
}

rabbitSender.on('message', function(m) {
  if (m == "RabbitOK") {
    console.log('PARENT got message:', m);
    startServer();
  }
});


