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
        if (timestampA == timestampB) { 
          if (+(objA.count_qty) > +(objB.count_qty)) {
            return -1;
          } else if (+(objA.count_qty) < +(objB.count_qty)) {
            return 1;
          } else {
            if (+(objA.defact_id) < +(objB.defact_id)) {
              return -1;
            } else if (+(objA.defact_id) > +(objB.defact_id)) {
              return 1;
            } else {
              return 0;
            }
          }
        }
      });

      callback(undefined, resultData);
    });
  });

}

exports.daily = daily
