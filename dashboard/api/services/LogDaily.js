exports.cachedJSON = function() {
  function overview (year, month, callback) {
    var startDate = year + "-" + PaddingZero.padding(+month)
    var endDate = year + "-" + PaddingZero.padding(+month+1)

    var mapReducer = MapReducer.defineOn({
      model: "daily",
      mongoFilters: {
        timestamp: {$gte: startDate, $lt: endDate}
      },
      groupingFunction: function (data) { 
        return new Date(data.timestamp).getDate();
      },
      converter: function (data) {
        return {
          name: data._id + " 日", 
          value: data.value,
          link: "/daily/" + year + "/" + month + "/" + data._id
        }
      }
    });

    mapReducer(callback);
  }

  function yearMonthDate (year, month, date, callback) {

    var startDate = year + "-" + PaddingZero.padding(+month) + "-" + PaddingZero.padding(+date);
    var endDate = year + "-" + PaddingZero.padding(+month) + "-" + PaddingZero.padding(+date+1);

    var mapReducer = MapReducer.defineOn({
      model: "daily",
      mongoFilters: {
        timestamp: {$gte: startDate, $lt: endDate}
      },
      groupingFunction: function (data) { 
        return data.mach_id;
      },
      converter: function (data) {
        return {
          name: data._id,  
          value: data.value,
          link: "/daily/" + year + "/" + month + "/" + date + "/" + data._id
        }
      }
    });

    mapReducer(callback);
  }

  function machineDetail (year, month, date, machine, callback) {
    var cacheTableName = year + "-" + PaddingZero.padding(+month) + "-" + PaddingZero.padding(+date);
    var query = {mach_id: machine}
    CacheQuery.daily(cacheTableName, query, callback);
  }

  return {
    overview: overview,
    yearMonthDate: yearMonthDate,
    machineDetail: machineDetail
  }

}


exports.jsonAPI = function() {
  return LogDaily.cachedJSON();
}
