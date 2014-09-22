/**
 * DashboardController
 *
 * @description :: Server-side logic for managing users
 * @help        :: See http://links.sailsjs.org/docs/controllers
 */

module.exports = {

  overview: function(req, res) {

    var logDaily = LogDaily.jsonAPI();
    var year = req.param("year");
    var month = req.param("month");

    logDaily.overview(year, month, function(err, result) {

      if (err) {
        res.serverError(err);
        return;
      }


      res.type('text/csv');
      res.write(new Buffer('"日","數量"\n'));
      for (var i = 0; i < result.length; i++) {
        var record = result[i];
        res.write(new Buffer('"' + record.name + '",' + record.value + "\n"));
      }

      res.end();

    });
  },

  yearMonthDate: function(req, res) {
    var logDaily = LogDaily.jsonAPI();
    var year = req.param("year");
    var month = req.param("month");
    var date = req.param("date");

    logDaily.yearMonthDate(year, month, date, function(err, result) {

      if (err) {
        res.serverError(err);
        return;
      }


      res.type('text/csv');
      res.write(new Buffer('"機器","數量"\n'));

      for (var i = 0; i < result.length; i++) {
        var record = result[i];
        res.write(new Buffer('"' + record.name + '",' + record.value + "\n"));;
      }

      res.end();
    });

  },

  machineDetail: function(req, res) {
    var logDaily = LogDaily.jsonAPI();
    var year = req.param("year");
    var month = req.param("month");
    var date = req.param("date");
    var machine = req.param("machine");

    logDaily.machineDetail(year, month, date, machine, function(err, data) {

      if (err) {
        res.serverError(err);
        return;
      }

      var lines = '"日期","錯誤種類","數量"\n'

      res.type('text/csv');
      res.write(new Buffer('"日期","錯誤種類","數量"\n'));

      for (var i = 0; i < data.length; i++) {
        var record = data[i];
        var time = 
          record.name.date.getFullYear() + "-" + (+record.name.date.getMonth() + 1) + "-" +
          record.name.date.getDate() + " " + record.name.date.getHours() + ":" +
          record.name.date.getMinutes() + ":" + record.name.date.getSeconds();

        res.write(new Buffer('"' + time + '","' + record.name.error + '",' + record.value + "\n"));;
      }

      res.end();
    });
  }         
};

