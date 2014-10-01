exports.sendMail = function(to, subject, plainTextContent, htmlContent) {
  var nodemailer = require('nodemailer');
  var transporter = nodemailer.createTransport({
    host: sails.config.mail.host,
    port: sails.config.mail.port,
    secure: sails.config.mail.secure,
    auth: {
      user: sails.config.mail.username,
      pass: sails.config.mail.password
    }
  });

  var options = {
    form: sails.config.mail.from,
    to: to,
    subject: subject,
    text: plainTextContent,
    htmlContent: htmlContent
  }

  transporter.sendMail(options, function(error, info) {
    if (error) {
      console.log(error);
    } else {
      console.log("mail sent successfully to " + to);
    }
  });
}
