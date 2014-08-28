var mongoose = require('mongoose')
var conn = mongoose.createConnection('mongodb://mongo_server/zhenhai')
var Data = require(__dirname + '/data_model').getModel(conn)

var net = require('net')
var server = net.createServer()

var array = []

server.on('connection', function(client) {
    client.setEncoding('utf8')

    client.on('data', function(data) {
        var tmp = data.replace(/(\r\n|\n|\r)/gm,'')
        //array = tmp.toString().split(/\t+/g)
        array = tmp.toString().split(',')

        console.log('size: ' + array.length)
        array.forEach(function(data) {
            console.log('hi: ' + data)
        })

        var data = new Data({
           order_type: array[0],
           lot_no: array[1],
           work_qty: array[2],
           count_qty: array[3],
           emb_date: array[4],
           bad_qty: array[5],
           mach_ip: array[6],
           defact_id: array[7],
           mach_id: array[8],
           work_id: array[9],
           CX: array[10],
           DX: array[11],
           LC: array[12],
           mach_status: array[13]
        })
        data.save(function(error) {
            if (error) console.error(error)
            console.log('add data ok.')
        })
        console.log('test')
    })

    client.on('close', function() {
        console.log('close')
    })

    client.on('end', function() {
        console.log('end')
    })
})

server.listen(5566)
