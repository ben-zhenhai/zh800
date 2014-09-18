exports.defineOn = function(options) {

  var model = options.model
  var aggeration = options.aggeration
  var converter = options.converter

  return function(callback) {

    // Schedule to send error if we got timeout, so we 
    // could prevent being blocked if MongoDB is crashed.
    var timeoutProtect = setTimeout(function() {
      timeoutProtect = null;
      callback({error:'MongoDB connection timeout'});
    }, sails.config.timeout.mongoDB);

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

        // If timeoutPreotect has triggered, it will set itself to
        // null, so our real callback will not executed.
        if (timeoutProtect) {

          // We have to clear scheduled timeoutProetect to avoid it
          // being called if we have result before timeout.
          clearTimeout(timeoutProtect);

          if (callback) {
            callback(err, resultSet);
          } 
        }
        
      }

      collection.aggregate(aggeration, onGetResultSet);
    });
  }
}
