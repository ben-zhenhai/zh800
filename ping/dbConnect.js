var MongoClient = require('mongodb').MongoClient;
var assert = require('assert');

var url = 'mongodb://localhost:27017/zhenhai';

var data = [];

var getStatus = function(db, callback) {
  //var collection = db.collection('dailyMachineCount');
  var collection = db.collection('machineStatus');

  /*
  collection.find({}).toArray(function(err, docs) {
      console.log("Fould the following records...");
      console.log(docs);
  });
  */
  var stream = collection.find({}).stream();
  stream.on("data", function(doc) {
    //console.log(doc.machineID + "::" + doc.status + "::" + doc.count_qty);
    var item = {};
    item["ID"] = doc.machineID;
    item["STATUS"] = doc.status;
    item["QTY"] = 0;
    data.push(item);
  });

  stream.on("end", function() {
    db.close();
    process.send(data);
    callback();
  });
};

var getQty = function(db, callback) {
  var collection = db.collection('dailyMachineCount');

  var stream = collection.find({}).stream();
  stream.on("data", function(doc) {
    var tmp = data.filter(function(item) {
      return item.machineID === doc.machineID;
    });
    console.log(">>>>" + tmp[0].machineID + ">>>>" + doc.machineID + ">>>>" + doc.count_qty);
  });

  stream.on("end", function() {
    callback();
  });
}

function queryDatabase() {
  MongoClient.connect(url, function(err, db) {
    assert.equal(null, err);
    console.log("Connected correctly to server");

    getStatus(db, function() {
        db.close();
	//queryDatabaseQty();
        // process.exit(code=0);
    });
  });
}

function queryDatabaseQty() {
  MongoClient.connect(url, function(err, db) {
    assert.equal(null, err);
    console.log("Connected correctly to server");

    getQty(db, function() {
      db.close();
      // process.exit(code=0);
    });
  });
}


process.on("message", function(msg) {
  console.log("dbConnect.js get process msg: " + msg);
  if (msg === 'init') {
    queryDatabase();
  }
});

//setInterval(queryDatabase,300000);
