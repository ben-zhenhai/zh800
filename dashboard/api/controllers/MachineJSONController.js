/**
 * DashboardController
 *
 * @description :: Server-side logic for managing users
 * @help        :: See http://links.sailsjs.org/docs/controllers
 */

module.exports = {

  overview: function(req, res) {

    var logMachine = LogMachine.jsonAPI();

    logMachine.overview(function(err, result) {

      if (err) {
        res.serverError(err);
        return;
      }

      var resultJSON = {dataSet: result}

      res.json(resultJSON);
    });
  },

  step: function(req, res) {

    var logMachine = LogMachine.jsonAPI();
    var step = req.param("step")

    logMachine.step(step, function(err, result) {

      if (err) {
        res.serverError(err);
        return;
      }

      var resultJSON = {dataSet: result}

      res.json(resultJSON);
    });
  },

  stepModel: function(req, res) {

    var logMachine = LogMachine.jsonAPI();
    var step = req.param("step")
    var model = req.param("model")

    logMachine.stepModel(step, model, function(err, result) {

      if (err) {
        res.serverError(err);
        return;
      }

      var resultJSON = {dataSet: result}

      res.json(resultJSON);
    });
  },

  detailPie: function(req, res) {

    var logMachine = LogMachine.jsonAPI();
    var machineID = req.param("machineID");

    logMachine.detailPie(machineID, function(err, result) {

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

    var logMachine = LogMachine.jsonAPI();
    var machineID = req.param("machineID");

    logMachine.detailTable(machineID, function(err, result) {

      if (err) {
        res.serverError(err);
        return;
      }

      res.json(result);
    });
  }


};

