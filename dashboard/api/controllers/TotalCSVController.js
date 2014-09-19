/**
 * DashboardController
 *
 * @description :: Server-side logic for managing users
 * @help        :: See http://links.sailsjs.org/docs/controllers
 */

module.exports = {

  overview: function(req, res) {

    var logTotal = LogTotal.jsonAPI();

    logTotal.overview(function(err, result) {


      if (err) {
        res.serverError(err);
        return;
      }

      var lines = '"Φ 別","數量"\n'

      res.type('text/csv');
      for (var i = 0; i < result.length; i++) {
        var record = result[i];
        lines += '"' + record.name + '",' + record.value + "\n";
      }

      res.end(lines);
    });
  },

  product: function(req, res) {

    var product = req.param("product");
    var logTotal = LogTotal.jsonAPI();

    logTotal.product(product, function(err, data) {

      if (err) {
        res.serverError(err);
        return;
      }

      var lines = '"月份","數量"\n'

      res.type('text/csv');

      for (var i = 0; i < data.length; i++) {
        var record = data[i];
        lines += '"' + record.name + '",' + record.value + "\n";
      }

      res.end(lines);

    });
  },

  productMonth: function(req, res) {
    var product = req.param("product");
    var month = req.param("month")

    var logTotal = LogTotal.jsonAPI();

    logTotal.productMonth(product, month, function(err, data) {

      if (err) {
        res.serverError(err);
        return;
      }

      var lines = '"週","數量"\n'

      res.type('text/csv');

      for (var i = 0; i < data.length; i++) {
        var record = data[i];
        lines += '"' + record.name + '",' + record.value + "\n";
      }

      res.end(lines);
    });
  },

  productMonthWeek: function(req, res) {
    var product = req.param("product");
    var month = req.param("month");
    var week = req.param("week");

    var logTotal = LogTotal.jsonAPI();

    logTotal.productMonthWeek(product, month, week, function(err, data) {

      if (err) {
        res.serverError(err);
        return;
      }

      var lines = '"日","數量"\n'

      res.type('text/csv');

      for (var i = 0; i < data.length; i++) {
        var record = data[i];
        lines += '"' + record.name + '",' + record.value + "\n";
      }

      res.end(lines);

    });
  },

  productMonthWeekDate: function(req, res) {
    var product = req.param("product");
    var month = req.param("month");
    var week = req.param("week");
    var date = req.param("date");

    var logTotal = LogTotal.jsonAPI();

    logTotal.productMonthWeekDate(product, month, week, date, function(err, data) {

      if (err) {
        res.serverError(err);
        return;
      }

      var lines = '"機器","數量"\n'

      res.type('text/csv');

      for (var i = 0; i < data.length; i++) {
        var record = data[i];
        lines += '"' + record.name + '",' + record.value + "\n";
      }

      res.end(lines);

    });
  },

  machineDetail: function(req, res) {
    var product = req.param("product");
    var month = req.param("month");
    var week = req.param("week");
    var date = req.param("date");
    var machine = req.param("machine");

    var logTotal = LogTotal.jsonAPI();

    logTotal.machineDetail(product, month, date, machine, function(err, data) {

      if (err) {
        res.serverError(err);
        return;
      }

      var lines = '"日期","錯誤種類","數量"\n'

      res.type('text/csv');

      for (var i = 0; i < data.length; i++) {
        var record = data[i];
        var time = 
          record.name.date.getFullYear() + "-" + (+record.name.date.getMonth() + 1) + "-" +
          record.name.date.getDate() + " " + record.name.date.getHours() + ":" +
          record.name.date.getMinutes() + ":" + record.name.date.getSeconds();

        lines += '"' + time + '","' + record.name.error + '",' + record.value + "\n";
      }

      res.end(lines);
    });
  }
};

