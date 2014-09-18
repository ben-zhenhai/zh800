exports.jsonAPI = function() {

  function overview (callback) {

    var aggeration = [ 
      { $group: { _id: "$order_type", bad_qty: { $sum: "$bad_qty" } } },
      { $sort: {_id: 1}}
    ]

    var converter = function (data) {
      return {
        name: data._id, 
        value: data.bad_qty,
        link: "/total/" + data._id
      }
    }

    var aggerator = Aggerator.defineOn({
      model: Log,
      aggeration: aggeration,
      converter: converter
    });

    aggerator(callback);
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


    var mapReducer = MapReducer.defineOn({
      model: Log,
      mongoFilters: {order_type: product},
      sorting: sortByDate,
      groupingFunction: function (data) { 
        return data.emb_date.getFullYear() + "-" + (data.emb_date.getMonth() + 1) 
      },
      converter: function (data) {
        return {
          name: data._id, 
          value: data.value,
          link: "/total/" + product + "/" + data._id
        }
      }
    });

    mapReducer(callback);
  }

  function productMonth (product, yearAndMonth, callback) {

    var year = yearAndMonth.split("-")[0];
    var month = +(yearAndMonth.split("-")[1]) - 1; // JSDate's month count from 0
    var startDate = new Date(+year, +month, 1);
    var endDate = new Date(+year, (+month)+1, 1);

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
        order_type: product,
        emb_date: {$gte: startDate, $lt: endDate}
      },
      converter: function (data) {
        return {
          name: "第 " + data._id + " 週", 
          value: data.value,
          link: "/total/" + product + "/" + yearAndMonth + "/" + data._id
        }
      }
    });

    mapReducer(callback);
  }

  function productMonthWeek (product, yearAndMonth, week, callback) {

    var year = yearAndMonth.split("-")[0];
    var month = +(yearAndMonth.split("-")[1]) - 1; // JSDate's month count from 0
    var startDate = new Date(+year, +month, 1);
    var endDate = new Date(+year, (+month)+1, 1);

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
        order_type: product,
        emb_date: {$gte: startDate, $lt: endDate}
      },
      customFilter: function (data) { return data._id["week"] == week; },
      sorting: sortByDate,
      converter: function (data) {
        var dateString = data._id["date"].split("-")[2]
        var currentWeek = data._id["week"]

        return {
          name: dateString + " 日",
          value: data.value,
          link: "/total/" + product + "/" + yearAndMonth + "/" + currentWeek + "/" + dateString
        };
      }
    });

    mapReducer(callback);
  }

  function productMonthWeekDate (product, yearAndMonth, week, date, callback) {
    var year = yearAndMonth.split("-")[0];
    var month = +(yearAndMonth.split("-")[1]) - 1; // JSDate's month count from 0
    var minDate = new Date(+year, (+month), 1);
    var maxDate = new Date(+year, (+month)+1, 1);

    var targetStartDate = new Date(year, +month, +date);
    var targetEndDate = new Date(year, +month, (+date)+1);

    var startDate = targetStartDate > minDate ? targetStartDate : minDate;
    var endDate = targetEndDate < maxDate ? targetEndDate : maxDate;

    var mapReducer = MapReducer.defineOn({
      model: Log,
      groupingFunction: function (data) { return data.mach_id; },
      mongoFilters: {
        order_type: product,
        emb_date: {$gte: startDate, $lt: endDate}
      },
      converter: function (data) {
        return {
          name: data._id,
          value: data.value,
          link: "/total/" + product + "/" + yearAndMonth + "/" + week + "/" + date + "/" + data._id
        };
      }
    });

    mapReducer(callback);
  }

  function machineDetail (product, yearAndMonth, date, machine, callback) {

    var year = yearAndMonth.split("-")[0];
    var month = +(yearAndMonth.split("-")[1]) - 1; // JSDate's month count from 0
    var startDate = new Date(+year, +month, +date);
    var endDate = new Date(+year, +month, (+date) + 1);

    var mapReducer = MapReducer.defineOn({
      model: Log,
      groupingFunction: function(data) {
        return {date: data.emb_date, error: data.defact_id};
      },
      mongoFilters: {
        order_type: product,
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

  function getDateRange(callback) {

    var converter = function (data) { return {minDate: data.min, maxDate: data.max} }
    var aggeration = {
      $group: {
        _id: "$log", 
        min: {$min: "$emb_date"}, 
        max: {$max: "$emb_date"}
      }
    }

    function checkResult(err, result) {
      if (result && result.length == 1) {
        callback(err, result[0].minDate, result[0].maxDate);
      } else {
        callback({error: "Cannot get correct date range from MongoDB"});
      }
    }

    var aggerator = Aggerator.defineOn({
      model: Log,
      aggeration: aggeration,
      converter: converter
    });

    aggerator(checkResult);
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
