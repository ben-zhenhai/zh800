var MongoClient = require('mongodb').MongoClient;

var offsetOf7Hours = 7 * 60 * 60 * 1000
var originalDate = new Date();
var date = new Date(originalDate.getTime() - offsetOf7Hours);
var year = date.getFullYear();
var month = date.getMonth() + 1;
month = (month < 10 ? "0" : "") + month;
var day = date.getDate();
day = (day < 10 ? "0" : "") + day;
var today = year + "-" + month + "-" + day;
console.log("today:" + today);

const BIG = "16 - 18";
const MEDIUM = "10 - 12.5";
const SMALL = "5 -8";

const TYPE_E = 1;
const TYPE_G = 2;
const TYPE_A = 3;
const TYPE_A4 = 4;
const TYPE_TC = 5;

/*
var BIG_E = 0;
var BIG_G = 0;
var BIG_A = 0;
var BIG_A4 = 0;
var BIG_TC = 0;

var MEDIUM_E = 0;
var MEDIUM_G = 0;
var MEDIUM_A = 0;
var MEDIUM_A4 = 0;
var MEDIUM_TC = 0;

var SMALL_E = 0;
var SMALL_G = 0;
var SMALL_A = 0;
var SMALL_A4 = 0;
var SMALL_TC = 0;
*/

function queryDatabase() {
MongoClient.connect("mongodb://localhost:27017/zhenhai", function(err, db) {
  if(err) { return console.dir(err); }

  var collection = db.collection('daily');
  
  var BIG_E = 0;
  var BIG_G = 0;
  var BIG_A = 0;
  var BIG_A4 = 0;
  var BIG_TC = 0;

  var MEDIUM_E = 0;
  var MEDIUM_G = 0;
  var MEDIUM_A = 0;
  var MEDIUM_A4 = 0;
  var MEDIUM_TC = 0;

  var SMALL_E = 0;
  var SMALL_G = 0;
  var SMALL_A = 0;
  var SMALL_A4 = 0;
  var SMALL_TC = 0;

  var data = {};
/*
  collection.find({"shiftDate":today}).toArray(function(err, items) {
    console.log(items);
  });
*/
/*
  var stream = collection.find({"capacityRange":BIG,"machineType":1,"shiftDate":today}).stream();
  stream.on("data", function(item) {
   console.log(item.count_qty);
  });
*/
  var stream = collection.find({"shiftDate":today}).stream();
  stream.on("data", function(item) {
    //console.log(item.mach_id);
    if (item.capacityRange === BIG && item.machineType === TYPE_E) {
      //console.log("big e:" + item.count_qty);
      BIG_E += item.count_qty;
    } else if (item.capacityRange === BIG && item.machineType === TYPE_G) {
      //console.log("big g:" + item.count_qty);
      BIG_G += item.count_qty;
    } else if (item.capacityRange === BIG && item.machineType === TYPE_A) {
      //console.log("big a:" + item.count_qty);
      BIG_A += item.count_qty;
    } else if (item.capacityRange === BIG && item.machineType === TYPE_A4) {
      //console.log("big a4:" + item.count_qty);
      BIG_A4 += item.count_qty;
    } else if (item.capacityRange === BIG && item.machineType === TYPE_TC) {
      //console.log("big tc:" + item.count_qty);
      BIG_TC += item.count_qty;
    } else if (item.capacityRange === MEDIUM && item.machineType === TYPE_E) {
      MEDIUM_E += item.count_qty;
    } else if (item.capacityRange === MEDIUM && item.machineType === TYPE_G) {
      MEDIUM_G += item.count_qty;
    } else if (item.capacityRange === MEDIUM && item.machineType === TYPE_A) {
      MEDIUM_A += item.count_qty;
    } else if (item.capacityRange === MEDIUM && item.machineType === TYPE_A4) {
      MEDIUM_A4 += item.count_qty;
    } else if (item.capacityRange === MEDIUM && item.machineType === TYPE_TC) {
      MEDIUM_TC += item.count_qty;
    } else if (item.capacityRange === SMALL && item.machineType === TYPE_E) {
      SMALL_E += item.count_qty;
    } else if (item.capacityRange === SMALL && item.machineType === TYPE_G) {
      SMALL_G += item.count_qty;
    } else if (item.capacityRange === SMALL && item.machineType === TYPE_A) {
      SMALL_A += item.count_qty;
    } else if (item.capacityRange === SMALL && item.machineType === TYPE_A4) {
      SMALL += item.count_qty;
    } else if (item.capacityRange === SMALL && item.machineType === TYPE_TC) {
      console.log("small tc:" + item.count_qty);
      SMALL_TC += item.count_qty;
    }
  });

  stream.on("end", function() {
    db.close();

    data["BIG_E"] = BIG_E;
    data["BIG_G"] = BIG_G;
    data["BIG_A"] = BIG_A;
    data["BIG_A4"] = BIG_A4;
    data["BIG_TC"] = BIG_TC;

    data["MEDIUM_E"] = MEDIUM_E;
    data["MEDIUM_G"] = MEDIUM_G;
    data["MEDIUM_A"] = MEDIUM_A;
    data["MEDIUM_A4"] = MEDIUM_A4;
    data["MEDIUM_TC"] = MEDIUM_TC;

    data["SMALL_E"] = SMALL_E;
    data["SMALL_G"] = SMALL_G;
    data["SMALL_A"] = SMALL_A;
    data["SMALL_A4"] = SMALL_A4;
    data["SMALL_TC"] = SMALL_TC;
    console.log("---------------------------------------------------");
    console.log(BIG_E);
    console.log(BIG_G);
    console.log(BIG_A);
    console.log(BIG_A4);
    console.log(BIG_TC);

    console.log(MEDIUM_E);
    console.log(MEDIUM_G);
    console.log(MEDIUM_A);
    console.log(MEDIUM_A4);
    console.log(MEDIUM_TC);

    console.log(SMALL_E);
    console.log(SMALL_G);
    console.log(SMALL_A);
    console.log(SMALL_A4);
    console.log(SMALL_TC);
    console.log("---------------------------------------------------");
    process.send(data);
  });
});
}

process.on("message", function(msg) {
  console.log("dailyCount.js get process msg: "+ msg);
  if (msg === 'init') {
    queryDatabase();
  }
});

setInterval(queryDatabase, 420000);
//setInterval(queryDatabase, 3600000);
