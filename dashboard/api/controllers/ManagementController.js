/**
 * UserController
 *
 * @description :: Server-side logic for managing users
 * @help        :: See http://links.sailsjs.org/docs/controllers
 */

module.exports = {

  index:  function(req, res) {
    res.view("management/index");
  },

  workers: function(req, res) {

    Worker.find({sort: {workerID: 0, team: 0, department: 0}}, function(err, workers) {

      if (err) { return next(err); }

      res.view("management/workers", {workers: workers});
    });

  },

  deleteWorker: function(req, res) {

    var objectID = req.param("objectID");

    Worker.destroy({id: objectID}, function(err) {
      res.json("{}");
    });
    
  },

  addWorker: function(req, res) {
    res.view("management/addWorker");
  },

  editWorker: function(req, res) {

    var objectID = req.param("objectID");

    Worker.findOne({id: objectID}, function (err, worker) {

      if (!worker) {
        res.notFound();
        return;
      }

      res.view("management/editWorker", {objectID: objectID, worker: worker});
    });

  },

  editWorkerImpl: function(req, res) {

    var objectID = req.param("objectID");
    var name = req.param("name");
    var workerID = req.param("workerID");
    var department = req.param("department");
    var team = req.param("team");

    function redirectWithErrorMessage(message) {
      var errorMessage = [{name: "reqiured", message: message}]
      req.session.flash = {err: errorMessage};
      res.redirect("/management/editWorker/" + objectID);
    }

    Worker.findOne({id: objectID}, function (err, worker) {

      if (!worker) {
        redirectWithErrorMessage("查無此員工");
        return;
      }

      Worker.findOneByName(name, function (err, worker1) {

        if (err) { return next(err); }

        console.log(worker1);

        if (worker1 && worker1.id != objectID) { 
          redirectWithErrorMessage("員工姓名已經存在。");
          return;
        }

        Worker.findOneByWorkerID(workerID, function(err, worker2) {

          if (worker2 && worker2.id != objectID) { 
            console.log("工號已經存在");
            redirectWithErrorMessage("工號已經存在。");
            return;
          }

          Worker.update({id: objectID}, req.params.all(), function userCreated(err, newUser) {

            if (err) {
              req.session.flash = {err: err}
              res.redirect("/management/editWorker/" + objectID);
              return;
            }

            res.redirect("/management/workers");
          });
        });
      });

    });

  },

  addWorkerImpl: function(req, res) {

    function redirectWithErrorMessage(message) {
      var errorMessage = [{name: "reqiured", message: message}]
      req.session.flash = {err: errorMessage};
      res.redirect("/management/addWorker");
    }

    var name = req.param("name");
    var workerID = req.param("workerID");

    Worker.findOneByName(name, function (err, worker) {


      if (err) { return next(err); }

      if (worker) { 
        redirectWithErrorMessage("員工姓名已經存在。");
        return;
      }

      Worker.findOneByWorkerID(workerID, function(err, worker) {

        if (worker) { 
          console.log("工號已經存在");
          redirectWithErrorMessage("工號已經存在。");
          return;
        }

        Worker.create(req.params.all(), function userCreated(err, newUser) {

          if (err) {
            req.session.flash = {err: err}
            res.redirect("/management/addWorker");
            return;
          }

          res.redirect("/management/workers");
        });
      });
    });

  }

}

