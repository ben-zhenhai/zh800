exports.cachedJSON = function() {

  function overview (year, callback) {

    var converter = function(url, title, record) {
      return {
        name: title + " 月",
        value: +(record.count_qty),
        link: url + "/" + title
      }
    }

    CacheQuery.query("/monthly/" + year, converter, callback);
  }

  function yearMonth (year, month, callback) {

    var converter = function(url, title, record) {
      return {
        name: title + " 週",
        value: +(record.count_qty),
        link: url + "/" + title
      }
    }
    CacheQuery.query("/monthly/" + year + "/" + month, converter, callback);

  }

  function yearMonthWeek (year, month, week, callback) {

    var converter = function(url, title, record) {
      return {
        name: title + " 日",
        value: +(record.count_qty),
        link: url + "/" + title
      }
    }

    CacheQuery.query("/monthly/" + year + "/" + month + "/" + week, converter, callback);
  }

  function yearMonthWeekDate (year, month, week, date, callback) {

    var converter = function(url, title, record) {
      return {
        name: title,
        value: +(record.count_qty),
        link: url + "/" + title
      }
    }

    CacheQuery.query("/monthly/" + year + "/" + month + "/" + week + "/" + date, converter, callback);
  }

  function machineDetail (year, month, date, machine, callback) {
    var cacheTableName = year + "-" + month + "-" + date;
    var query = {mach_id: machine}
    CacheQuery.daily(cacheTableName, query, callback);

  }

  return {
    overview: overview,
    yearMonth: yearMonth,
    yearMonthWeek: yearMonthWeek,
    yearMonthWeekDate: yearMonthWeekDate,
    machineDetail: machineDetail
  }

}

exports.jsonAPI = function() {
  return LogMonthly.cachedJSON();
}
