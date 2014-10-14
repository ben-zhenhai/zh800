exports.realtimeJSON = function() {
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
      model: Log,
      aggeration: aggeration,
      converter: converter
    });

    aggerator(callback);
  }

  function reasonDetail (reasonID, callback) {

    var mapReducer = MapReducer.defineOn({
      model: Log,
      groupingFunction: function(data) { return data.mach_id; },
      mongoFilters: {defact_id: +reasonID},
      converter: function (data) {
        return {
          name: data._id,
          value: data.value
        };
      }
    });

    mapReducer(callback);
  }

  function detailTable (reasonID, callback) {

    var sortByDate = function (objA, objB) {
      function strToDate(dateString) {
        var columns = dateString.split("-");
        var year = columns[0];
        var month = +(columns[1]) - 1; // JSDate's month count from 0
        return new Date(+year, +month, 1);
      }

      return strToDate(objA._id.time).getTime() - strToDate(objB._id.time).getTime();
    }

    var mapReducer = MapReducer.defineOn({
      model: Log,
      sorting: sortByDate,
      groupingFunction: function(data) { 
        var month = +(data.emb_date.getMonth()) + 1
        var dateString = 
          data.emb_date.getFullYear() + "-" + month

        return {name: data.mach_id, time: dateString}; 
      },
      mongoFilters: {defact_id: +reasonID},
      converter: function (data) {
        return {
          name: data._id.name,
          time: data._id.time,
          value: data.value
        };
      }
    });

    mapReducer(callback);
  }

  return {
    overview: overview,
    reasonDetail: reasonDetail,
    detailTable: detailTable
  }

}

exports.cachedJSON = function() {
  function overview(callback) {

    var converter = function (url, title, record) {
      return {
        name: title,
        value: record.bad_qty,
        link: "/reason/" + title
      }
    }

    CacheQuery.query("/reason", converter, callback);
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
        var defactID = dataSet[i].name.split(" ")[1];
        var value = dataSet[i].value;
        var currentValue = tmpData[defactID] ? tmpData[defactID] : 0;
        tmpData[defactID] = currentValue + value;
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

    CacheQuery.query("/reason/" + reasonID, converter, callback);
  }

  return {
    overview: overview,
    reasonDetail: reasonDetail,
    detailTable: detailTable
  }

}

exports.jsonAPI = function() {
  switch (sails.config.models.fetch) {
    case "realtime": return LogReason.realtimeJSON();
    case "cached": return LogReason.cachedJSON();
  }
}
