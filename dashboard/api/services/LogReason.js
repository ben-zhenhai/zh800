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

  return {
    overview: overview
  }
}
