var MongoClient = require('mongodb').MongoClient;
var assert = require('assert');

var url = 'mongodb://localhost:27017/zhenhai';

var findData = function(db, callback) {
  var collection = db.collection('dailyMachineCount');

  /*
  collection.find({}).toArray(function(err, docs) {
      console.log("Fould the following records...");
      console.log(docs);
  });
  */
  var stream = collection.find({}).stream();
  var data = [];
  stream.on("data", function(doc) {
    console.log(doc.machineID + "::" + doc.status + "::" + doc.count_qty);
    var item = {};
    item["ID"] = doc.machineID;
    item["STATUS"] = doc.status;
    item["QTY"] = doc.count_qty;
    data.push(item);
  });

  stream.on("end", function() {
    db.close();
    process.send(data);
  });
};

function queryDatabase() {
  MongoClient.connect(url, function(err, db) {
    assert.equal(null, err);
    console.log("Connected correctly to server");

    findData(db, function() {
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
