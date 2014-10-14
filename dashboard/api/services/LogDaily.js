exports.realtimeJSON = function() {
  function overview (year, month, callback) {

    var startDate = new Date(year, (+month)-1, 1);
    var endDate = new Date(year, +(month), 1);

    var mapReducer = MapReducer.defineOn({
      model: Log,
      mongoFilters: {
        emb_date: {$gte: startDate, $lt: endDate}
      },
      groupingFunction: function (data) { 
        return data.emb_date.getDate();
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

    var startDate = new Date(year, (+month)-1, date);
    var endDate = new Date(year, +(month)-1, date+1);

    var mapReducer = MapReducer.defineOn({
      model: Log,
      mongoFilters: {
        emb_date: {$gte: startDate, $lt: endDate}
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

    var startDate = new Date(+year, +(month-1), +date);
    var endDate = new Date(+year, +(month-1), (+date) + 1);

    var mapReducer = MapReducer.defineOn({
      model: Log,
      groupingFunction: function(data) {
        return {date: data.emb_date, error: data.defact_id};
      },
      mongoFilters: {
        mach_id: machine,
        emb_date: {$gte: startDate, $lt: endDate}
      },
      converter: function (data) {
        return {
          name: data._id,
          value: data.value,
        };
      }
    });

    mapReducer(callback);
  }

  return {
    overview: overview,
    yearMonthDate: yearMonthDate,
    machineDetail: machineDetail
  }

}

exports.cachedJSON = function() {
  function overview (year, month, callback) {

    var converter = function(url, title, record) {
      return {
        name: title + " 日",
        value: +(record.count_qty),
        link: url + "/" + title
      }
    }

    CacheQuery.query("/daily/" + year + "/" + month, converter, callback);
  }

  function yearMonthDate (year, month, date, callback) {

    var converter = function(url, title, record) {
      return {
        name: title,
        value: +(record.count_qty),
        link: url + "/" + title
      }
    }

    CacheQuery.query("/daily/" + year + "/" + month + "/" + date, converter, callback);

  }

  function machineDetail (year, month, date, machine, callback) {
    var cacheTableName = year + "-" + month + "-" + date;
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
  switch (sails.config.models.fetch) {
    case "realtime": return LogDaily.realtimeJSON();
    case "cached": return LogDaily.cachedJSON();
  }
}
