var mongoURLDaily = 'mongodb://localhost/daily'
var mongoURLMonthly = 'mongodb://localhost/monthly'

var net = require('net')
var server = net.createServer()
 
var recordCount = 0;

function paddingZero(number) {
  if (+number < 10) {
    return "0" + number;
  } else {
    return number;
  }
}

function parseData(data) {
    var array = data.replace(/(\r\n|\n|\r)/gm,'').toString().split(" ");
    var dateObject = new Date(array[4] * 1000);
    var product = array[1];

    record = {
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
       product: product,
       insertDate: dateObject.getFullYear() + "-" + paddingZero(+dateObject.getMonth()+1) + "-" + paddingZero(+dateObject.getDate())
    }
  
    return record;
}

function insertToProduct(mongoDB, mongoDBMonthly, record, ch, msg) {

  var dailyTable = mongoDB.collection("product");
  var query = {product: record.product};

  var modifyAction = {$inc: {bad_qty: +record.bad_qty, count_qty: +record.count_qty}}

  dailyTable.update(
    query, modifyAction, {upsert: true},
    function(err, data){
      if (err) {
        console.log("save error:" + err);
        return;
      }
      insertToProductDetail(mongoDB, mongoDBMonthly, record, ch, msg);      
    }
  );
}

function insertToProductDetail(mongoDB, mongoDBMonthly, record, ch, msg) {

  var dailyTable = mongoDB.collection("product-" + record.product);
  var query = {timestamp: record.insertDate, mach_id: record.mach_id};

  var modifyAction = {$inc: {bad_qty: +record.bad_qty, count_qty: +record.count_qty}}

  dailyTable.update(
    query, modifyAction, {upsert: true},
    function(err, data){
      if (err) {
        console.log("save error:" + err);
        return;
      }
      insertToInterval(mongoDB, mongoDBMonthly, record, ch, msg)
    }
  );
}

function insertToInterval(mongoDB, mongoDBMonthly, record, ch, msg) {

  var dateObject = new Date(record.emb_date * 1000);
  var timestamp = record.insertDate + " " + paddingZero(dateObject.getHours()) + ":" + paddingZero(dateObject.getMinutes());
  var timestamp = timestamp.substring(0, 15) + "0";
  var intervalTable = mongoDB.collection(record.insertDate);
  var query = {timestamp: timestamp, product: record.product, mach_id: record.mach_id, defact_id: record.defact_id};

  var modifyAction = {$inc: {bad_qty: +record.bad_qty, count_qty: +record.count_qty}}

  intervalTable.update(
    query, modifyAction, {upsert: true},
    function(err, data){
      if (err) {
        console.log("save error:" + err);
        return;
      }
      insertToMonthly(mongoDB, mongoDBMonthly, record, ch, msg)
    }
  );

}

function insertToMonthly(mongoDB, mongoDBMonthly, record, ch, msg) {

  dateObject = new Date(record.emb_date * 1000);
  timestamp = record.insertDate.substring(0, 7);
  intervalTable = mongoDBMonthly.collection("monthly");
  query = {timestamp: timestamp, mach_id: record.mach_id, defact_id: record.defact_id};

  modifyAction = {$inc: {bad_qty: +record.bad_qty, count_qty: +record.count_qty}}

  intervalTable.update(
    query, modifyAction, {upsert: true},
    function(err, data){
      if (err) {
        console.log("save error:" + err);
        return;
      }
      insertToDaily(mongoDB, mongoDBMonthly, record, ch, msg)
    }
  );

}


function insertToDaily(mongoDB, mongoDBMonthly, record, ch, msg) {
  var dailyTable = mongoDB.collection("daily");
  var query = {timestamp: record.insertDate, mach_id: record.mach_id};

  var modifyAction = {$inc: {bad_qty: +record.bad_qty, count_qty: +record.count_qty}}

  dailyTable.update(
    query, modifyAction, {upsert: true},
    function(err, data){
      if (err) {
        console.log("save error:" + err);
        return;
      }
      insertToRaw(mongoDB, mongoDBMonthly, record, ch, msg);
    }
  );
}

function insertToRaw(mongoDB, mongoDBMonthly, record, ch, msg) {

  var rawTable = mongoDB.collection("raw");
  rawTable.insert(record, function(err, data) {
    if (err) {
      console.log("save error:" + err);
      return;
    }
    recordCount++;
  });
  ch.ack(msg);

}

function processData(mongoDB, mongoDBMonthly, data, ch, msg) {

  var record = parseData(data);

  console.log('add data [' + recordCount + "] / " + data + '...OK.')
  insertToProduct(mongoDB, mongoDBMonthly,record, ch, msg);
}

function startProcessingServer(callback) {
  var mongoClient = require('mongodb').MongoClient
  
  mongoClient.connect(mongoURLDaily, function(err, mongoDB) {
  
    if (err) {
      console.log("Cannot cannto to mongoDB:" + err);
      return;
    }

    mongoClient.connect(mongoURLMonthly, function(err, mongoDBMonthly) {
    
      if (err) {
        console.log("Cannot cannto to mongoDB:" + err);
        return;
      }

      callback(mongoDB, mongoDBMonthly);
    });
  });
}

function prepareRabbitMQ(mongoDB, mongoDBMonthly) {

  var rabbitMQ = require('amqplib/callback_api')
  var dataQueue = "rawDataLine"
  var count = 0
  
  rabbitMQ.connect('amqp://localhost', function(err, conn) {

    if (err != null) {
      console.log("cannot connection to rabbitMQ server, error:" + err);
      process.exit(-1);
    }
 
    conn.createChannel(on_open);

    function on_open(err, ch) {

      if (err != null) {
        console.log("canno open rabbitMQ cahnnel, error:" + err);
        process.exit(-1);
      }

      ch.assertQueue(dataQueue);
      ch.prefetch(1);
      ch.consume(dataQueue, function(msg) {
        if (msg !== null) {
          var data = msg.content.toString();
          processData(mongoDB, mongoDBMonthly, data, ch, msg);
        }
      });
    }
  });
}

startProcessingServer(prepareRabbitMQ)
