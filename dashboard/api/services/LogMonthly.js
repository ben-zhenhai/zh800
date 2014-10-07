exports.realtimeJSON = function() {
  function overview (year, callback) {

    var startDate = new Date(year, 0, 1);
    var endDate = new Date(year, 12, 1);

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
      model: Log,
      sorting: sortByDate,
      mongoFilters: {
        emb_date: {$gte: startDate, $lt: endDate}
      },
      groupingFunction: function (data) { 
        return data.emb_date.getFullYear() + "-" + (data.emb_date.getMonth() + 1) 
      },
      converter: function (data) {
        var month = data._id.split("-")[1];
        return {
          name: month + " 月", 
          value: data.value,
          link: "/monthly/" + year + "/" + month
        }
      }
    });

    mapReducer(callback);
  }

  function yearMonth (year, month, callback) {

    var startDate = new Date(year, (+month)-1, 1);
    var endDate = new Date(year, +month, 1);

    var mapReducer = MapReducer.defineOn({
      model: Log,
      groupingFunction: function (data) { 

        function getWeek(isoDate) {
          var date = isoDate.getDate();
          var day = isoDate.getDay();
          return Math.ceil((date - 1 - day) / 7) + 1;
        }

        return getWeek(data.emb_date) 
      },
      mongoFilters: {
        emb_date: {$gte: startDate, $lt: endDate}
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

    var startDate = new Date(+year, (+month)-1, 1);
    var endDate = new Date(+year, (+month), 1);

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
        date: dateFormatter(data.emb_date),
        week: getWeek(data.emb_date) 
      }
    }

    var sortByDate = function (objA, objB) {
      function strToDate(dateString) {
        var columns = dateString.split("-");
        var year = columns[0];
        var month = +(columns[1]) - 1; // JSDate's month count from 0
        var date = columns[2];
        return new Date(+year, +month, +date);
      }

      return strToDate(objA._id.date).getTime() - strToDate(objB._id.date).getTime();
    }

    var mapReducer = MapReducer.defineOn({
      model: Log,
      groupingFunction: getWeekAndDate,
      mongoFilters: {
        emb_date: {$gte: startDate, $lt: endDate}
      },
      customFilter: function (data) { return data._id["week"] == week; },
      converter: function (data) {
        var dateString = data._id["date"].split("-")[2]
        var currentWeek = data._id["week"]

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

    var minDate = new Date(+year, (+month)-1, 1);
    var maxDate = new Date(+year, (+month), 1);

    var targetStartDate = new Date(year, (+month)-1, +date);
    var targetEndDate = new Date(year, +month, (+date)+1);

    var startDate = targetStartDate > minDate ? targetStartDate : minDate;
    var endDate = targetEndDate < maxDate ? targetEndDate : maxDate;

    var mapReducer = MapReducer.defineOn({
      model: Log,
      groupingFunction: function (data) { return data.mach_id; },
      mongoFilters: {
        emb_date: {$gte: startDate, $lt: endDate}
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
    yearMonth: yearMonth,
    yearMonthWeek: yearMonthWeek,
    yearMonthWeekDate: yearMonthWeekDate,
    machineDetail: machineDetail
  }

}

exports.jsonAPI = function() {
  switch (sails.config.models.fetch) {
    case "realtime": return LogMonthly.realtimeJSON();
    case "cached": return LogMonthly.realtimeJSON();
  }
}
