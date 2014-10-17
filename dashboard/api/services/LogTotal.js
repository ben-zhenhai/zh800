exports.cachedJSON = function () {

  function overview (callback) {
    Product.native(function(err, collection) {
      if (err) {
        callback(err);
        return;
      }

      var resultData = [];

      collection.find({}).sort({product: 1}).toArray(function(err, records) {
        if (err) {
          callback(err);
          return;
        }

        for (var i = 0; i < records.length; i++) {
          resultData.push({
            name: records[i].product,
            value: +(records[i].count_qty),
            link: "/total/" + records[i].product
          });
        }
        callback(undefined, resultData);
      })
    });
  }

  function product (product, callback) {

    var sortByMonth = function (objA, objB) {
      if (objA._id < objB._id) { return -1; }
      if (objA._id > objB._id) { return 1; }
      if (objA._id == objB._id) { return 0; }
    }

    var mapReducer = MapReducer.defineOn({
      model: "product-" + product,
      groupingFunction: function (data) { 
        return data.timestamp.substring(0, 7);
      },
      sorting: sortByMonth,
      converter: function (data) {
        var date = new Date(data._id + "-01");
        return {
          name: data._id, 
          value: data.value,
          link: "/total/" + product + "/" + date.getFullYear() + "/" + (+date.getMonth() + 1)
        }
      }
    });

    mapReducer(callback);
  }

  function productMonth (product, year, month, callback) {

    var startDate = year + "-" + PaddingZero.padding(month);
    var endDate = year + "-" + PaddingZero.padding(+month+1);

    var mapReducer = MapReducer.defineOn({
      model: "product-" + product,
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
          link: "/total/" + product + "/" + year + "/" + month + "/" + data._id
        }
      }
    });

    mapReducer(callback);
  }

  function productMonthWeek (product, year, month, week, callback) {

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
      model: "product-" + product,
      groupingFunction: getWeekAndDate,
      mongoFilters: {
        timestamp: {$gte: startDate, $lt: endDate}
      },
      customFilter: function (data) { return data._id["week"] == week; },
      sorting: sortByDate,
      converter: function (data) {
        var dateString = data._id["date"].split("-")[2]
        var currentWeek = data._id["week"]

        return {
          name: dateString + " 日",
          value: data.value,
          link: "/total/" + product + "/" + year + "/" + month + "/" + currentWeek + "/" + dateString
        };
      }
    });

    mapReducer(callback);

  }

  function productMonthWeekDate (product, year, month, week, date, callback) {

    var startDate = year + "-" + PaddingZero.padding(+month) + "-" + PaddingZero.padding(+date);
    var endDate = year + "-" + PaddingZero.padding(+month) + "-" + PaddingZero.padding(+date+1);

    var mapReducer = MapReducer.defineOn({
      model: "product-" + product,
      groupingFunction: function (data) { return data.mach_id; },
      mongoFilters: {
        timestamp: {$gte: startDate, $lt: endDate}
      },
      converter: function (data) {
        return {
          name: data._id,
          value: data.value,
          link: "/total/" + product + "/" + year + "/" + month + "/" + week + "/" + date + "/" + data._id
        };
      }
    });

    mapReducer(callback);
  }


  function machineDetail (product, year, month, week, date, machine, callback) {
    var cacheTableName = year + "-" + PaddingZero.padding(month) + "-" + PaddingZero.padding(date);
    var query = {product: product, mach_id: machine}
    CacheQuery.daily(cacheTableName, query, callback);
  }

  function getDateRange(callback) {

    var maxTime = new Date();
    var minTime = new Date();



    Daily.native(function(err, collection) {
      if (err) {
        console.log("database error:" + err);
        callback(err);
        return;
      }

      var query = {$group: { _id: "", minTime: {$min: "$timestamp"}, maxTime: {$max: "$timestamp"}}}
      collection.aggregate(query, function(err, data) {
        if (err) {
          console.log("database error:" + err);
          callback(err);
          return;
        }
        console.log(data[0]);
        var maxTime = new Date();
        var minTime = new Date();

        if (data.length == 1 && data[0].minTime) {
          minTime = new Date(data[0].minTime);
        }

        if (data.length == 1 && data[0].maxTime) {
          maxTime = new Date(data[0].maxTime)
        }

        callback(undefined, minTime, maxTime);
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
