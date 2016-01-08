var HOST = 'localhost'
var username = "zhenhai"
var password = "zhenhai123456"
var filename = process.argv.slice(2).toString();
var rabbitMQ = require('amqplib/callback_api')
var dataQueue = "rawDataLine"

rabbitMQ.connect('amqp://' + username + ":" + password + "@" + HOST, function(err, conn) {

  if (err != null) {
    console.log("[ERROR] cannot connect to rabbitMQ server, error:" + err);
    var fs = require('fs');
    fs.unlinkSync(filename);
    return;
  }

  conn.createConfirmChannel(function(err, channel) {

    if (err != null) {
      console.log("[ERROR] cannot open rabbitMQ channel error:" + err);
      var fs = require('fs');
      fs.unlinkSync(filename);
      return;
    }

    channel.assertQueue(dataQueue);

    var lineReader = require('line-reader');
    var totalLines = 0;
    var counter = 0;
    var isDone = false;
    var trim = require("trim");

    lineReader.eachLine(filename, function(line, isLast) {

      isDone = isLast;

      if (trim(line).length != 0) {

        totalLines++;

        channel.sendToQueue(dataQueue, new Buffer(line), {persistent: true}, function(err) {

          counter++;
          console.log("[OK] Send [" + counter + "][" + line + "] to server...");

          if (counter == totalLines && isDone == true) {
            var fs = require('fs');
            fs.unlinkSync(filename);
            console.log("END\n");
            conn.close();
            //process.exit();
          }
        });
      } else {
        conn.close();
      }
    });
  });
});
