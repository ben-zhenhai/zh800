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
  return LogDaily.cachedJSON();
}
