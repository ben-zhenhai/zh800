exports.cachedJSON = function() {
  function overview(callback) {

    var converter = function (url, title, record) {
      return {
        name: title,
        value: record.bad_qty,
        link: "/reason/" + title
      }
    }

    CacheQuery.query("/reason", converter, function(err, dataSet) {

      if (err) {
        callback(err, undefined);
        return;
      }

      var resultData = [];

      callback(err, dataSet);
    });
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

    var converter = function (url, title, record) {
      var machineID = title.split(" ")[1];
      var date = title.split(" ")[0];
      return {
        name: machineID,
        time: date,
        value: record.bad_qty
      }
    }

    CacheQuery.query("/reason/" + reasonID, converter, function(err, dataSet) {

      if (err) {
        callback(err, undefined);
        return;
      }

      dataSet.sort(function(objA, objB) {
        if (objA.time < objB.time) { return -1; }
        if (objA.time > objB.time) { return 1; }
        if (objA.time == objB.time) { return 1; }
      });

      callback(undefined, dataSet);

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
