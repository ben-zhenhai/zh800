/**
 * Route Mappings
 * (sails.config.routes)
 *
 * Your routes map URLs to views and controllers.
 *
 * If Sails receives a URL that doesn't match any of the routes below,
 * it will check for matching files (images, scripts, stylesheets, etc.)
 * in your assets directory.  e.g. `http://localhost:1337/images/foo.jpg`
 * might match an image file: `/assets/images/foo.jpg`
 *
 * Finally, if those don't match either, the default 404 handler is triggered.
 * See `api/responses/notFound.js` to adjust your app's 404 logic.
 *
 * Note: Sails doesn't ACTUALLY serve stuff from `assets`-- the default Gruntfile in Sails copies
 * flat files from `assets` to `.tmp/public`.  This allows you to do things like compile LESS or
 * CoffeeScript for the front-end.
 *
 * For more information on configuring custom routes, check out:
 * http://sailsjs.org/#/documentation/concepts/Routes/RouteTargetSyntax.html
 */

module.exports.routes = {

  
  /***************************************************************************
  *                                                                          *
  * Make the view located at `views/homepage.ejs` (or `views/homepage.jade`, *
  * etc. depending on your default view engine) your home page.              *
  *                                                                          *
  * (Alternatively, remove this and add an `index.html` file in your         *
  * `assets` directory)                                                      *
  *                                                                          *
  ***************************************************************************/

  "/forget": {
    view: 'forget'
  },
  "GET /": "Dashboard.index",
  "GET /dashboard": "Dashboard.main",
  "GET /user/show/:username": "User.show",
  "GET /user/signup": "User.signup",
  "POST /user/create": "User.create",
  "POST /user/login": "User.login",
  "GET /user/logout": "User.logout",
  "POST /user/sendRestEmail": "User.sendResetEmail",
  "GET /user/resetPassword/:confirmCode": "User.resetPasswordForm",
  "POST /user/resetPassword/reset": "User.resetPassword",

  // Total Dashboard
  "GET /total": "TotalReport.index",
  "GET /total/:product": "TotalReport.product",
  "GET /total/:product/:year/:month": "TotalReport.productMonth",
  "GET /total/:product/:year/:month/:week": "TotalReport.productMonthWeek",
  "GET /total/:product/:year/:month/:week/:date": "TotalReport.productMonthWeekDate",
  "GET /total/:product/:year/:month/:week/:date/:machine": "TotalReport.productMonthWeekDateMachine",

  // Monthly Dashboard
  "GET /monthly/:year": "MonthlyReport.year",
  "GET /monthly/:year/:month": "MonthlyReport.yearMonth",
  "GET /monthly/:year/:month/:week": "MonthlyReport.yearMonthWeek",
  "GET /monthly/:year/:month/:week/:date": "MonthlyReport.yearMonthWeekDate",
  "GET /monthly/:year/:month/:week/:date/:machine": "MonthlyReport.machineDetail",

  // Daily Dashboard
  "GET /daily/:year/:month": "DailyReport.overview",
  "GET /daily/:year/:month/:date": "DailyReport.yearMonthDate",
  "GET /daily/:year/:month/:date/:machine": "DailyReport.machineDetail",

  // By Reason Dashboard
  "GET /reason": "ReasonReport.overview",
  "GET /reason/:step": "ReasonReport.step",
  "GET /reason/:step/:model": "ReasonReport.stepModel",
  "GET /reason/:step/:model/:machineID": "ReasonReport.detail",

  // Alert Dashboard
  "GET /alert": "AlertReport.overview",

  // Total Reivew JSON API
  "GET /api/json/total": "TotalJSON.overview",
  "GET /api/json/total/:product": "TotalJSON.product",
  "GET /api/json/total/:product/:year/:month": "TotalJSON.productMonth",
  "GET /api/json/total/:product/:year/:month/:week": "TotalJSON.productMonthWeek",
  "GET /api/json/total/:product/:year/:month/:week/:date": "TotalJSON.productMonthWeekDate",
  "GET /api/json/total/:product/:year/:month/:week/:date/:machine": "TotalJSON.machineDetail",

  // Monthly Review JSON API
  "GET /api/json/monthly/:year": "MonthlyJSON.overview",
  "GET /api/json/monthly/:year/:month": "MonthlyJSON.yearMonth",
  "GET /api/json/monthly/:year/:month/:week": "MonthlyJSON.yearMonthWeek",
  "GET /api/json/monthly/:year/:month/:week/:date": "MonthlyJSON.yearMonthWeekDate",
  "GET /api/json/monthly/:year/:month/:week/:date/:machine": "MonthlyJSON.machineDetail",

  // Daily Review JSON API
  "GET /api/json/daily/:year/:month": "DailyJSON.overview",
  "GET /api/json/daily/:year/:month/:date": "DailyJSON.yearMonthDate",
  "GET /api/json/daily/:year/:month/:date/:machine": "DailyJSON.machineDetail",

  // By reason JSON API
  "GET /api/json/reason": "ReasonJSON.overview",
  "GET /api/json/reason/:step": "ReasonJSON.step",
  "GET /api/json/reason/:step/:model": "ReasonJSON.stepModel",
  "GET /api/json/reason/:step/:model/:machineID/pie": "ReasonJSON.detailPie",
  "GET /api/json/reason/:step/:model/:machineID/table": "ReasonJSON.detailTable",

  // Total Reivew CSV API
  "GET /api/csv/total": "TotalCSV.overview",
  "GET /api/csv/total/:product": "TotalCSV.product",
  "GET /api/csv/total/:product/:year/:month": "TotalCSV.productMonth",
  "GET /api/csv/total/:product/:year/:month/:week": "TotalCSV.productMonthWeek",
  "GET /api/csv/total/:product/:year/:month/:week/:date": "TotalCSV.productMonthWeekDate",
  "GET /api/csv/total/:product/:year/:month/:week/:date/:machine": "TotalCSV.machineDetail",

  // Monthly Review CSV API
  "GET /api/csv/monthly/:year": "MonthlyCSV.overview",
  "GET /api/csv/monthly/:year/:month": "MonthlyCSV.yearMonth",
  "GET /api/csv/monthly/:year/:month/:week": "MonthlyCSV.yearMonthWeek",
  "GET /api/csv/monthly/:year/:month/:week/:date": "MonthlyCSV.yearMonthWeekDate",
  "GET /api/csv/monthly/:year/:month/:week/:date/:machine": "MonthlyCSV.machineDetail",

  // Daily Review CSV API
  "GET /api/csv/daily/:year/:month": "DailyCSV.overview",
  "GET /api/csv/daily/:year/:month/:date": "DailyCSV.yearMonthDate",
  "GET /api/csv/daily/:year/:month/:date/:machine": "DailyCSV.machineDetail",

  // By reason CSV API
  "GET /api/csv/reason": "ReasonCSV.overview",
  "GET /api/csv/reason/:step": "ReasonCSV.step",
  "GET /api/csv/reason/:step/:model": "ReasonCSV.stepModel",
  "GET /api/csv/reason/:step/:model/:machineID": "ReasonCSV.detailTable",


  // Alert Dashboard
  "GET /api/json/alert": "AlertJSON.overview",


  /***************************************************************************
  *                                                                          *
  * Custom routes here...                                                    *
  *                                                                          *
  *  If a request to a URL doesn't match any of the custom routes above, it  *
  * is matched against Sails route blueprints. See `config/blueprints.js`    *
  * for configuration options and examples.                                  *
  *                                                                          *
  ***************************************************************************/

};
