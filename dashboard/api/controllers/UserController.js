/**
 * UserController
 *
 * @description :: Server-side logic for managing users
 * @help        :: See http://links.sailsjs.org/docs/controllers
 */

module.exports = {

  singup:  function(req, res) {
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
      res.json(newUser);
    })
  }
	
};

