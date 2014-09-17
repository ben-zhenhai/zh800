/**
 * DashboardController
 *
 * @description :: Server-side logic for managing users
 * @help        :: See http://links.sailsjs.org/docs/controllers
 */

module.exports = {

  index: function(req, res) {
    res.view("index");
  },

  main: function(req, res) {
    var variables = {
      currentYear: (new Date()).getFullYear(),
      currentMonth: (+(new Date()).getMonth()) + 1,
    }
    res.view("dashboard", variables);
  }


};

