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

    var logTotal = LogTotal.jsonAPI();
    var logAlert = LogAlert.jsonAPI();

    logTotal.getDateRange(function(err, minDate, maxDate) {

      if (err) {
        res.serverError(err);
        return;
      }


      logAlert.hasData(function(hasAlert) {
        console.log("alert:" + hasAlert)
        var variables = {
          currentYear: (new Date()).getFullYear(),
          currentMonth: (+(new Date()).getMonth()) + 1,
          minYear: minDate.getFullYear(),
          minMonth: (+minDate.getMonth() + 1),
          maxYear: maxDate.getFullYear(),
          maxMonth: (+maxDate.getMonth() + 1),
          alertDisabled: hasAlert ? "" : "disabled",
          alertLink: hasAlert ? "/alert" : "#"
        }

        res.view("dashboard", variables);
      });

    });

  }


};

