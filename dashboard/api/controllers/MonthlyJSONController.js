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

      var resultJSON = {dataSet: result}

      res.json(resultJSON);
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

      var resultJSON = {
        steps: [month + " 月"],
        dataSet: result
      }

      res.json(resultJSON);
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

      var resultJSON = {
        steps: [month + " 月", "第 " + week + " 週"],
        dataSet: result
      }

      res.json(resultJSON);
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

      var resultJSON = {
        steps: [month + " 月", "第 " + week + " 週", date + " 日"],
        dataSet: result
      }

      res.json(resultJSON);
    });
  },

  machineDetail: function(req, res) {

    var logMonth = LogMonthly.jsonAPI();
    var year = req.param("year");
    var month = req.param("month");
    var week = req.param("week");
    var date = req.param("date");
    var machine = req.param("machine");

    logMonth.machineDetail(year, month, date, machine, function(err, result) {

      if (err) {
        res.serverError(err);
        return;
      }

      var resultJSON = {
        steps: [month + " 月", "第 " + week + " 週", date + " 日", machine],
        dataSet: result
      }

      res.json(resultJSON);
    });
  }


};

