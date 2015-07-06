var http = require('http');

var email = require('emailjs');

var error = 0;

var server = email.server.connect({
  user: "freeman@zhenhai.com.tw",
  password: "aaaa9881#",
  host: "www.zhenhai.com.tw",
  ssl: false
});

function sendMail(msg) {
  server.send(
    {
      text:msg,
      from: "ZhenHai 機台存活率檢查 <freeman@zhenhai.com.tw>",
      to: "z_h_e_n_h_a_i@mailinator.com",
      subject: "台容 謝崗廠 機台存活率過低"
    },
    function(err, msg) {
      console.log("err: " + err || msg);
    }
  );
}

function checkErrorRate() {
  if (error > 30) {
    sendMail("出錯機台 " + error + " %");
  }
}

process.on('message', function(msg) {
    error = msg;
});

setInterval(checkErrorRate, 10000, error);
