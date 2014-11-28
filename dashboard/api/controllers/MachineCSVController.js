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


      res.type('text/csv');
      res.write(new Buffer('"製程","數量"\n'));
      for (var i = 0; i < result.length; i++) {
        var record = result[i];
        res.write(new Buffer('"' + record.name + '",' + record.value + "\n"));
      }

      res.end();

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

      res.type('text/csv');
      res.write(new Buffer('"機種","數量"\n'));
      for (var i = 0; i < result.length; i++) {
        var record = result[i];
        res.write(new Buffer('"' + record.name + '",' + record.value + "\n"));
      }

      res.end();

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

      res.type('text/csv');
      res.write(new Buffer('"機台","數量"\n'));
      for (var i = 0; i < result.length; i++) {
        var record = result[i];
        res.write(new Buffer('"' + record.name + '",' + record.value + "\n"));
      }

      res.end();

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


      res.type('text/csv');
      res.write(new Buffer('"日期","數量","錯誤種類", "說明"\n'));

      for (var i = 0; i < result.length; i++) {
        var record = result[i];
        var machineModel = sails.config.machineModel[machineID]
        var pin = sails.config.pinDefine[machineModel]["P" + record.defact_id]
        res.write(new Buffer('"' + record.time + '", "' + record.bad_qty +'",' +  record.defact_id + ',"' + pin + '"' + "\n"));
      }

      res.end();

    });
  }


};

