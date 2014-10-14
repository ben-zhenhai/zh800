var cacheTableName = 'cached';
var stats = {};

function paddingZero(number) {
  if (number < 10) {
    return "0" + number;
  } else {
    return number;
  }
}

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
    date: jsDate.getDate(),
    hour: jsDate.getHours(),
    minute: jsDate.getMinutes()
  }
}

function getTotalURLs(record) {
  var date = getDate(record);
  var dateMonth = (+date.year) + "-" + paddingZero(+date.month);

  return [
    "total", getProduct(record), dateMonth, date.week, date.date, record.mach_id
  ];
}

function getMonthlyURLs(record) {
  var date = getDate(record);

  return [
    "monthly", date.year, paddingZero(date.month), date.week, paddingZero(date.date), record.mach_id
  ];
}

function getDailyURLs(record) {
  var date = getDate(record);

  return [
    "daily", date.year, paddingZero(date.month), paddingZero(date.date), record.mach_id
  ];
}

function getReasonURLs(record) {
  var date = getDate(record);
  var detail = date.year + "-" + paddingZero(date.month) + "-" + paddingZero(date.date) + " " + record.mach_id;

  return [
    "reason", record.defact_id, detail
  ];
}

function getMachineURLs(record) {
  var date = getDate(record);
  var detail = date.year + "-" + paddingZero(date.month) + "-" + paddingZero(date.date) + " " + record.defact_id;

  return [
    "machine", record.mach_id, detail
  ];
}

function updateMaxTime(record) {

  var dateInRecord = record.emb_date;
  var maxTime = stats['maxTime'] ? stats['maxTime'] : dateInRecord;

  if (maxTime < dateInRecord) {
    maxTime = dateInRecord;
  }

  stats['maxTime'] = maxTime;
}

function updateMinTime(record) {

  var dateInRecord = record.emb_date;
  var minTime = stats['minTime'] ? stats['minTime'] : dateInRecord;

  if (minTime > dateInRecord) {
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

function insertToDailyTable(mongoDB, record) {

  var recordDate = getDate(record);
  var dailyTableName = recordDate.year + "-" + paddingZero(recordDate.month) + "-" + paddingZero(recordDate.date);
  var dailyTable = mongoDB.collection(dailyTableName);

  var timestamp = recordDate.year + "-" + recordDate.month + "-" + recordDate.date + " " + paddingZero(+recordDate.hour) + ":" + paddingZero(+recordDate.minute)
  var errorKind = record.defact_id;
  var lotNo = record.lot_no;
  var machineID = record.mach_id;

  var query = {timestamp: timestamp, defact_id: errorKind, lot_no: lotNo, mach_id: machineID};

  dailyTable.findOne(query, function(err, data) {
    
    var newRecord = {
      timestamp: timestamp, defact_id: errorKind, lot_no: lotNo, mach_id: machineID, 
      bad_qty: +record.bad_qty, count_qty: +record.count_qty
    }

    if (!data) {
      dailyTable.insert(newRecord, function(err, data) {})
    } else {
      newRecord.bad_qty = newRecord.bad_qty + data.bad_qty;
      newRecord.count_qty = newRecord.count_qty + data.count_qty;

      // console.log(query);
      // console.log(newRecord.bad_qty + " / " + newRecord.count_qty);

      dailyTable.update(query, newRecord,function(err, record) {});
    }
  });

}

function addToCache(mongoDB, record) {

  updateMaxTime(record);
  updateMinTime(record);

  var totalURLComponets = getTotalURLs(record);
  var monthlyURLComponets = getMonthlyURLs(record);
  var dailyURLComponets = getDailyURLs(record);
  var reasonURLComponets = getReasonURLs(record);
  var machineURLComponets = getMachineURLs(record);

  updateStats(totalURLComponets, "", 0, record);
  updateStats(monthlyURLComponets, "", 0, record);
  updateStats(dailyURLComponets, "", 0, record);
  updateStats(reasonURLComponets, "", 0, record);
  updateStats(machineURLComponets, "", 0, record);

  insertToDailyTable(mongoDB, record);
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
