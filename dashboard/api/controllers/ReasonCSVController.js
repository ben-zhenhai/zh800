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

    var logReason = LogReason.jsonAPI();
    var step = req.param("step")

    logReason.step(step, function(err, result) {

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

    var logReason = LogReason.jsonAPI();
    var step = req.param("step")
    var model = req.param("model")

    logReason.stepModel(step, model, function(err, result) {

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

    var logReason = LogReason.jsonAPI();
    var machineID = req.param("machineID");

    logReason.detailTable(machineID, function(err, result) {

      if (err) {
        res.serverError(err);
        return;
      }


      res.type('text/csv');
      res.write(new Buffer('"日期","數量","錯誤種類"\n'));

      for (var i = 0; i < result.length; i++) {
        var record = result[i];
        res.write(new Buffer('"' + record.time + '", "' + record.bad_qty +'",' +  record.defact_id + "\n"));
      }

      res.end();

    });
  }


};

