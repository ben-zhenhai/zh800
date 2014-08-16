var http = require('http');
var fs = require('fs');
var express = require('express');
var morgan = require('morgan')
var app = express();
var routes = require('./routes')(app);

app.set('view engine', 'jade');
app.set('views', './views');
app.use(express.static('./public'));
app.use(express.static('./javascript'));
app.use(express.static('./files'));
app.use(express.static('./images'));
app.use(express.static('./views'));
app.use(morgan('tiny',{
  stream : fs.createWriteStream('app.log', {'flags':'w'})
}));
app.use(morgan('tiny'));


   http.createServer(app).listen(3000, function(){
     console.log('Express server listening on port ' + 3000);
});
   