module.exports = function (req, res, next) {
  
  if (req.session.authenticated) {
    res.redirect("/dashboard")
    return;
  } else {
    return next();
  }
}
