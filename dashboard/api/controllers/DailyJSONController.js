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

      var resultJSON = {dataSet: result}

      res.json(resultJSON);
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

      var resultJSON = {dataSet: result}

      res.json(resultJSON);
    });

  },

  machineDetail: function(req, res) {
    var logDaily = LogDaily.jsonAPI();
    var year = req.param("year");
    var month = req.param("month");
    var date = req.param("date");
    var machine = req.param("machine");

    logDaily.machineDetail(year, month, date, machine, function(err, result) {

      if (err) {
        res.serverError(err);
        return;
      }

      var resultJSON = {dataSet: result}

      res.json(resultJSON);
    });
  }         
};

