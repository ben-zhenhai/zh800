var mongoURL = 'mongodb://localhost/zhenhai'
var mongoose = require('mongoose')
var conn = mongoose.createConnection(mongoURL)
var Data = require(__dirname + '/data_model').getModel(conn)

var net = require('net')
var server = net.createServer()

var array = []

function parseData(data) {
    var tmp = data.replace(/(\r\n|\n|\r)/gm,'')
    array = tmp.toString().split(" ")
    //array = tmp.toString().split(',')
  
    console.log('size: ' + array.length)

    /*
    array.forEach(function(data) {
        console.log('hi: ' + data)
    })
    */
  
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

    var urlMapper = require("./URLMapper");
    var count = 0;

    server.on('connection', function(client) {
        client.setEncoding('utf8')
    
        client.on('data', function(data) {
            var record = parseData(data);
            record.mongoose.save(function(error) {
                if (error) console.error(error)

                console.log('add data ok.')
                urlMapper.addToCache(mongoDB, record.raw);
            })
            console.log('test')
        })
    
        client.on('close', function() {
            if (count % 100 == 0) {
                urlMapper.saveCache(mongoDB);
            }
            count++;
            console.log('close')
        })
    
        client.on('end', function() {
            console.log('end')
        })
    })
    
    server.listen(5566)
}

var mongoClient = require('mongodb').MongoClient

mongoClient.connect(mongoURL, function(err, mongoDB) {

    if (err) {
      console.log("Cannot cannto to mongoDB:" + err);
      return;
    }

    startServer(mongoDB);
});
