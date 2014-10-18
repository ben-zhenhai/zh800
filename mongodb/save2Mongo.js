var net = require('net')
var recordCount = 0;
var BATCH_LIMIT = 10000;

function paddingZero(number) {
  if (+number < 10) {
    return "0" + number;
  } else {
    return number;
  }
}

function parseDate(data) {
    var tmp = data.replace(/(\r\n|\n|\r)/gm,'')
    var array = tmp.toString().split(" ")
    var dateObject = new Date(array[4] * 1000);
    return dateObject.getFullYear() + "-" + paddingZero(+dateObject.getMonth()+1) + "-" + paddingZero(+dateObject.getDate());
}

var child_process = require("child_process")
var rabbitSender = child_process.fork('./rabbitSender.js');

function startServer(messageQueue) {

    console.log("START SERVER...")
    var server = net.createServer()
    server.on('connection', function(client) {
        client.setEncoding('utf8')
        client.on('data', function(data) {
          var fs = require("fs")
          var file = process.env.HOME + "/dataArchive/" + parseDate(data);

          if (data != "saveData") {
            fs.appendFile(file, data + "\n", function (err) {});
            console.log("Received data:" + data)
            rabbitSender.send(data)
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


