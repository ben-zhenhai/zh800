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

function parseData(data) {
    tmp = data.replace(/(\r\n|\n|\r)/gm,'')
    array = tmp.toString().split(" ")
    dateObject = new Date(array[4] * 1000);
    product = array[1];

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
  
    return {
      raw: record,
      model: new Data(record)
    }
}

function insertToProduct(mongoDB, record) {
  dailyTable = mongoDB.collection("product");
  query = {product: record.raw.product};

  modifyAction = {$inc: {bad_qty: +record.raw.bad_qty, count_qty: +record.raw.count_qty}}

  dailyTable.update(
    query, modifyAction, {upsert: true},
    function(err, data){
      if (err) {
        console.log("save error:" + err);
        return;
      }
    }
  );
}

function insertToProductDetail(mongoDB, record) {
  dailyTable = mongoDB.collection("product-" + record.raw.product);
  query = {timestamp: record.raw.insertDate, mach_id: record.raw.mach_id};

  modifyAction = {$inc: {bad_qty: +record.raw.bad_qty, count_qty: +record.raw.count_qty}}

  dailyTable.update(
    query, modifyAction, {upsert: true},
    function(err, data){
      if (err) {
        console.log("save error:" + err);
        return;
      }
    }
  );
}

function insertToInterval(mongoDB, record) {

  dateObject = new Date(record.raw.emb_date * 1000);
  timestamp = record.raw.insertDate + " " + paddingZero(dateObject.getHours()) + ":" + paddingZero(dateObject.getMinutes());
  timestamp = timestamp.substring(0, 15) + "0";
  intervalTable = mongoDB.collection(record.raw.insertDate);
  query = {timestamp: timestamp, product: record.raw.product, mach_id: record.raw.mach_id, defact_id: record.raw.defact_id};

  modifyAction = {$inc: {bad_qty: +record.raw.bad_qty, count_qty: +record.raw.count_qty}}

  intervalTable.update(
    query, modifyAction, {upsert: true},
    function(err, data){
      if (err) {
        console.log("save error:" + err);
        return;
      }
    }
  );

}

function insertToMonthly(mongoDB, record) {

  dateObject = new Date(record.raw.emb_date * 1000);
  timestamp = record.raw.insertDate.substring(0, 7);
  intervalTable = mongoDB.collection("monthly");
  query = {timestamp: timestamp, mach_id: record.raw.mach_id, defact_id: record.raw.defact_id};

  modifyAction = {$inc: {bad_qty: +record.raw.bad_qty, count_qty: +record.raw.count_qty}}

  intervalTable.update(
    query, modifyAction, {upsert: true},
    function(err, data){
      if (err) {
        console.log("save error:" + err);
        return;
      }
    }
  );

}


function insertToDaily(mongoDB, record) {
  dailyTable = mongoDB.collection("daily");
  query = {timestamp: record.raw.insertDate, mach_id: record.raw.mach_id};

  modifyAction = {$inc: {bad_qty: +record.raw.bad_qty, count_qty: +record.raw.count_qty}}

  dailyTable.update(
    query, modifyAction, {upsert: true},
    function(err, data){
      if (err) {
        console.log("save error:" + err);
        return;
      }
    }
  );
}

function processData(mongoDB, mongoDBMonthly, data) {

  record = parseData(data);

  console.log('add data [' + recordCount + "] / " + data + '...OK.')

  insertToDaily(mongoDB, record);
  insertToProduct(mongoDB, record);
  insertToProductDetail(mongoDB, record);
  insertToInterval(mongoDB, record);
  insertToMonthly(mongoDBMonthly, record);

}

function processFile(filename, mongoDB, mongoDBMonthly) {
  console.log("enter " + filename);
  fs = require("fs");
  lines = fs.readFileSync(filename, {encoding: "UTF-8"}).split("\n");

  for (i = 0; i < lines.length; i++) {
    //console.log("processing [" + recordCount + "]" + lines[i]);
    processData(mongoDB, mongoDBMonthly, lines[i]);
    recordCount++;
  }

  fs.unlinkSync(filename);
  
  console.log("should wait..");
}

function processQueueDir() {
  fs = require("fs");
  queueDir = process.env.HOME + "/dataQueue";
  files = fs.readdirSync(queueDir);

  mongoClient = require('mongodb').MongoClient

  console.log("Prepare to process file queue...");
  
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

      for (var i = 0; i < files.length; i++) {
        if (files[i].indexOf(".txt") > 0) {
          console.log("Processing file " + files[i]);
          processFile(queueDir + "/" + files[i], mongoDB, mongoDBMonthly);
        }
      }

      mongoDB.close();
      mongoDBMonthly.close();
      setTimeout(function(){ processQueueDir() }, 5000);
    });
  });
}

processQueueDir()
