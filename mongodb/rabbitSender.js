var rabbitMQ = require('amqplib/callback_api')
var dataQueue = "rawDataLine"

rabbitMQ.connect('amqp://localhost', function(err, conn) {


  if (err != null) {
    console.log("cannot connect to rabbitMQ server, error:" + err);
    return;
  }

  conn.createChannel(on_open);

  function on_open(err, ch) {
    if (err != null) {
      console.log("cannot open rabbitMQ channel error:" + err);
      return;
    }

    ch.assertQueue(dataQueue);

    process.on('message', function(m) {
      ch.sendToQueue(dataQueue, new Buffer(m), {persistent: true});
    });
    process.send("RabbitOK");
  }
});
