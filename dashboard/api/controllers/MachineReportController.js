/**
 * DashboardController
 *
 * @description :: Server-side logic for managing users
 * @help        :: See http://links.sailsjs.org/docs/controllers
 */

module.exports = {

  overview: function(req, res) {
    var steps = [
      {active: "active", title: "總覽", link: "/machine"},
      {active: "", title: "製程"},
      {active: "", title: "機種"},
      {active: "", title: "機台"}
    ];

    var variables = {
      steps: steps,
      ajaxDataURL: "/api/json/machine",
      csvLink: "/api/csv/machine",
      threshold: 0.5
    }

    res.view("machine/overview", variables)
  },

  step: function(req, res) {
    var step = req.param("step");

    var steps = [
      {active: "active", title: "總覽", link: "/machine"},
      {active: "active", title: "製程：" + step, link: "/machine/" + step},
      {active: "", title: "機種"},
      {active: "", title: "機台"}
    ];

    var variables = {
      steps: steps,
      ajaxDataURL: "/api/json/machine/" + step,
      csvLink: "/api/csv/machine/" + step,
      threshold: 0.5
    }

    res.view("machine/overview", variables)
  },

  stepModel: function(req, res) {
    var step = req.param("step");
    var model = req.param("model");

    var steps = [
      {active: "active", title: "總覽", link: "/machine"},
      {active: "active", title: "製程：" + step, link: "/machine/" + step},
      {active: "active", title: "機種：" + model, link: "/machine/" + step + "/" + model},
      {active: "", title: "機台"}
    ];

    var variables = {
      steps: steps,
      ajaxDataURL: "/api/json/machine/" + step + "/" + model,
      csvLink: "/api/csv/machine/" + step + "/" + model,
      threshold: 0.2
    }

    res.view("machine/overview", variables)
  },


  detail: function(req, res) {
    var step = req.param("step")
    var model = req.param("model")
    var machineID = req.param("machineID");
    var pieChartDataURL = "/api/json/machine/" + step + "/" + model + "/" + machineID + "/pie";
    var tableDataURL = "/api/json/machine/" + step + "/" + model + "/" + machineID + "/table";
    var csvURL = "/api/csv/machine/" + step + "/" + model + "/" + machineID;

    var steps = [
      {active: "active", title: "總覽", link: "/machine"},
      {active: "active", title: "製程：" + step, link: "/machine/" + step},
      {active: "active", title: "機種：" + model, link: "/machine/" + step + "/" + model},
      {active: "active", title: "機台：" + machineID}
    ];

    var variables = {
      pieChartDataURL: pieChartDataURL,
      tableDataURL: tableDataURL,
      csvURL: csvURL,
      steps: steps,
      machineID: machineID
    }

    res.view("machine/detail", variables)
  }


};

