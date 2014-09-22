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
    User.create(req.params.all(), function userCreated(err, newUser) {
      if (err) {
        req.session.flash = {
          'err': err
        }
        return res.redirect("/user/singup");
      }
      res.redirect("/user/show/" + newUser.username);
    })
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
      if (err) return next(err);

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

  sendResetEmail: function(req, res) {

    var email = req.param("email");

    if (email && email.length > 0) {
      User.findOneByEmail(email, function (err, user) {

        if (err) {
          res.serverError(err);
        }

        if (user) {
          var crypto = require('crypto');
          var token = crypto.randomBytes(16).toString('hex');
          var newLink = {
            username: user.username,
            email: user.email,
            randomString: token,
            timestamp: new Date()
          }

          ResetLink.create(newLink, function(err, newLink) {
            if (err) {
              res.serverError(err);
            }
            console.log("Add user link....");
            res.view('user/sendResetEmail');
          });
        }
      });
    }
  }
};

