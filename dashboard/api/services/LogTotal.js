exports.cachedJSON = function () {

  function overview (callback) {
    var converter = function(url, title, record) {
      return {
        name: title,
        value: +(record.count_qty),
        link: url + "/" + title
      }
    }

    CacheQuery.query("/total", converter, callback);
  }

  function product (product, callback) {

    var sortByDate = function (objA, objB) {
      function strToDate(dateString) {
        var columns = dateString.split("-");
        var year = columns[0];
        var month = +(columns[1]) - 1; // JSDate's month count from 0
        return new Date(+year, +month, 1);
      }

      return strToDate(objA._id).getTime() - strToDate(objB._id).getTime();
    }

    var converter = function(url, title, record) {
      return {
        name: title,
        value: +(record.count_qty),
        link: url + "/" + title.replace("-", "/")
      }
    }

    CacheQuery.query("/total/" + product, converter, callback);
  }

  function productMonth (product, year, month, callback) {
    var yearMonth = year + "-" + month;

    var converter = function(url, title, record) {
      return {
        name: '第 ' + title + ' 週',
        value: +(record.count_qty),
        link: url.replace("-", "/") + "/" + title
      }
    }

    CacheQuery.query("/total/" + product + "/" + yearMonth, converter, callback);
  }

  function productMonthWeek (product, year, month, week, callback) {

    var yearMonth = year + "-" + month;

    var converter = function(url, title, record) {
      return {
        name: title + ' 號',
        value: +(record.count_qty),
        link: url.replace("-", "/") + "/" + title
      }
    }

    CacheQuery.query("/total/" + product + "/" + yearMonth + "/" + week, converter, callback);
  }

  function productMonthWeekDate (product, year, month, week, date, callback) {

    var yearMonth = year + "-" + month;

    var converter = function(url, title, record) {
      return {
        name: title,
        value: +(record.count_qty),
        link: url.replace("-", "/") + "/" + title
      }
    }

    CacheQuery.query("/total/" + product + "/" + yearMonth + "/" + week + "/" + date, converter, callback);
  }


  function machineDetail (product, year, month, week, date, machine, callback) {
    var cacheTableName = year + "-" + month + "-" + date;
    var query = {lot_no: product, mach_id: machine}
    CacheQuery.daily(cacheTableName, query, callback);
  }

  function getDateRange(callback) {

    var maxTime = new Date();
    var minTime = new Date();

    Cached.native(function(err, collection) {
      if (err) {
        console.log("database error:" + err);
        return;
      }

      collection.findOne({url: "maxTime"}, function(err, maxTimeInDB) {

        collection.findOne({url: "minTime"}, function(err, minTimeInDB) {
          
            if (maxTimeInDB) {
              maxTime = new Date((+maxTimeInDB.value) * 1000);
            }

            if (minTimeInDB) {
              minTime = new Date((+minTimeInDB.value) * 1000);
            }
            callback(undefined, minTime, maxTime);
        });
      });
    });
  }

  return {
    overview: overview,
    product: product,
    productMonth: productMonth,
    productMonthWeek: productMonthWeek,
    productMonthWeekDate: productMonthWeekDate,
    machineDetail: machineDetail,
    getDateRange: getDateRange
  }

}


exports.jsonAPI = function() {
  return LogTotal.cachedJSON();
}
