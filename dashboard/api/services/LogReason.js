exports.cachedJSON = function() {
  function overview(callback) {

    var mapReducer = MapReducer.defineOn({
      model: "dailyDefact",
      queryField: "bad_qty",
      groupingFunction: function (data) { return data.mach_id + "__" + data.defact_id; },
      mongoFilters: {
        bad_qty: {$gt: 0}
      },
      converter: function (data) {
        return {
          name: data._id,
          value: data.value,
          link: "/reason/" + data._id
        }
      }
    });

    mapReducer(callback);

  }

  function reasonDetail (reasonID, callback) {
    var defactID = reasonID.split("__")[1] + "";
    var machineID = reasonID.split("__")[0] + "";

    var mapReducer = MapReducer.defineOn({
      model: "dailyDefact",
      mongoFilters: {
        defact_id: +defactID,
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

  function detailTable (reasonID, callback) {
    var mongoURL = "mongodb://localhost/zhenhai"
    var mongoClient = require('mongodb').MongoClient

    mongoClient.connect(mongoURL, function(err, mongoDB) {
      
      if (err) {
        console.log("Cannot cannto to mongoDB:" + err);
        callback(err, undefined);
        return;
      }

      var collection = mongoDB.collection("dailyDefact");
      var defactID = reasonID.split("__")[1] + "";
      var machineID = reasonID.split("__")[0] + "";
      
      var resultData = [];
      collection.find({defact_id: +defactID, mach_id: machineID}, function(err, dataSet) {
        dataSet.forEach(function(d) {
          if (d.bad_qty > 0) {
            resultData.push({time: d.timestamp, name: d.mach_id, value: d.bad_qty});
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
    reasonDetail: reasonDetail,
    detailTable: detailTable
  }

}

exports.jsonAPI = function() {
  return LogReason.cachedJSON();
}
