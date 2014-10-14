exports.cachedJSON = function() {

  function overview(convert, callback) {

    var converter = function (url, title, record) {
      return {
        name: title,
        value: record.bad_qty,
        link: "/machine/" + title
      }
    }

    CacheQuery.query("/machine", converter, callback);
  }

  function detailPie (machineID, callback) {

    var converter = function (url, title, record) {
      return {
        name: title,
        value: record.bad_qty
      }
    }

    CacheQuery.query("/machine/" + machineID, converter, function(err, dataSet) {

      var tmpData = {};
      var resultData = [];

      for (var i = 0; i < dataSet.length; i++) {
        var data = dataSet[i];
        var defactID = dataSet[i].name.split(" ")[1];
        var value = dataSet[i].value;
        var currentValue = tmpData[defactID] ? tmpData[defactID] : 0;
        tmpData[defactID] = currentValue + value;
      }

      for (var defactID in tmpData) {
        if (tmpData.hasOwnProperty(defactID)) {
          resultData.push({name: defactID, value: tmpData[defactID]});
        }
      }

      callback(undefined, resultData);
    })

  }

  function detailTable (machineID, callback) {

    var converter = function (url, title, record) {
      var defactID = title.split(" ")[1];
      var date = title.split(" ")[0];
      return {
        name: defactID,
        time: date,
        value: record.bad_qty
      }
    }

    CacheQuery.query("/machine/" + machineID, converter, callback);

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
