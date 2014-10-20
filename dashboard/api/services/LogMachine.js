exports.cachedJSON = function() {

  function overview(convert, callback) {

    var mapReducer = MapReducer.defineOn({
      model: "dailyDefact",
      groupingFunction: function (data) { return data.mach_id },
      mongoFilters: {
        bad_qty: {$gt: 0}
      },
      converter: function (data) {
        return {
          name: data._id,
          value: data.value,
          link: "/machine/" + data._id
        }
      }
    });

    mapReducer(callback);

  }

  function detailPie (machineID, callback) {

    var mapReducer = MapReducer.defineOn({
      model: "dailyDefact",
      mongoFilters: {
        mach_id: machineID,
        bad_qty: {$gt: 0}
      },
      groupingFunction: function (data) { return data.defact_id },
      queryField: "bad_qty",
      converter: function (data) {
        return {
          name: data._id,
          value: data.value
        }
      }
    });

    mapReducer(callback);

  }

  function detailTable (machineID, callback) {
    var mongoURL = "mongodb://localhost/zhenhai"
    var mongoClient = require('mongodb').MongoClient

    mongoClient.connect(mongoURL, function(err, mongoDB) {
      
      if (err) {
        console.log("Cannot cannto to mongoDB:" + err);
        callback(err, undefined);
        return;
      }

      var resultData = [];

      var collection = mongoDB.collection("dailyDefact");

      collection.find({mach_id: machineID}, function(err, dataSet) {
        dataSet.forEach(function(d) {
          var q = {time: d.timestamp, defact_id: d.defact_id, bad_qty: d.bad_qty};
          if (q.bad_qty > 0) {
            resultData.push(q);
          }
        },function(d) {
          resultData.sort(function(a, b) {
            if (a.time < b.time) { return -1; }
            if (a.time > b.time) { return 1; }
            if (a.time == b.time) { return 0; }
          });

          callback(undefined, resultData);
        });
      });

    });

  }

  return {
    overview: overview,
    detailPie: detailPie,
    detailTable: detailTable
  }

}


exports.jsonAPI = function() {
  return LogMachine.cachedJSON();
}
