/**
 * UserController
 *
 * @description :: Server-side logic for managing users
 * @help        :: See http://links.sailsjs.org/docs/controllers
 */

module.exports = {

  index: function(req, res) {
    res.view("barcode/index");
  },

  view: function(req, res) {
    var orderType = req.param("orderType")
    var lotNumber = req.param("lotNumber")
    var workQty = req.param("workQty")
    var machineID = req.param("machineID")
    var workerID = req.param("workerID")
    var barcode = require('barcode');
    var barcodeData = orderType + " " + lotNumber + " " + workQty + " " + machineID + " " + workerID
    console.log("barcodeData:" + barcodeData);
    var code128 = barcode('code128', {
      data: barcodeData,
      width: 300,
      height: 50,
    });
    code128.getBase64(function (err, barcodeImage) {
      if (err) {
        res.next(err);
      }
      res.view("barcode/view", {barcodeImage: barcodeImage, barcodeData: barcodeData});
    });
  }

};

