/**
* Log.js
*
* @description :: TODO: You might write a short summary of how this model works and what it represents here.
* @docs        :: http://sailsjs.org/#!documentation/models
*/

module.exports = {

  schema: true,
  attributes: {
    order_type: { type: "string", required: true, unique: true },
    lot_no: { "type": "string", required: true, unique: true },
    work_qty:  { "type": "integer", required: true },
    count_qty: { "type": "integer", required: true },
    emb_date:  { "type": "date", required: true },
    bad_qty:   { "type": "integer", required: true },
    mach_ip:   { "type": "string", required: true },
    defact_id: { "type": "integer", required: true },
    mach_id:   { "type": "string", required: true },
    work_id:   { "type": "integer", required: true },
    cx:        { "type": "string", required: true },
    dx:        { "type": "string", required: true },
    lc:        { "type": "string", required: true },
    mach_status: { "type": "string", required: true }
  },

  totalMachine: function (product, yearAndMonth, date, machine, callback) {

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
  },

  totalProductMonthWeekDate: function(product, yearAndMonth, week, date, callback) {
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
  },

  totalProductMonthWeek: function(product, yearAndMonth, week, callback) {

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
  },

  totalProductMonth: function(product, yearAndMonth, callback) {

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
  },

  totalProduct: function(product, callback) {

    var mapReducer = MapReducer.defineOn({
      model: Log,
      mongoFilters: {order_type: product},
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
  },

  overviewByOrderType: function(res, callback) {
    Log.native(function(err, logCollection) {

      if (err) { return res.serverError(err); }

      var aggerateMethod = [ 
        { $group: { _id: "$order_type", bad_qty: { $sum: "$bad_qty" } } },
        { $sort: {_id: 1}}
      ]

      var onGetResultSet = function (err, result) {

        if (err) { return res.serverError(err); }

        var resultSet = [];

        for (var i = 0; i < result.length; i++) {
          resultSet[i] = {
            name: result[i]._id, 
            value: result[i].bad_qty,
            link: "/total/" + result[i]._id
          }
        }

        if (callback) {
          callback(resultSet);
        }
      }

      logCollection.aggregate(aggerateMethod, onGetResultSet);
    });
  }
};

