/**
 * DashboardController
 *
 * @description :: Server-side logic for managing users
 * @help        :: See http://links.sailsjs.org/docs/controllers
 */

module.exports = {

  overview: function(req, res) {
    var steps = [
      {active: "active", title: "總覽"},
      {active: "", title: "製程"},
      {active: "", title: "機種"},
      {active: "", title: "機台"}
    ];

    var variables = {
      steps: steps,
      ajaxDataURL: "/api/json/reason",
      csvLink: "/api/csv/reason",
      threshold: 0.5
    }

    res.view("reason/overview", variables)
  },

  step: function(req, res) {
    var step = req.param("step");

    var steps = [
      {active: "active", title: "總覽"},
      {active: "active", title: "製程：" + step},
      {active: "", title: "機種"},
      {active: "", title: "機台"}
    ];

    var variables = {
      steps: steps,
      ajaxDataURL: "/api/json/reason/" + step,
      csvLink: "/api/csv/reason/" + step,
      threshold: 0.5
    }

    res.view("reason/overview", variables)
  },

  stepModel: function(req, res) {
    var step = req.param("step");
    var model = req.param("model");

    var steps = [
      {active: "active", title: "總覽"},
      {active: "active", title: "製程：" + step},
      {active: "active", title: "機種：" + model},
      {active: "", title: "機台"}
    ];

    var variables = {
      steps: steps,
      ajaxDataURL: "/api/json/reason/" + step + "/" + model,
      csvLink: "/api/csv/reason/" + step + "/" + model,
      threshold: 0.2
    }

    res.view("reason/overview", variables)
  },


  detail: function(req, res) {
    var step = req.param("step")
    var model = req.param("model")
    var machineID = req.param("machineID");
    var pieChartDataURL = "/api/json/reason/" + step + "/" + model + "/" + machineID + "/pie";
    var tableDataURL = "/api/json/reason/" + step + "/" + model + "/" + machineID + "/table";
    var csvURL = "/api/csv/reason/" + step + "/" + model + "/" + machineID;

    var steps = [
      {active: "active", title: "總覽"},
      {active: "active", title: "製程：" + step},
      {active: "active", title: "機種：" + model},
      {active: "active", title: "機台：" + machineID}
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

