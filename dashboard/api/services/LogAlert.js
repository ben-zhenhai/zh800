exports.cachedJSON = function () {

  function overview (callback) {
    Alert.native(function(err, collection) {
      if (err) {
        callback(err);
        return;
      }
      collection.find({}, function(err, cursor) {
        if (err) {
          callback(err);
          return;
        }

        cursor.sort({timestamp: 1, mach_id: 1}).toArray(function(err, resultData) {
          callback(undefined, resultData);
        });

      })
    });
  }

  function hasData(callback) {
    Alert.native(function(err, collection) {
      if (err) {
        callback(err);
        return;
      }

      collection.find({}, function(err, cursor) {

        if (err) {
          callback(err);
          return;
        }

        cursor.next(function(err, data) {
          if (data) {
            callback(true);
          } else {
            callback(false);
          }
        });

      })
    });

  }

  return {
    overview: overview,
    hasData: hasData
  }

}


exports.jsonAPI = function() {
  return LogAlert.cachedJSON();
}
