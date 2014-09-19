/**
 * DashboardController
 *
 * @description :: Server-side logic for managing users
 * @help        :: See http://links.sailsjs.org/docs/controllers
 */

module.exports = {

  index: function(req, res) {

    var steps = [
      {active: "active", title: "總覽"},
      {active: "", title: "Φ 別"},
      {active: "", title: "月份"},
      {active: "", title: "週"},
      {active: "", title: "日期"},
      {active: "", title: "機器"}
    ];

    res.view(
      "total/overview", 
      {dataURL: "/api/json/total", 
       steps: steps,
       csvURL: "/api/csv/total"}
    );
  },

  product: function(req, res) {
    var product = req.param("product");
    var productDataURL = "/api/json/total/" + product;

    var steps = [
      {active: "active", title: "總覽"},
      {active: "active", title: product},
      {active: "", title: "月份"},
      {active: "", title: "週"},
      {active: "", title: "日期"},
      {active: "", title: "機器"}
    ];

    res.view(
      "total/overview", 
      {dataURL: productDataURL, 
       steps: steps, 
       csvURL: "/api/csv/total/" + product}
    );
  },

  productMonth: function(req, res) {
    var product = req.param("product");
    var year = req.param("year");
    var month = req.param("month");

    var steps = [
      {active: "active", title: "總覽"},
      {active: "active", title: product},
      {active: "active", title: year + "-" + month},
      {active: "", title: "週"},
      {active: "", title: "日期"},
      {active: "", title: "機器"}
    ];

    var productDataURL = "/api/json/total/" + product + "/" + year + "/" + month;
    var csvURL = "/api/csv/total/" + product + "/" + year + "/" + month;

    res.view(
      "total/overview", 
      {dataURL: productDataURL, 
       steps: steps, 
       csvURL: csvURL}
    );

  },

  productMonthWeek: function(req, res) {

    var product = req.param("product");
    var year = req.param("year");
    var month = req.param("month");
    var week = req.param("week");

    var steps = [
      {active: "active", title: "總覽"},
      {active: "active", title: product},
      {active: "active", title: year + "-" + month},
      {active: "active", title: "第 " + week + " 週"},
      {active: "", title: "日期"},
      {active: "", title: "機器"}
    ];


    var productDataURL = "/api/json/total/" + product + "/" + year + "/" + month + "/" + week;
    var csvURL = "/api/csv/total/" + product + "/" + year + "/" + month + "/" + week;

    res.view(
      "total/overview", 
      {dataURL: productDataURL, 
       steps: steps, 
       csvURL: csvURL}
    );
  },

  productMonthWeekDate: function(req, res) {

    var product = req.param("product");
    var year = req.param("year");
    var month = req.param("month");
    var week = req.param("week");
    var date = req.param("date");

    var steps = [
      {active: "active", title: "總覽"},
      {active: "active", title: product},
      {active: "active", title: year + "-" + month},
      {active: "active", title: "第 " + week + " 週"},
      {active: "active", title: date + " 日"},
      {active: "", title: "機器"}
    ];

    var productDataURL = "/api/json/total/" + product + "/" + year + "/" + month + "/" + week + "/" + date;
    var csvURL = "/api/csv/total/" + product + "/" + year + "/" + month + "/" + week + "/" + date;

    res.view(
      "total/overview", 
      {dataURL: productDataURL, 
       steps: steps, 
       csvURL: csvURL}
    );

  },

  productMonthWeekDateMachine: function(req, res) {

    var product = req.param("product");
    var year = req.param("year");
    var month = req.param("month");
    var week = req.param("week");
    var date = req.param("date");
    var machine = req.param("machine");

    var steps = [
      {active: "active", title: "總覽"},
      {active: "active", title: product},
      {active: "active", title: year + "-" + month},
      {active: "active", title: "第 " + week + " 週"},
      {active: "active", title: date + " 日"},
      {active: "active", title: machine }
    ];

    var productDataURL = "/api/json/total/" + product + "/" + year + "/" + month + 
                         "/" + week + "/" + date + "/" + machine;
    var csvURL = "/api/csv/total/" + product + "/" + year + "/" + month + 
                 "/" + week + "/" + date + "/" + machine;

    var year = req.param("month").split("-")[0];
    var month = +(req.param("month").split("-")[1]) - 1;
    var week = req.param("week");

    res.view("total/machine", {
      fullYear: year,
      month: month,
      week: week,
      date: date,
      productName: product,
      productMachine: machine,
      dataURL: productDataURL,
      steps: steps,
      machineList: sails.config.machineList,
      csvURL: csvURL
    });
  }
};

