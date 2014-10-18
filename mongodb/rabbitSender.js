var rabbitMQ = require('amqplib/callback_api')
var q = "test"

rabbitMQ.connect('amqp://localhost', function(err, conn) {
  conn.createChannel(on_open);
  function on_open(err, ch) {
    if (err != null) {
      console.log("error:" + err);
    }

    ch.assertQueue(q);

    process.on('message', function(m) {
      //console.log('CHILD got message:' + m);
      ch.sendToQueue(q, new Buffer(m), {persistent: true});
    });
    process.send("RabbitOK");
  }
});
