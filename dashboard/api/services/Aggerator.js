exports.defineOn = function(options) {

  var model = options.model
  var aggeration = options.aggeration
  var converter = options.converter

  return function(callback) {
    model.native(function(err, collection) {
      if (err) {
        callback(err)
        return;
      }

      var onGetResultSet = function (err, result) {

        if (err) {
          callback(err);
          return;
        }

        var resultSet = [];

        for (var i = 0; i < result.length; i++) {
          resultSet.push(converter(result[i]));
        }
        
        if (callback) {
          callback(err, resultSet);
        } 
      }

      collection.aggregate(aggeration, onGetResultSet);
    });
  }
}
