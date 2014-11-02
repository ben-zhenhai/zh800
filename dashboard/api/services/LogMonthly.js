exports.cachedJSON = function() {

  function overview (year, callback) {
    var startDate = year + "";
    var endDate = (+year+1) + "";

    var sortByDate = function (objA, objB) {
      function strToDate(dateString) {
        var columns = dateString.split("-");
        var year = columns[0];
        var month = +(columns[1]) - 1; // JSDate's month count from 0
        return new Date(+year, +month, 1);
      }

      return strToDate(objA._id).getTime() - strToDate(objB._id).getTime();
    }

    var mapReducer = MapReducer.defineOn({
      model: "daily",
      mongoFilters: {
        timestamp: {$gte: startDate, $lt: endDate}
      },
      groupingFunction: function (data) { 
        return data.timestamp.substring(0, 7);
      },
      converter: function (data) {
        var date = new Date(data._id);
        
        return {
          name: (+date.getMonth()+1) + " 月", 
          value: data.value,
          link: "/monthly/" + date.getFullYear() + "/" + (+date.getMonth()+1)
        }
      }
    });

    mapReducer(callback);
  }

  function yearMonth (year, month, callback) {

    var startDate = year + "-" + PaddingZero.padding(month);
    var endDate = year + "-" + PaddingZero.padding(+month+1);

    var mapReducer = MapReducer.defineOn({
      model: "daily",
      groupingFunction: function (data) { 

        function getWeek(isoDate) {
          var date = isoDate.getDate();
          var day = isoDate.getDay();
          return Math.ceil((date - 1 - day) / 7) + 1;
        }

        return getWeek(new Date(data.timestamp)) 
      },
      mongoFilters: {
        timestamp: {$gte: startDate, $lt: endDate}
      },
      converter: function (data) {
        return {
          name: "第 " + data._id + " 週", 
          value: data.value,
          link: "/monthly/" + year + "/" + month + "/" + data._id
        }
      }
    });

    mapReducer(callback);
  }

  function yearMonthWeek (year, month, week, callback) {
    var startDate = year + "-" + PaddingZero.padding(+month) + "-01";
    var endDate = year + "-" + PaddingZero.padding(+month+1) + "-01";

    var sortByDate = function (objA, objB) {
      if (new Date(objA._id.date) < new Date(objB._id.date)) { return -1; }
      if (new Date(objA._id.date) > new Date(objB._id.date)) { return 1; }
      if (new Date(objA._id.date) == new Date(objB._id.date)) { return 0; }
    }

    var getWeekAndDate = function (data) {

      function getWeek(isoDate) {
        var date = isoDate.getDate();
        var day = isoDate.getDay();
        return Math.ceil((date - 1 - day) / 7) + 1;
      }

      function dateFormatter(isoDate) {
        return isoDate.getFullYear() + "-" + (isoDate.getMonth() + 1) + "-" + isoDate.getDate() ;
      }

      return {
        date: dateFormatter(new Date(data.timestamp)),
        week: getWeek(new Date(data.timestamp)) 
      }
    }

    var mapReducer = MapReducer.defineOn({
      model: "daily",
      groupingFunction: getWeekAndDate,
      mongoFilters: {
        timestamp: {$gte: startDate, $lt: endDate}
      },
      customFilter: function (data) { return data._id["week"] == week; },
      converter: function (data) {
        var dateString = data._id["date"].split("-")[2]
        var currentWeek = data._id["week"]
        var date = new Date(data._id["date"]);

        return {
          name: dateString + " 日",
          value: data.value,
          link: "/monthly/" + year + "/" + month + "/" + currentWeek + "/" + dateString
        };
      }
    });

    mapReducer(callback);
  }

  function yearMonthWeekDate (year, month, week, date, callback) {

    var startDate = year + "-" + PaddingZero.padding(+month) + "-" + PaddingZero.padding(+date);
    var endDate = year + "-" + PaddingZero.padding(+month) + "-" + PaddingZero.padding(+date+1);

    var mapReducer = MapReducer.defineOn({
      model: "daily",
      groupingFunction: function (data) { return data.mach_id; },
      mongoFilters: {
        timestamp: {$gte: startDate, $lt: endDate}
      },
      converter: function (data) {
        return {
          name: data._id,
          value: data.value,
          link: "/monthly/" + year + "/" + month + "/" + week + "/" + date + "/" + data._id
        };
      }
    });

    mapReducer(callback);
  }

  function machineDetail (year, month, date, machine, callback) {
    var cacheTableName = year + "-" + PaddingZero.padding(month) + "-" + PaddingZero.padding(date);
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
