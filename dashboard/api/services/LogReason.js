exports.cachedJSON = function() {
  function overview(callback) {

    var aggeration = [ 
      { $group: { _id: "$defact_id", bad_qty: { $sum: "$bad_qty" } } },
      { $sort: {_id: 1}}
    ]

    var converter = function (data) {
      return {
        name: data._id, 
        value: data.bad_qty,
        link: "/reason/" + data._id
      }
    }

    var aggerator = Aggerator.defineOn({
      model: "monthly",
      mongoURL: "mongodb://localhost/monthly",
      aggeration: aggeration,
      converter: converter
    });

    aggerator(callback);
  }

  function reasonDetail (reasonID, callback) {

    
    var converter = function (url, title, record) {
      return {
        name: title,
        value: record.bad_qty
      }
    }

    CacheQuery.query("/reason/" + reasonID, converter, function(err, dataSet) {

      var tmpData = {};
      var resultData = [];

      for (var i = 0; i < dataSet.length; i++) {
        var data = dataSet[i];
        var machineID = dataSet[i].name.split(" ")[1];
        var value = dataSet[i].value;
        var currentValue = tmpData[machineID] ? tmpData[machineID] : 0;
        tmpData[machineID] = currentValue + value;
      }

      for (var machineID in tmpData) {
        if (tmpData.hasOwnProperty(machineID)) {
          resultData.push({name: machineID, value: tmpData[machineID]});
        }
      }

      callback(undefined, resultData);
    })
  }

  function detailTable (reasonID, callback) {

    
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
