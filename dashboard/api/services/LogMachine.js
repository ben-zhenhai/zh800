exports.jsonAPI = function() {

  function overview(convert, callback) {

    var aggeration = [
      { $group: { _id: "$mach_id", bad_qty: { $sum: "$bad_qty" } } },
      { $sort: {_id: 1}}
    ]

    var converter = function(data) {
      return {
        name: data._id, 
        value: data.bad_qty,
        link: "/machine/" + data._id
      }
    }

    var aggerator = Aggerator.defineOn({
      model: Log,
      aggeration: aggeration,
      converter: converter
    });

    aggerator(callback);
  }

  function detailPie (machineID, callback) {

    var mapReducer = MapReducer.defineOn({
      model: Log,
      groupingFunction: function(data) { return data.defact_id; },
      mongoFilters: {mach_id: machineID},
      converter: function (data) {
        return {
          name: data._id,
          value: data.value
        };
      }
    });

    mapReducer(callback);
  }

  function detailTable (machineID, callback) {

    var sortByDate = function (objA, objB) {
      function strToDate(dateString) {
        var columns = dateString.split("-");
        var year = columns[0];
        var month = +(columns[1]) - 1; // JSDate's month count from 0
        return new Date(+year, +month, 1);
      }

      return strToDate(objA._id.time).getTime() - strToDate(objB._id.time).getTime();
    }

    var mapReducer = MapReducer.defineOn({
      model: Log,
      sorting: sortByDate,
      groupingFunction: function(data) { 
        var month = +(data.emb_date.getMonth()) + 1
        var dateString = 
          data.emb_date.getFullYear() + "-" + month

        return {name: data.mach_id, time: dateString}; 
      },
      mongoFilters: {mach_id: machineID},
      converter: function (data) {
        return {
          name: data._id.name,
          time: data._id.time,
          value: data.value
        };
      }
    });

    mapReducer(callback);
  }

  return {
    overview: overview,
    detailPie: detailPie,
    detailTable: detailTable
  }
}
