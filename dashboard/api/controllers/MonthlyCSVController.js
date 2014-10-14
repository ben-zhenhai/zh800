/**
 * DashboardController
 *
 * @description :: Server-side logic for managing users
 * @help        :: See http://links.sailsjs.org/docs/controllers
 */

module.exports = {

  overview: function(req, res) {

    var logMonth = LogMonthly.jsonAPI();
    var year = req.param("year");

    logMonth.overview(year, function(err, result) {

      if (err) {
        res.serverError(err);
        return;
      }

      res.type('text/csv');
      res.write(new Buffer('"月","數量"\n'));

      for (var i = 0; i < result.length; i++) {
        var record = result[i];
        res.write(new Buffer('"' + record.name + '",' + record.value + "\n"));
      }

      res.end();
    });
  },

  yearMonth: function(req, res) {

    var logMonth = LogMonthly.jsonAPI();
    var year = req.param("year");
    var month = req.param("month");

    logMonth.yearMonth(year, month, function(err, result) {

      if (err) {
        res.serverError(err);
        return;
      }


      res.type('text/csv');
      res.write(new Buffer('"週","數量"\n'));

      for (var i = 0; i < result.length; i++) {
        var record = result[i];
        res.write(new Buffer('"' + record.name + '",' + record.value + "\n"));;
      }

      res.end();

    });
  },

  yearMonthWeek: function(req, res) {

    var logMonth = LogMonthly.jsonAPI();
    var year = req.param("year");
    var month = req.param("month");
    var week = req.param("week");

    logMonth.yearMonthWeek(year, month, week, function(err, result) {

      if (err) {
        res.serverError(err);
        return;
      }

      var lines = '"日","數量"\n'

      res.type('text/csv');
      res.write(new Buffer('"日","數量"\n'));

      for (var i = 0; i < result.length; i++) {
        var record = result[i];
        res.write(new Buffer('"' + record.name + '",' + record.value + "\n"));
      }

      res.end();
    });

  },

  yearMonthWeekDate: function(req, res) {

    var logMonth = LogMonthly.jsonAPI();
    var year = req.param("year");
    var month = req.param("month");
    var week = req.param("week");
    var date = req.param("date");

    logMonth.yearMonthWeekDate(year, month, week, date, function(err, result) {

      if (err) {
        res.serverError(err);
        return;
      }


      res.type('text/csv');
      res.write(new Buffer('"機器","數量"\n'));

      for (var i = 0; i < result.length; i++) {
        var record = result[i];
        res.write(new Buffer('"' + record.name + '",' + record.value + "\n"));
      }

      res.end();
    });
  },

  machineDetail: function(req, res) {

    var logMonth = LogMonthly.jsonAPI();
    var year = req.param("year");
    var month = req.param("month");
    var week = req.param("week");
    var date = req.param("date");
    var machine = req.param("machine");

    logMonth.machineDetail(year, month, date, machine, function(err, data) {

      if (err) {
        res.serverError(err);
        return;
      }

      res.type('text/csv');
      res.write(new Buffer('"日期","生產數量","錯誤數量","錯誤種類"\n'));

      for (var i = 0; i < data.length; i++) {
        var record = data[i];
        res.write(new Buffer('"' + record.timestamp + '",' + record.count_qty + ',' + record.bad_qty + ',' + record.defact_id + "\n"));
      }

      res.end();
    });
  }


};

