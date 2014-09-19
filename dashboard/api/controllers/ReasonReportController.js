/**
 * DashboardController
 *
 * @description :: Server-side logic for managing users
 * @help        :: See http://links.sailsjs.org/docs/controllers
 */

module.exports = {

  overview: function(req, res) {
    res.view("reason/overview")
  },

  detail: function(req, res) {
    var reasonID = req.param("reasonID");
    var pieChartDataURL = "/api/json/reason/" + reasonID + "/pie";
    var tableDataURL = "/api/json/reason/" + reasonID + "/table"
    var csvURL = "/api/csv/reason/" + reasonID;

    var steps = [
      {active: "active", title: "總覽"},
      {active: "active", title: "錯誤原因：" + reasonID}
    ];

    var variables = {
      pieChartDataURL: pieChartDataURL,
      tableDataURL: tableDataURL,
      csvURL: csvURL,
      steps: steps
    }

    res.view("reason/detail", variables)
  }


};

