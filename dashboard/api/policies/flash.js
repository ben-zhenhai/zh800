/**
 *  The policy will automatically display message when there is
 *  message content in `req.session.flash` vairable, and clear 
 *  it after the message been displayed.
 */
module.exports = function (req, res, next) {

  res.locals.flash = {};
  
  // If there is no message, we simply continue next middleware.
  if (!req.session.flash) {
    return next()
  }

  res.locals.flash = _.clone(req.session.flash);
  req.session.flash = {};
  next();
}
