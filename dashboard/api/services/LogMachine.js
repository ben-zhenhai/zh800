exports.cachedJSON = function() {
  function overview(callback) {

    var mapReducer = MapReducer.defineOn({
      model: "reasonByMachine",
      queryField: "bad_qty",
      groupingFunction: function (data) { return data.mach_type },
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

  function step(step, callback) {

    var mapReducer = MapReducer.defineOn({
      model: "reasonByMachine",
      queryField: "bad_qty",
      groupingFunction: function (data) { return data.mach_model },
      mongoFilters: {
        mach_type: step,
        bad_qty: {$gt: 0}
      },
      converter: function (data) {
        return {
          name: data._id,
          value: data.value,
          link: "/machine/" + step + "/" + data._id
        }
      }
    });

    mapReducer(callback);
  }

  function stepModel(step, model, callback) {

    var mapReducer = MapReducer.defineOn({
      model: "reasonByMachine",
      queryField: "bad_qty",
      groupingFunction: function (data) { return data.mach_id },
      mongoFilters: {
        mach_type: step,
        mach_model: model,
        bad_qty: {$gt: 0}
      },
      converter: function (data) {
        return {
          name: data._id,
          value: data.value,
          link: "/machine/" + step + "/" + model + "/" + data._id
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

      var collection = mongoDB.collection("dailyDefact");
      
      var resultData = [];
      collection.find({mach_id: machineID}, function(err, dataSet) {
        dataSet.forEach(function(d) {
          if (d.bad_qty > 0) {
            resultData.push({time: d.timestamp, defact_id: d.defact_id, bad_qty: d.bad_qty});
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
    step: step,
    stepModel: stepModel,
    detailPie: detailPie,
    detailTable: detailTable
  }

}

exports.jsonAPI = function() {
  return LogMachine.cachedJSON();
}
