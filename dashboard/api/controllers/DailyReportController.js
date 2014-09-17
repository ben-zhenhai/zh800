/**
 * DashboardController
 *
 * @description :: Server-side logic for managing users
 * @help        :: See http://links.sailsjs.org/docs/controllers
 */

module.exports = {

  overview: function(req, res) {

    var year = req.param("year");
    var month = req.param("month");
    var dataURL = "/api/json/daily/" + year + "/" + month

    var steps = [
      {active: "active", title: year + " 年 " + month + " 月"},
      {active: "", title: "日期"},
      {active: "", title: "機器"}
    ];

    res.view("daily/overview", {dataURL: dataURL, steps: steps});
  },

  yearMonthDate: function(req, res) {

    var year = req.param("year");
    var month = req.param("month");
    var date = req.param("date");
    var dataURL = "/api/json/daily/" + year + "/" + month + "/" + date

    var steps = [
      {active: "active", title: year + " 年 " + month + " 月"},
      {active: "active", title: date + " 日"},
      {active: "", title: "機器"}
    ];

    res.view("daily/overview", {dataURL: dataURL, steps: steps});
  }


};

