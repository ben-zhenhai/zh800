function setTable(selection, tableSchema) {
  var tableBody = d3.select(selection);

  tableBody.selectAll("tr").remove();

  var tableData = tableSchema.data;
  var extractors = tableSchema.extractors;
  var tableRow = tableBody.selectAll("tr").data(tableData).enter().append("tr");

  for (var i = 0; i < extractors.length; i++) {
    tableRow.append("td").text(extractors[i]);
  }
}

function setupMachineList(id, productMachine, urlPrefix) {
  $(id).select2({width: "100%"}).on('change', function(e) {
    window.location = urlPrefix + "/" + e.val;
  });
  $(id).select2('val', productMachine);
}

function sortTable(tableID, totalColumn) {
  $(tableID).stupidtable().bind('aftertablesort', function(event, data) {
    for (var i = 0; i < totalColumn; i++) {
      $('#arrow' + i).attr('class', 'ui sort icon');
    }
    var arrowIcon = data.direction == "asc" ? 'ascending' : 'descending';
    $('#arrow' + data.column).attr('class', 'ui sort ' + arrowIcon + ' icon');
  });
}

