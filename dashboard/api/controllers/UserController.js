/**
 * UserController
 *
 * @description :: Server-side logic for managing users
 * @help        :: See http://links.sailsjs.org/docs/controllers
 */

module.exports = {

  signup:  function(req, res) {
    res.view();
  },

  create: function(req, res, next) {

    function redirectWithErrorMessage(message) {
      var errorMessage = [{name: "reqiured", message: message}]
      req.session.flash = {err: errorMessage};
      res.redirect("/user/signup");
    }

    var username = req.param("username");
    User.findOneByUsername(username, function (err, user) {
      if (err) { return next(err); }

      if (user) { 
        redirectWithErrorMessage("此使用者名稱已存在");
        return;
      }

      User.create(req.params.all(), function userCreated(err, newUser) {
        if (err) {
          redirectWithErrorMessage("所有欄位均為必填，且兩次輸入的密碼需相同，請再次檢查。");
          return;
        }

        req.session.authenticated = true;
        req.session.user = user;
        res.redirect("/dashboard");
      });
    });
  },

  show: function(req, res, next) {
    User.findOne({username: req.param("username")}, function foundUser(err, user) {
      if (err) { return next(err) }
      if (!user) { return next() }
      res.view({user: user});
    });
  },

  login: function(req, res, next) {

    var username = req.param("username");
    var password = req.param("password");

    function redirectWithErrorMessage() {
      var errorMessage = [{name: "reqiured", message: "使用者名稱及帳號錯誤"}]
      req.session.flash = {err: errorMessage};
      res.redirect("/");
    }

    if (!username || !password) {
      redirectWithErrorMessage();
      return;
    }

    User.findOneByUsername(username, function (err, user) {
      if (err) { return next(err); }

      if (!user) { 
        redirectWithErrorMessage(); 
        return; 
      }

      var bcrypt = require("bcrypt");

      bcrypt.compare(password, user.password, function(err, isValid) {

        if (err) { return next(err); }

        if (!isValid) {
          redirectWithErrorMessage(); 
          return; 
        }

        req.session.authenticated = true;
        req.session.user = user;
        res.redirect("/dashboard");
      });

    });

  },

  logout: function(req, res, next) {
    req.session.destroy();
    res.redirect("/");
  },

  resetPassword: function(req, res, next) {

    var confirmCode = req.param("confirmCode");
    var password = req.param("password");
    var confirmPassword = req.param("confirmPassword");
    var formURL = "/user/resetPassword/" + confirmCode

    function redirectWithErrorMessage(message, url) {
      var errorMessage = [{name: "reqiured", message: message}]
      req.session.flash = {err: errorMessage};
      res.redirect(url);
    }

    if (!confirmCode || !password || !confirmPassword) {
      redirectWithErrorMessage("所有欄位均為必填欄位", formURL);
      return;
    }

    if (password != confirmPassword) {
      redirectWithErrorMessage("兩次輸入的密碼不同，請再次檢查後重新設定", formURL);
      return;
    }

    ResetLink.findOneByConfirmCode(confirmCode, function(err, resetLink) {

      if (err) { return next(err); }

      if (!resetLink) {
        redirectWithErrorMessage("無效的表單", formURL);
        return;
      }

      var bcrypt = require("bcrypt");

      bcrypt.hash(password, 10, function passwordEncrypted(err, encrypedPassword) {
        if (err) { next(err); }

        ResetLink.destroy({username: resetLink.username}).exec(function(err, link) {});

        User.update(
          {username: resetLink.username}, 
          {password: encrypedPassword}
        ).exec(function (err, user) {
          redirectWithErrorMessage("密碼已變更，請重新登入", "/");
        });
      });

    });
  },

  resetPasswordForm: function(req, res, next) {

    function redirectWithErrorMessage(message) {
      var errorMessage = [{name: "reqiured", message: message}]
      req.session.flash = {err: errorMessage};
      res.redirect("/");
    }

    var confirmCode = req.param("confirmCode");

    ResetLink.findOneByConfirmCode(confirmCode, function(err, resetLink) {

      if (err) { return next(err); }

      if (!resetLink) {
        redirectWithErrorMessage("無效的密碼重設網址");
        return;
      }

      var oneDay = 1000 * 60 * 60 * 24;
      var duration = (new Date()).getTime() - resetLink.timestamp.getTime()

      if (duration > oneDay) {
        ResetLink.destroy({confirmCode: confirmCode}).exec(function (err, resetLink) {
          redirectWithErrorMessage("此重設網址已過期，請重新至「忘記密碼」頁面填寫表單。");
        });
      }

      res.view("user/resetPassword", {confirmCode: confirmCode});
    });
  },

  sendResetEmail: function(req, res, next) {

    var email = req.param("email");

    if (email && email.length > 0) {
      User.findOneByEmail(email, function (err, user) {

        if (err) { return next(err); }

        if (user) {
          var crypto = require('crypto');
          var token = crypto.randomBytes(16).toString('hex');
          var newLink = {
            username: user.username,
            email: user.email,
            confirmCode: token,
            timestamp: new Date()
          }

          ResetLink.create(newLink, function(err, newLink) {

            if (err) { return next(err); }

            console.log("Add user link....");
            console.log("Send email.....");

            var resetURL = "請至 http://[RealURL]/user/resetPassword/" + newLink.confirmCode;
            var plainTextBody = "請至 " + resetURL + " 重設您的密碼，此網址將在 24 小時候失效。";
            var htmlBody = "請至 <a href=\"" + resetURL + "\">" + resetURL + "</a> 重設您的密碼，此網址將在 24 小時候失效。";

            Mailer.sendMail(user.email, "Zhenhai Dashboard 密碼重設網址", plainTextBody, htmlBody);

            res.view('user/sendResetEmail');
          });
        }
      });
    }
  }
};

