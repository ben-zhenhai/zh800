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
    var csvURL = "/api/csv/daily/" + year + "/" + month

    var steps = [
      {active: "active", title: year + " 年 " + month + " 月", link: "/daily/" + year + "/" + month},
      {active: "", title: "日期"},
      {active: "", title: "機器"}
    ];

    res.view(
      "daily/overview", 
      {dataURL: dataURL, steps: steps, csvURL: csvURL}
    );
  },

  yearMonthDate: function(req, res) {

    var year = req.param("year");
    var month = req.param("month");
    var date = req.param("date");
    var dataURL = "/api/json/daily/" + year + "/" + month + "/" + date
    var csvURL = "/api/csv/daily/" + year + "/" + month + "/" + date

    var steps = [
      {active: "active", title: year + " 年 " + month + " 月", link: "/daily/" + year + "/" + month},
      {active: "active", title: date + " 日", link: "/daily/" + year + "/" + month + "/" + date},
      {active: "", title: "機器"}
    ];

    res.view(
      "daily/overview", 
      {dataURL: dataURL, steps: steps, csvURL: csvURL}
    );
  },

  machineDetail: function(req, res) {

    var year = req.param("year");
    var month = req.param("month");
    var date = req.param("date");
    var machine = req.param("machine");
    var dataURL = "/api/json/daily/" + year + "/" + month + "/" + date + "/" + machine;
    var csvURL = "/api/csv/daily/" + year + "/" + month + "/" + date + "/" + machine;

    var steps = [
      {active: "active", title: year + " 年 " + month + " 月", link: "/daily/" + year + "/" + month },
      {active: "active", title: date + " 日", link: "/daily/" + year + "/" + month + "/" + date},
      {active: "active", title: machine }
    ];

    res.view("daily/machine", {
      fullYear: year,
      month: month,
      date: date,
      productMachine: machine,
      dataURL: dataURL,
      steps: steps,
      machineList: sails.config.machineList,
      machineID: machine,
      csvURL: csvURL
    });
  }


};

