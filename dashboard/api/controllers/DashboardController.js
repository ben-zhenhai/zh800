/**
 * DashboardController
 *
 * @description :: Server-side logic for managing users
 * @help        :: See http://links.sailsjs.org/docs/controllers
 */

module.exports = {

  index: function(req, res) {
    res.view("index");
  },

  main: function(req, res) {
    res.view("dashboard");
  },

  totalOverviewJSON: function(req, res) {
    Log.overviewByOrderType(res, function(result) {
      var resultJSON = {dataSet: result}
      res.json(resultJSON);
    });
  },

  totalProductJSON: function(req, res) {
    var product = req.param("product");

    Log.totalProduct(product, function(err, data) {

      if (err) {
        res.serverError(err);
      }

      var resultJSON = {
        steps: [product],
        dataSet: data
      }
      res.json(resultJSON);
    });
  }

};

