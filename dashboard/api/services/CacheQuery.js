function query(url, converter, callback) {
  var resultData = [];

  Cached.native(function(err, collection) {

    if (err) {
      console.error("database error:" + err);
      callback(err, undefined);
      return;
    }

    collection.findOne({url: url}, function(err, data) {

      if (err) {
        callback(err, undefined);
        return;
      }

      if (data) {
        var records = data.value;

        for (var title in records) {
          var processedTitle = title.replace("__DOT__", ".");
          resultData.push(converter(url, processedTitle, records[title]));
        }

        resultData.sort(function(objA, objB) {
          if (objA.name < objB.name) { return -1; }
          if (objA.name > objB.name) { return 1; }
          if (objA.name == objB.name) { return 0; }
        });
      }

      callback(undefined, resultData);
    });
  });

}

function daily(tableName, query, callback) {
  var mongoClient = require('mongodb').MongoClient
  var mongoURL = 'mongodb://localhost/zhenhai'
  var data = [];

  mongoClient.connect(mongoURL, function(err, mongoDB) {
    
    if (err) {
      console.log("Cannot cannto to mongoDB:" + err);
      callback(err, undefined);
      return;
    }

    var cacheTable = mongoDB.collection(tableName)

    var mapFunction = function() {
      var tenMinute = this.timestamp.substring(0, 15) + "0";
      emit(tenMinute + "|" + this.defact_id, {bad_qty: +this.bad_qty, count_qty: +this.count_qty} ) 
    }

    var reduceFunction = function(key, values) { 
      reducedVal = { bad_qty: 0, count_qty: 0 };
      for (var idx = 0; idx < values.length; idx++) {
        reducedVal.bad_qty += values[idx].bad_qty;
        reducedVal.count_qty += values[idx].count_qty;
      }
      return reducedVal;
    }

    var mapReduceOptions = {
      query: query,
      out: {inline: 1}
    }

    cacheTable.mapReduce(mapFunction, reduceFunction, mapReduceOptions, function(err, result) {

      if (err) {
        callback(err, undefined);
        return;
      }

      var resultData = [];

      for (var i = 0; i < result.length; i++) {
        var data = result[i];
        var timestamp = data._id.split("|")[0];
        var defactID = data._id.split("|")[1];
        var countQty = data.value.count_qty;
        var badQty = data.value.bad_qty;
        resultData.push({timestamp: timestamp, defact_id: defactID, count_qty: countQty, bad_qty: badQty});
      }

      resultData.sort(function(objA, objB) {
        var timestampA = objA.timestamp;
        var timestampB = objB.timestamp;
        if (timestampA < timestampB) { return -1; }
        if (timestampA > timestampB) { return 1; }
        if (timestampA == timestampB) { return 0; }
      });

      callback(undefined, resultData);
    });
  });

}

exports.query = query
exports.daily = daily
