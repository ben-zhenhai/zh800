/**
* User.js
*
* @description :: TODO: You might write a short summary of how this model works and what it represents here.
* @docs        :: http://sailsjs.org/#!documentation/models
*/

module.exports = {

  schema: true,
  attributes: {
    username:   {type: "string", required: true, unique: true},
    employeeID: {type: "string", required: true, unique: true},
    email:      {type: "email",  required: true, unique: true},
    password:   {type: "string", required: true}
  },

  // Convert password to bcrypt one-way hash function, so we don't have password stored
  // in plaintext.
  beforeCreate: function(values, next) {

    if (!values.password || values.password != values.passwordConfirm) {
      return next({err: ["Password doesn't match password confirmation."]})
    }

    var bcrypt = require("bcrypt");

    bcrypt.hash(values.password, 10, function passwordEncrypted(err, encrypedPassword) {
      if (err) { next(err); }
      values.password = encrypedPassword;
      next();
    });
  }
};

