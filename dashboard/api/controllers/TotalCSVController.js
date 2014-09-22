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


      res.type('text/csv');

      res.write(new Buffer('"Φ 別","數量"\n'), "utf-8");

      for (var i = 0; i < result.length; i++) {
        var record = result[i];
        res.write(new Buffer('"' + record.name + '",' + record.value + "\n"), "utf-8");
      }

      res.end();

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

      res.type('text/csv');

      res.write('"月份","數量"\n');

      for (var i = 0; i < data.length; i++) {
        var record = data[i];
        res.write('"' + record.name + '",' + record.value + "\n");
      }

      res.end();

    });
  },

  productMonth: function(req, res) {
    var product = req.param("product");
    var year = req.param("year");
    var month = req.param("month")

    var logTotal = LogTotal.jsonAPI();

    logTotal.productMonth(product, year, month, function(err, data) {

      if (err) {
        res.serverError(err);
        return;
      }

      res.type('text/csv');

      res.write(new Buffer('"週","數量"\n'));

      for (var i = 0; i < data.length; i++) {
        var record = data[i];
        res.write(new Buffer('"' + record.name + '",' + record.value + "\n"));
      }

      res.end();
    });
  },

  productMonthWeek: function(req, res) {
    var product = req.param("product");
    var year = req.param("year");
    var month = req.param("month");
    var week = req.param("week");

    var logTotal = LogTotal.jsonAPI();

    logTotal.productMonthWeek(product, year, month, week, function(err, data) {

      if (err) {
        res.serverError(err);
        return;
      }


      res.type('text/csv');

      res.write(new Buffer('"日","數量"\n'));

      for (var i = 0; i < data.length; i++) {
        var record = data[i];
        res.write(new Buffer('"' + record.name + '",' + record.value + "\n"));;
      }

      res.end();
    });
  },

  productMonthWeekDate: function(req, res) {
    var product = req.param("product");
    var year = req.param("year");
    var month = req.param("month");
    var week = req.param("week");
    var date = req.param("date");

    var logTotal = LogTotal.jsonAPI();

    logTotal.productMonthWeekDate(product, year, month, week, date, function(err, data) {

      if (err) {
        res.serverError(err);
        return;
      }

      res.type('text/csv');

      res.write(new Buffer('"機器","數量"\n'));

      for (var i = 0; i < data.length; i++) {
        var record = data[i];
        res.write(new Buffer('"' + record.name + '",' + record.value + "\n"));
      }

      res.end();
    });
  },

  machineDetail: function(req, res) {
    var product = req.param("product");
    var year = req.param("year");
    var month = req.param("month");
    var week = req.param("week");
    var date = req.param("date");
    var machine = req.param("machine");

    var logTotal = LogTotal.jsonAPI();

    logTotal.machineDetail(product, year, month, date, machine, function(err, data) {

      if (err) {
        res.serverError(err);
        return;
      }

      res.type('text/csv');
      res.write(new Buffer('"日期","錯誤種類","數量"\n'));

      for (var i = 0; i < data.length; i++) {
        var record = data[i];
        var time = 
          record.name.date.getFullYear() + "-" + (+record.name.date.getMonth() + 1) + "-" +
          record.name.date.getDate() + " " + record.name.date.getHours() + ":" +
          record.name.date.getMinutes() + ":" + record.name.date.getSeconds();

        res.write(new Buffer('"' + time + '","' + record.name.error + '",' + record.value + "\n"));
      }

      res.end();
    });
  }
};

