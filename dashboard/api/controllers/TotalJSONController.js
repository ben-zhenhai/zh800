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

      var resultJSON = {dataSet: result}
      res.json(resultJSON);
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

      var resultJSON = {
        steps: [product],
        dataSet: data
      }
      res.json(resultJSON);
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

      var resultJSON = {
        steps: [product, year + "-" + month],
        dataSet: data
      }

      res.json(resultJSON);
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

      var resultJSON = {
        steps: [product, year + "-" + month, "第 " + week + " 週"],
        dataSet: data
      }

      res.json(resultJSON);
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

      var resultJSON = {
        steps: [product, year + "-" + month, "第 " + week + " 週", date + " 日"],
        dataSet: data
      }

      res.json(resultJSON);
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

      var resultJSON = {
        steps: [product, year + "-" + month, "第 " + week + " 週", date + " 日", machine],
        dataSet: data
      }

      res.json(resultJSON);
    });
  }
};

