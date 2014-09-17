exports.jsonAPI = function() {

  function overview (year, month, callback) {

    var startDate = new Date(year, (+month)-1, 1);
    var endDate = new Date(year, +(month), 1);

    var mapReducer = MapReducer.defineOn({
      model: Log,
      mongoFilters: {
        emb_date: {$gte: startDate, $lt: endDate}
      },
      groupingFunction: function (data) { 
        return data.emb_date.getDate();
      },
      converter: function (data) {
        return {
          name: data._id + " 日", 
          value: data.value,
          link: "/daily/" + year + "/" + month + "/" + data._id
        }
      }
    });

    mapReducer(callback);
  }

  function yearMonthDate (year, month, date, callback) {

    var startDate = new Date(year, (+month)-1, date);
    var endDate = new Date(year, +(month)-1, date+1);

    var mapReducer = MapReducer.defineOn({
      model: Log,
      mongoFilters: {
        emb_date: {$gte: startDate, $lt: endDate}
      },
      groupingFunction: function (data) { 
        return data.mach_id;
      },
      converter: function (data) {
        return {
          name: data._id,  
          value: data.value,
          link: "/daily/" + year + "/" + month + "/" + date + "/" + data._id
        }
      }
    });

    mapReducer(callback);
  }

  function machineDetail (year, month, date, machine, callback) {

    var startDate = new Date(+year, +(month-1), +date);
    var endDate = new Date(+year, +(month-1), (+date) + 1);

    var mapReducer = MapReducer.defineOn({
      model: Log,
      groupingFunction: function(data) {
        return {date: data.emb_date, error: data.defact_id};
      },
      mongoFilters: {
        mach_id: machine,
        emb_date: {$gte: startDate, $lt: endDate}
      },
      converter: function (data) {
        return {
          name: data._id,
          value: data.value,
        };
      }
    });

    mapReducer(callback);
  }

  return {
    overview: overview,
    yearMonthDate: yearMonthDate,
    machineDetail: machineDetail
  }
}
