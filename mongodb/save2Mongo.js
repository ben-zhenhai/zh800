var mongoURL = 'mongodb://localhost/zhenhai'
var mongoose = require('mongoose')
var conn = mongoose.createConnection(mongoURL)
var Data = require(__dirname + '/data_model').getModel(conn)

var net = require('net')
var server = net.createServer()
var statisticCache = require(__dirname + "/StatisticCache");
 
var array = []
var recordCount = 0;
var BATCH_SAVE_POINT = 5000;
var BATCH_LIMIT = 1000000;

function parseData(data) {
    var tmp = data.replace(/(\r\n|\n|\r)/gm,'')
    array = tmp.toString().split(" ")

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
       mach_status: array[13]
    }
  
    var data = new Data(record)
  
    return {
      raw: record,
      mongoose: data
    }
}

function startServer(mongoDB) {

    server.on('connection', function(client) {
        client.setEncoding('utf8')
    
        client.on('data', function(data) {

            if (data == "saveData") {
                console.log("Receive save command...");
                statisticCache.saveCache(mongoDB);
            } else {
                var record = parseData(data);

                console.log('add data [' + recordCount + "] / " + data + '...OK.')

                statisticCache.addToCache(mongoDB, record.raw);

                recordCount++;

                if (recordCount % BATCH_SAVE_POINT == 0) {
                   console.log("save statistic data...OK");
                   statisticCache.saveCache(mongoDB);
                }

                if (recordCount > BATCH_LIMIT) {
                   recordCount = 0;
                }

                record.mongoose.save(function(error) {
                    if (error) {
                        console.error(error)
                    }
                })
            }
        })
    })
    
    server.listen(5566)
}

statisticCache.initCache(mongoURL, startServer)
