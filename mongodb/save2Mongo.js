var mongoURL = 'mongodb://localhost/zhenhai'
var mongoURLDaily = 'mongodb://localhost/daily'
var mongoURLMonthly = 'mongodb://localhost/monthly'
var mongoURLArchive = 'mongodb://localhost/archive'

var mongoose = require('mongoose')
var conn = mongoose.createConnection(mongoURL)
var Data = require(__dirname + '/data_model').getModel(conn)

var net = require('net')
var server = net.createServer()
 
var array = []
var recordCount = 0;
var BATCH_LIMIT = 1000000;

function startServer() {

    var fork = require('child_process').fork;
    var example1 = fork(__dirname + '/processMongo.js');

    server.on('connection', function(client) {
        client.setEncoding('utf8')
    
        client.on('data', function(data) {
          example1.send(data);
        })
    })
    
    server.listen(5566)
}

startServer();
