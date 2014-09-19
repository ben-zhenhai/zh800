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
    window.location = urlPrefix + "/" + productMachine;
  });
  $(id).select2('val', productMachine);
}

