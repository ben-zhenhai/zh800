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

  reasonDetail: function(req, res) {
    var reasonID = req.param("reasonID");
    var pieChartDataURL = "/api/json/reason/" + reasonID;
    var tableDataURL = "/api/json/reason/" + reasonID + "/table"

    var steps = [
      {active: "active", title: "總覽"},
      {active: "active", title: "錯誤原因：" + reasonID}
    ];

    var variables = {
      pieChartDataURL: pieChartDataURL,
      tableDataURL: tableDataURL,
      steps: steps
    }

    res.view("reason/detail", variables)
  }


};

