exports.jsonAPI = function() {

  function overview(callback) {

     Log.native(function(err, logCollection) {

      if (err) { 
        callback(err) 
        return;
      }

      var aggerateMethod = [ 
        { $group: { _id: "$defact_id", bad_qty: { $sum: "$bad_qty" } } },
        { $sort: {_id: 1}}
      ]

      var onGetResultSet = function (err, result) {

        if (err) { 
          callback(err); 
          return;
        }

        var resultSet = [];

        for (var i = 0; i < result.length; i++) {
          resultSet[i] = {
            name: result[i]._id, 
            value: result[i].bad_qty,
            link: "/reason/" + result[i]._id
          }
        }

        if (callback) {
          callback(err, resultSet);
        }
      }

      logCollection.aggregate(aggerateMethod, onGetResultSet);
    });
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
