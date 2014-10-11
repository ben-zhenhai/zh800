var cacheTableName = 'cached';
var stats = {};

function getProduct(record) {
  return record.lot_no;
}

function getJSDate(record) {
  return new Date(record.emb_date * 1000);
}

function getDate(record) {
  var jsDate = getJSDate(record);

  function getWeek(isoDate) {
    var date = isoDate.getDate();
    var day = isoDate.getDay();
    return Math.ceil((date - 1 - day) / 7) + 1;
  }

  return {
    year: jsDate.getFullYear(),
    month: +(jsDate.getMonth()) + 1,
    week: getWeek(jsDate),
    date: jsDate.getDate()
  }
}

function getTotalURLs(record) {
  var date = getDate(record);

  return [
    "total", getProduct(record) //, date.year, date.month, date.week, date.date, record.mach_id
  ];
}

function getMonthlyURLs(record) {
  var date = getDate(record);

  return [
    "monthly", date.year, date.month, date.week, date.date, record.mach_id
  ];
}

function getDailyURLs(record) {
  var date = getDate(record);

  return [
    "daily", date.year, date.month, date.date, record.mach_id
  ];
}

function getReasonURLs(record) {
  var date = getDate(record);
  var detail = date.year + "-" + date.month + " " + record.mach_id;

  return [
    "reason", record.defact_id, detail
  ];
}

function getMachineURLs(record) {
  var date = getDate(record);
  var detail = date.year + "-" + date.month + " " + record.defact_id;

  return [
    "machine", record.mach_id, detail
  ];
}

function updateMaxTime(record) {

  var dateInRecord = new Date(record.emb_date * 1000);
  var maxTime = stats['maxTime'] ? stats['maxTime'] : dateInRecord;

  if (maxTime.getTime() < dateInRecord.getTime()) {
    maxTime = dateInRecord;
  }

  stats['maxTime'] = maxTime;
}

function updateMinTime(record) {

  var dateInRecord = new Date(record.emb_date * 1000);
  var minTime = stats['minTime'] ? stats['minTime'] : dateInRecord;

  if (minTime.getTime() > dateInRecord.getTime()) {
    minTime = dateInRecord;
  }

  stats['minTime'] = minTime;
}

function updateStats(urlComponets, previousURL, level, record) {

  if (urlComponets.length == level + 1) {
    return;
  }

  var url = previousURL + "/" + urlComponets[level];
  var title = (urlComponets[level+1] + "").replace(".", "__DOT__"); // Mongo cannot save field name with dot (.)

  var cachedData = stats[url] ? stats[url] : {};
  var data = cachedData[title] ? cachedData[title] : {bad_qty: +0, count_qty: +0};

  cachedData[title] = { 
    bad_qty: data.bad_qty + (+record.bad_qty),
    count_qty: data.count_qty + (+record.count_qty)
  }

  stats[url] = cachedData;
  updateStats(urlComponets, url, level + 1, record);
}


function addToCache(mongoDB, record) {
  var recordDate = getDate(record);
  var cacheTable = mongoDB.collection(cacheTableName)
  var dailyTableName = recordDate.year + "-" + recordDate.month + "-" + recordDate.date;
  var dailyTable = mongoDB.collection(dailyTableName);
  updateMaxTime(record);
  updateMinTime(record);

  var totalURLComponets = getTotalURLs(record);
  var monthlyURLComponets = getMonthlyURLs(record);
  var dailyURLComponets = getDailyURLs(record);
  var reasonURLComponets = getReasonURLs(record);
  var machineURLComponets = getMachineURLs(record);

  dailyTable.insert(record, function(err, record) {});

  updateStats(totalURLComponets, "", 0, record);
  updateStats(monthlyURLComponets, "", 0, record);
  updateStats(dailyURLComponets, "", 0, record);
  updateStats(reasonURLComponets, "", 0, record);
  updateStats(machineURLComponets, "", 0, record);
}

function saveCache(mongoDB) {
  var cacheTable = mongoDB.collection(cacheTableName)

  for (var url in stats) {
    if (stats.hasOwnProperty(url)) {
      cacheTable.update(
        {url: url}, {url: url, value: stats[url]}, 
        {upsert: true}, function(err, data){
          if (err) {
            console.log("save error:" + err);
            return;
          }
        }
      );
    }
  }
}

function initCache(mongoURL, callback) {

  var mongoClient = require('mongodb').MongoClient
  
  mongoClient.connect(mongoURL, function(err, mongoDB) {
  
    if (err) {
      console.log("Cannot cannto to mongoDB:" + err);
      return;
    }

    var cacheTable = mongoDB.collection(cacheTableName)
    cacheTable.find({}).toArray(function(err, docs) {
      for (var i = 0; i < docs.length; i++) {
        var url = docs[i].url;
        var value = docs[i].value;
        stats[url] = value;
      }
      callback(mongoDB);
    });
 
  });
}

module.exports = {
  addToCache: addToCache,
  saveCache: saveCache,
  initCache: initCache
}
