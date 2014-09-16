exports.defineOn = function(options) {

  console.log("In MapReducer....");

  var model = options.model
  var groupingFunction = options.groupingFunction
  var mongoFilters = options.mongoFilters
  var customFilter = options.customFilter
  var converter = options.converter
  var sorting = options.sorting

  return function(callback) {
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

        callback(err, resultSet);
      }

      collection.mapReduce(mapFunction, reduceFunction, mapReduceOptions, processCallback);
    });
  }
}
