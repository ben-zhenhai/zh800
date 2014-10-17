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

var recordCount = 0;
var batchCount = 0;
var BATCH_LIMIT = 10000;

function startServer(mongoDB, mongoDBMonthly) {

    var fork = require('child_process').fork;
    var fs = require("fs");

    server.on('connection', function(client) {
        client.setEncoding('utf8')

        client.on('data', function(data) {
          var file = process.env.HOME + "/dataArchive/" + parseDate(data);

          if (data != "saveData") {

            fs.appendFile(file, data + "\n", function (err) {});
            var tmpFile = process.env.HOME + "/dataQueue/tmpData" + batchCount;

            fs.appendFile(tmpFile, data + "\n", function (err) {
              recordCount++;
              console.log("Add record[" + recordCount + "] to file...");
              if (recordCount % BATCH_LIMIT == 0) {
                fs.renameSync(tmpFile, tmpFile + ".txt");
                batchCount++;
              }
            });
          }
        })
    })
    
    server.listen(5566)
}
startServer();
