var mongoURL = 'mongodb://localhost/zhenhai'
var mongoURLDaily = 'mongodb://localhost/daily'
var mongoURLMonthly = 'mongodb://localhost/monthly'
var mongoURLArchive = 'mongodb://localhost/archive'

var mongoose = require('mongoose')
var conn = mongoose.createConnection(mongoURL)
var Data = require(__dirname + '/data_model').getModel(conn)

var net = require('net')
var server = net.createServer()
 
var array = []
var recordCount = 0;
var BATCH_LIMIT = 1000000;

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

function startServer() {

    var fork = require('child_process').fork;
    var example1 = fork(__dirname + '/processMongo.js');
    var fs = require("fs");

    server.on('connection', function(client) {
        client.setEncoding('utf8')

        client.on('data', function(data) {
          var file = process.env.HOME + "/dataArchive/" + parseDate(data);
          fs.appendFile(file, data, function (err) {});
          if (data != "saveData") {
            example1.send(data);
          }
        })
    })
    
    server.listen(5566)
}

startServer();
