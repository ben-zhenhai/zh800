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

  detailPie: function(req, res) {

    var logReason = LogReason.jsonAPI();
    var reasonID = req.param("reasonID");

    logReason.reasonDetail(reasonID, function(err, result) {

      if (err) {
        res.serverError(err);
        return;
      }

      var resultJSON = {
        steps: ["錯誤原因：" + reasonID],
        dataSet: result
      }

      res.json(resultJSON);
    });
  },

  detailTable: function(req, res) {

    var logReason = LogReason.jsonAPI();
    var reasonID = req.param("reasonID");

    logReason.detailTable(reasonID, function(err, result) {

      if (err) {
        res.serverError(err);
        return;
      }

      res.json(result);
    });
  }


};

