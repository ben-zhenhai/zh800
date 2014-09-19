/**
 * DashboardController
 *
 * @description :: Server-side logic for managing users
 * @help        :: See http://links.sailsjs.org/docs/controllers
 */

module.exports = {

  year: function(req, res) {
    var year = req.param("year");
    var dataURL = "/api/json/monthly/" + year
    var csvURL = "/api/csv/monthly/" + year

    var steps = [
      {active: "active", title: year + " 年"},
      {active: "", title: "月份"},
      {active: "", title: "週"},
      {active: "", title: "日期"},
      {active: "", title: "機器"}
    ];

    res.view(
      "monthly/overview", 
      {dataURL: dataURL, steps: steps, csvURL: csvURL}
    );
  },

  yearMonth: function(req, res) {
    var year = req.param("year");
    var month = req.param("month");
    var dataURL = "/api/json/monthly/" + year + "/" + month;
    var csvURL = "/api/csv/monthly/" + year + "/" + month;

    var steps = [
      {active: "active", title: year + " 年"},
      {active: "active", title: month + " 月"},
      {active: "", title: "週"},
      {active: "", title: "日期"},
      {active: "", title: "機器"}
    ];

    res.view(
      "monthly/overview", 
      {dataURL: dataURL, steps: steps, csvURL: csvURL}
    );
  },

  yearMonthWeek: function(req, res) {
    var year = req.param("year");
    var month = req.param("month");
    var week = req.param("week");
    var dataURL = "/api/json/monthly/" + year + "/" + month + "/" + week;
    var csvURL = "/api/csv/monthly/" + year + "/" + month + "/" + week;

    var steps = [
      {active: "active", title: year + " 年"},
      {active: "active", title: month + " 月"},
      {active: "active", title: "第 " + week + " 週"},
      {active: "", title: "日期"},
      {active: "", title: "機器"}
    ];

    res.view(
      "monthly/overview", 
      {dataURL: dataURL, steps: steps, csvURL: csvURL}
    );

  },

  yearMonthWeekDate: function(req, res) {
    var year = req.param("year");
    var month = req.param("month");
    var week = req.param("week");
    var date = req.param("date");
    var dataURL = "/api/json/monthly/" + year + "/" + month + "/" + week + "/" + date;
    var csvURL = "/api/csv/monthly/" + year + "/" + month + "/" + week + "/" + date;

    var steps = [
      {active: "active", title: year + " 年"},
      {active: "active", title: month + " 月"},
      {active: "active", title: "第 " + week + " 週"},
      {active: "active", title: date + " 日"},
      {active: "", title: "機器"}
    ];

    res.view(
      "monthly/overview", 
      {dataURL: dataURL, steps: steps, csvURL: csvURL}
    );

  },

  machineDetail: function(req, res) {

    var year = req.param("year");
    var month = req.param("month");
    var week = req.param("week");
    var date = req.param("date");
    var machine = req.param("machine");
    var dataURL = "/api/json/monthly/" + year + "/" + month + "/" + week + "/" + date + "/" + machine;
    var csvURL = "/api/csv/monthly/" + year + "/" + month + "/" + week + "/" + date + "/" + machine;

    var steps = [
      {active: "active", title: year + " 年"},
      {active: "active", title: month + " 月"},
      {active: "active", title: "第 " + week + " 週"},
      {active: "active", title: date + " 日"},
      {active: "active", title: machine }
    ];

    res.view("monthly/machine", {
      fullYear: year,
      month: month,
      week: week,
      date: date,
      productMachine: machine,
      dataURL: dataURL,
      steps: steps,
      machineList: sails.config.machineList,
      csvURL: csvURL
    });
  }
};

