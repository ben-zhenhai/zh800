module.exports.http = {
  customMiddleware: function (app) {
    var logger = require('morgan');
    var fs = require('fs');
    //var accessLogStream = fs.createWriteStream(__dirname + '/../access.log', {flags: 'a'})
    var accessLogStream = fs.createWriteStream('/dev/null', {flags: 'a'})

    app.use(logger('combined', {stream: accessLogStream}));
  }
};
