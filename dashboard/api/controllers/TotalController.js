/**
 * DashboardController
 *
 * @description :: Server-side logic for managing users
 * @help        :: See http://links.sailsjs.org/docs/controllers
 */

module.exports = {

  index: function(req, res) {
    res.view("total/overview", {dataURL: "/api/json/total/overview"});
  },

  product: function(req, res) {
    var productDataURL = "/api/json/total/" + req.param("product");
    res.view("total/overview", {dataURL: productDataURL});
  },

  productMonth: function(req, res) {
    var productDataURL = "/api/json/total/" + req.param("product") + "/" + req.param("month");
    res.view("total/overview", {dataURL: productDataURL});
  },

  productMonthWeek: function(req, res) {
    var productDataURL = "/api/json/total/" + req.param("product") + "/" + req.param("month") + "/" + req.param("week");
    res.view("total/overview", {dataURL: productDataURL});
  },

  productMonthWeekDate: function(req, res) {
    var productDataURL = "/api/json/total/" + req.param("product") + "/" + req.param("month") + "/" + req.param("week") + "/" + req.param("date");
    res.view("total/overview", {dataURL: productDataURL});
  },

  productMonthWeekDateMachine: function(req, res) {
    var productDataURL = "/api/json/total/" + req.param("product") + "/" + req.param("month") + "/" + req.param("week") + "/" + req.param("date") + "/" + req.param("machine");
    var year = req.param("month").split("-")[0];
    var month = +(req.param("month").split("-")[1]) - 1;
    res.view("total/machine", {
      fullYear: year,
      month: month,
      date: req.param("date"),
      dataURL: productDataURL
    });
  }




};

