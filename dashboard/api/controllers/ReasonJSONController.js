/**
 * DashboardController
 *
 * @description :: Server-side logic for managing users
 * @help        :: See http://links.sailsjs.org/docs/controllers
 */

module.exports = {

  overview: function(req, res) {

    var logReason = LogReason.jsonAPI();

    logReason.overview(function(err, result) {

      if (err) {
        res.serverError(err);
        return;
      }

      var resultJSON = {dataSet: result}

      res.json(resultJSON);
    });
  },

  step: function(req, res) {

    var logReason = LogReason.jsonAPI();
    var step = req.param("step")

    logReason.step(step, function(err, result) {

      if (err) {
        res.serverError(err);
        return;
      }

      var resultJSON = {dataSet: result}

      res.json(resultJSON);
    });
  },

  stepModel: function(req, res) {

    var logReason = LogReason.jsonAPI();
    var step = req.param("step")
    var model = req.param("model")

    logReason.stepModel(step, model, function(err, result) {

      if (err) {
        res.serverError(err);
        return;
      }

      var resultJSON = {dataSet: result}

      res.json(resultJSON);
    });
  },

  detailPie: function(req, res) {

    var logReason = LogReason.jsonAPI();
    var machineID = req.param("machineID");

    logReason.detailPie(machineID, function(err, result) {

      if (err) {
        res.serverError(err);
        return;
      }

      var resultJSON = {
        steps: ["錯誤原因：" + machineID],
        dataSet: result
      }

      res.json(resultJSON);
    });
  },

  detailTable: function(req, res) {

    var logReason = LogReason.jsonAPI();
    var machineID = req.param("machineID");

    logReason.detailTable(machineID, function(err, result) {

      if (err) {
        res.serverError(err);
        return;
      }

      res.json(result);
    });
  }


};

