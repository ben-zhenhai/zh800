/**
 * DashboardController
 *
 * @description :: Server-side logic for managing users
 * @help        :: See http://links.sailsjs.org/docs/controllers
 */

module.exports = {

  overview: function(req, res) {

    var logMachine = LogMachine.jsonAPI();

    function convert(data) {
      return {
        name: data._id, 
        value: data.bad_qty,
        link: "/machine/" + data._id
      }
    }

    logMachine.overview(convert, function(err, result) {

      if (err) {
        res.serverError(err);
        return;
      }

      var lines = '"機器","數量"\n'

      res.type('text/csv');
      for (var i = 0; i < result.length; i++) {
        var record = result[i];
        lines += '"' + record.name + '",' + record.value + "\n";
      }

      res.end(lines);
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

      var lines = '"月份","錯誤種類","數量"\n'

      res.type('text/csv');
      for (var i = 0; i < result.length; i++) {
        var record = result[i];
        lines += '"' + record.time + '", "' + record.name +'",' +  record.value + "\n";
      }

      res.end(lines);
    });
  }

};

