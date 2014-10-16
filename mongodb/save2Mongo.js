var mongoURL = 'mongodb://localhost/zhenhai'
var mongoDaily = 'mongodb://localhost/daily'
var mongoose = require('mongoose')
var conn = mongoose.createConnection(mongoURL)
var Data = require(__dirname + '/data_model').getModel(conn)

var net = require('net')
var server = net.createServer()
 
var array = []
var recordCount = 0;
var BATCH_LIMIT = 1000000;

function parseData(data) {
    var tmp = data.replace(/(\r\n|\n|\r)/gm,'')
    array = tmp.toString().split(" ")
    var dateObject = new Date(array[4] * 1000);

    function paddingZero(number) {
      if (+number < 10) {
        return "0" + number;
      } else {
        return number;
      }
    }

    var record = {
       order_type: array[0],
       lot_no: array[1],
       work_qty: array[2],
       count_qty: array[3],
       emb_date: array[4],
       bad_qty: array[5],
       mach_ip: array[6],
       defact_id: array[7],
       mach_id: array[8],
       work_id: array[9],
       CX: array[10],
       DX: array[11],
       LC: array[12],
       mach_status: array[13],
       insertDate: dateObject.getFullYear() + "-" + paddingZero(+dateObject.getMonth()+1) + "-" + paddingZero(+dateObject.getDate())
    }
  
    return {
      raw: record,
      model: new Data(record)
    }
}

function startServer(mongoDB) {

    server.on('connection', function(client) {
        client.setEncoding('utf8')
    
        client.on('data', function(data) {

            var record = parseData(data);

            console.log('add data [' + recordCount + "] / " + data + '...OK.')
            recordCount++;
            var dailyTable = mongoDB.collection(record.raw.insertDate);
            dailyTable.insert(record.raw);

            if (recordCount > BATCH_LIMIT) {
               recordCount = 0;
            }

            record.model.save(function(error) {
                if (error) {
                    console.error(error)
                }
            })
        })
    })
    
    server.listen(5566)
}

function initMongoServer(callback) {
  var mongoClient = require('mongodb').MongoClient
  
  mongoClient.connect(mongoDaily, function(err, mongoDB) {
  
    if (err) {
      console.log("Cannot cannto to mongoDB:" + err);
      return;
    }

    callback(mongoDB);
  });
}

initMongoServer(startServer);
