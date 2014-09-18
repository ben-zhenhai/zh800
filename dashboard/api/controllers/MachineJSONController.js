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

  detailPie: function(req, res) {

    var logMachine = LogMachine.jsonAPI();
    var machineID = req.param("machineID");

    logMachine.detailPie(machineID, function(err, result) {

      if (err) {
        res.serverError(err);
        return;
      }

      var resultJSON = {
        steps: [machineID],
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

