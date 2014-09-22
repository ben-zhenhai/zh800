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

      res.type('text/csv');

      res.write(new Buffer('"機器","數量"\n'));

      for (var i = 0; i < result.length; i++) {
        var record = result[i];
        res.write(new Buffer('"' + record.name + '",' + record.value + "\n"));;
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
      res.write(new Buffer('"月份","錯誤種類","數量"\n'));

      for (var i = 0; i < result.length; i++) {
        var record = result[i];
        res.write(new Buffer('"' + record.time + '", "' + record.name +'",' +  record.value + "\n"));;
      }

      res.end();
    });
  }

};

