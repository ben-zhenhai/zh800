/**
 *  This define a generic wrapper function for mongoDB's aggerator function,
 *  it will issue a timeout error to Sails.js HTTP server if the mongoDB server
 *  crashed.
 *
 *  The `options` arguement should have the following attributes:
 *
 *    - model: Which Sails.js ORM model should this aggerator operates.
 *    - aggeration: The mongoDB aggeration setting array that will passed to mongoDB
 *    - conveter: To convert mongoDB's response object to custom JS object.
 *
 *  After calling this function, it will return a function that accepts a callback of
 *   `function callback(err, data)`, which this callback will handle the response after
 *  MongoDB returned the data or error has occured.
 *
 */
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
