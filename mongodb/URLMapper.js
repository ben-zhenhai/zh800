var cacheTableName = 'cached';

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

function getURLs(record) {
  var date = getDate(record);

  return [
    "/total", 
    "/total/" + getProduct(record), 
    "/total/" + getProduct(record) + "/" + date.year,
    "/total/" + getProduct(record) + "/" + date.year + "/" + date.month,
    "/total/" + getProduct(record) + "/" + date.year + "/" + date.month + "/" + date.week,
    "/total/" + getProduct(record) + "/" + date.year + "/" + date.month + "/" + date.week + "/" + date.date,
    "/total/" + getProduct(record) + "/" + date.year + "/" + date.month + "/" + date.week + "/" + date.date + "/" + record.mach_id
  ];
}

function updateMaxTime(cacheTable, record) {

  function dummyCallback(err, result) { }

  cacheTable.find({url: "maxTime"}).toArray(function(err, docs) {

    var dateInRecord = getJSDate(record)

    if (err) { 
      console.log("database error:" + err);
      return;
    }

    if (docs.length == 0) {
      cacheTable.insert({url: "maxTime", value: dateInRecord}, dummyCallback);
    } else {
      if (docs[0].value.getTime() < dateInRecord.getTime()) {
        console.log("update maxTime to " + dateInRecord);
        cacheTable.update({url: "maxTime"}, {$set: {value: dateInRecord}}, dummyCallback);
      }
    }

  });
}

function updateMinTime(cacheTable, record) {

  function dummyCallback(err, result) { }

  cacheTable.find({url: "minTime"}).toArray(function(err, docs) {

    var dateInRecord = getJSDate(record)

    if (err) { 
      console.log("database error:" + err);
      return;
    }

    if (docs.length == 0) {
      cacheTable.insert({url: "minTime", value: dateInRecord}, dummyCallback);
    } else {
      if (docs[0].value.getTime() > dateInRecord.getTime()) {
        cacheTable.update({url: "minTime"}, {$set: {value: dateInRecord}}, dummyCallback);
      }
    }
  });
}


function addToCache(mongoDB, record) {
  console.log("Add record to cache...");

  var cacheTable = mongoDB.collection(cacheTableName)

  updateMaxTime(cacheTable, record);
  updateMinTime(cacheTable, record);
}

module.exports = {
  addToCache: addToCache
}
