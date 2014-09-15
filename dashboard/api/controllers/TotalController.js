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
    console.log("month:" + req.param("month"));
    res.view("total/overview", {dataURL: productDataURL});
  }

};

