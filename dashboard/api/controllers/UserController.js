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
    console.log("I'm here!")
    User.create(req.params.all(), function userCreated(err, newUser) {
      if (err) {
        console.log(err);
        req.session.flash = {
          'err': err
        }
        return res.redirect("/user/singup");
      }
      res.redirect("/user/show/" + newUser.username);
    })
  },

  show: function(req, res, next) {
    console.log(req.param("username"));
    User.findOne({username: req.param("username")}, function foundUser(err, user) {

      if (err) { return next(err) }
      if (!user) { return next() }
      res.view({user: user});
    });
  }
};

