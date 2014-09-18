exports.defineOn = function(options) {

  var model = options.model
  var groupingFunction = options.groupingFunction
  var mongoFilters = options.mongoFilters
  var customFilter = options.customFilter
  var converter = options.converter
  var sorting = options.sorting

  return function(callback) {

    // Schedule to send error if we got timeout, so we 
    // could prevent being blocked if MongoDB is crashed.
    var timeoutProtect = setTimeout(function() {
      timeoutProtect = null;
      callback({error:'MongoDB connection timeout'});
    }, sails.config.timeout.mongoDB);

    model.native(function(err, collection) {
      var mapFunction = function() { emit(groupingFunction(this), this.bad_qty) }
      var reduceFunction = function(key, values) { return Array.sum(values); }
      var mapReduceOptions = {
        out: {inline: 1},
        query: mongoFilters,
        scope: {
          groupingFunction: groupingFunction, 
          mongoFilters: mongoFilters, 
          customFilter: customFilter,
          converter: converter
        }
      }

      var processCallback = function (err, result) {
        if (err) {
          callback(err);
          return;
        }

        if (sorting) {
          result.sort(sorting);
        }

        var resultSet = [];

        for (var i = 0; i < result.length; i++) {
          if (customFilter && customFilter(result[i])) {
            resultSet.push(converter(result[i]));
          } else if (!customFilter) {
            resultSet.push(converter(result[i]));
          }
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

      collection.mapReduce(mapFunction, reduceFunction, mapReduceOptions, processCallback);
    });
  }
}
