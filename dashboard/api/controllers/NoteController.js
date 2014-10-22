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

};

