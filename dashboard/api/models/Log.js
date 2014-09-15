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

  totalProductMonth: function(product, yearAndMonth, callback) {
    var year = yearAndMonth.split("-")[0];
    var month = +(yearAndMonth.split("-")[1]) - 1; // JSDate's month count from 0
    var startDate = new Date(year, month, 1);
    var endDate = new Date(year, month+1, 1);
    Log.native(function(err, logCollection) {

      var mapFunction = function() {

        function dateFormatter(isoDate) {
          var date = isoDate.getDate();
          var day = isoDate.getDay();
          return Math.ceil((date - 1 - day) / 7) + 1;
        }

        emit(dateFormatter(this.emb_date), this.bad_qty)
      }

      var reduceFunction = function (key, values) { return Array.sum(values); }

      var outputControl = {
        out: {inline: 1},
        query: {
          order_type: product,
          emb_date: {$gte: startDate, $lt: endDate}
        }
      }

      logCollection.mapReduce(mapFunction, reduceFunction, outputControl, function (err, result) {

        if (err) { 
          callback(err); 
          return; 
        }

        var resultSet = [];

        for (var i = 0; i < result.length; i++) {
          resultSet[i] = {
            name: "第 " + result[i]._id + " 週", 
            value: result[i].value,
            link: "/total/" + product + "/" + yearAndMonth + "/" + result[i]._id
          }
        }

        console.log(resultSet);
        callback(err, resultSet);
      });

    });
  },

  totalProduct: function(product, callback) {
    Log.native(function(err, logCollection) {

      var mapFunction = function() {
        function dateFormatter(date) {
          return date.getFullYear() + "-" + (date.getMonth() + 1)
        }

        emit(dateFormatter(this.emb_date), this.bad_qty)
      }

      var reduceFunction = function (key, values) {
        return Array.sum(values);
      }

      var outputControl = {
        out: {inline: 1},
        query: {order_type: product}
      }

      logCollection.mapReduce(mapFunction, reduceFunction, outputControl, function (err, result) {
        if (err) { 
          callback(err); 
          return; 
        }

        var resultSet = [];

        for (var i = 0; i < result.length; i++) {
          resultSet[i] = {
            name: result[i]._id, 
            value: result[i].value,
            link: "/total/" + product + "/" + result[i]._id
          }
        }

        console.log(resultSet);
        callback(err, resultSet);
      });
    });
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

