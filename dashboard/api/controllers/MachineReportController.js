/**
 * DashboardController
 *
 * @description :: Server-side logic for managing users
 * @help        :: See http://links.sailsjs.org/docs/controllers
 */

module.exports = {

  overview: function(req, res) {
    res.view("machine/overview")
  },

  detail: function(req, res) {
    var machineID = req.param("machineID");
    var pieChartDataURL = "/api/json/machine/" + machineID + "/pie";
    var tableDataURL = "/api/json/machine/" + machineID + "/table"

    var steps = [
      {active: "active", title: "總覽"},
      {active: "active", title: machineID}
    ];

    var variables = {
      pieChartDataURL: pieChartDataURL,
      tableDataURL: tableDataURL,
      steps: steps
    }

    res.view("reason/detail", variables)
  }

};

