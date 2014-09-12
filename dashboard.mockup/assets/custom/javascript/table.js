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


