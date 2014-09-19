/**
 *  Create a function that colud draw a bar chart.
 *
 *  This function will return another function that could draw a pie chart
 *  with siganature of function (selection, dataSet, onClickCallback), where
 *  selection, dataSet is required and onClickCallback is optional.
 *
 *  onClickCallback should have the signature of function(data), where data
 *  will be the data from clicked bar.
 *
 *  The bar chart will bind to the node that selected by selection parameter,
 *  which should be a valid d3 / jquery selector.
 *
 *  The options parameter will control the basic attribute of this chart, and
 *  must have the following attribute:
 *
 *   - options.totalHeight
 *   - options.barWidth
 *   - options.barPadding
 *   - options.bottomMargin
 *   - options.topMargin
 *   - options.extractValue
 *   - options.extractName
 *
 */
function barChart(options) {

  function assertInt(variable, message) {
    if (typeof variable !== "number") {
      var errorMessage = message || "Assertion failed";
      if (typeof Error !== "undefined") {
        throw new Error(errorMessage);
      } else {
        throw message;
      }
    }
  }        

  assertInt(options.totalHeight, "No totalHeight attribute");
  assertInt(options.barWidth, "No barWidth attribute");
  assertInt(options.barPadding, "No barPadding attribute");
  assertInt(options.bottomMargin, "No bottomMargin attribute");
  assertInt(options.topMargin, "No topMargin attribute");

  var draw = function (selection, dataSet, onClickCallback) {

    function getXOffset(index) {
      return index * options.barWidth + index * options.barPadding;
    }

    function getDataValue(data) { 
      return options.extractValue(data); 
    }

    function calculateRectYPosition(data) {
      return options.totalHeight - scalar(getDataValue(data)) - options.bottomMargin-10;
    }

    function calculateTopLabelYPosition(data) {
      return options.totalHeight - scalar(getDataValue(data)) - options.topMargin;
    }

    var totalWidth = getXOffset(dataSet.length);
    var minimalY = options.bottomMargin;
    var maximalY = options.totalHeight - 
  		 (options.bottomMargin + options.topMargin + 10);

    var scalar = d3.scale.linear().
                    domain([0, d3.max(dataSet, options.extractValue)]).
                    range([minimalY, maximalY]);

    var chart = d3.select(selection).
                   attr("width", totalWidth).
                   attr("height", options.totalHeight);

    // Remove all existing content, so we could draw different
    // bar chart on same HTML element.
    chart.selectAll("g").remove();

    var bar = chart.selectAll("g").data(dataSet).enter().
                    append("g")

    // Bar rectangular
    bar.append("rect").
        attr("width", options.barWidth - 5).
        attr("height", function(d) { return scalar(getDataValue(d)) }).
        attr("x", function(d, i) { return getXOffset(i) }).
        attr("y", calculateRectYPosition)

    // The top label, which is the actually data value
    bar.append("text").
        attr("x", function(d, i) { return getXOffset(i) }).
        attr("y", calculateTopLabelYPosition).
        attr("dy", "-15px").
        attr("dx", "20px").
        text(options.extractValue)

    // The bottom label, the name of current data
    bar.append("text").
        attr("x", function(d, i) { return getXOffset(i) }).
        attr("y", options.totalHeight-10).
        attr("dx", "25px").
        text(options.extractName)

    if ($.isFunction(onClickCallback)) {
      bar.on("click", onClickCallback);
    }
  }

  return draw;
}


function pieChart(options) {

  function assertInt(variable, message) {
    if (typeof variable !== "number") {
      var errorMessage = message || "Assertion failed";
      if (typeof Error !== "undefined") {
        throw new Error(errorMessage);
      } else {
        throw message;
      }
    }
  }        

  assertInt(options.width, "No width attribute");
  assertInt(options.height, "No height attribute");
  assertInt(options.radius, "No radius");

  var draw = function (selection, data) {

    function calculateArcCenter(data) {
      data.innerRadius = 0;
      data.outerRadius = options.radius;
      return "translate(" + arc.centroid(data) + ")";
    }

    var color = d3.scale.category20c();

    var pieData = d3.layout.pie().value(options.extractValue)(data);


    var pieChart = d3.select(selection)
        .attr("width", options.width)
        .attr("height", options.height)
        .append("g")
        .attr("transform", "translate(" + options.radius + "," + options.radius + ")")

    var arc = d3.svg.arc().outerRadius(options.radius);

    var arcs = pieChart.selectAll("g.slice")
        .data(pieData)
        .enter()
        .append("g")
        .attr("class", "slice")
        .attr("onclick", function(d) { 
          if (d.data.link) {
            return "window.location='" + d.data.link + "'"; 
          } else {
            return "void(0);"
          }
        });

    arcs.append("path")
      .attr("fill", function(d, i) { return color(i); } )
      .attr("d", arc);

    arcs.append("text")
        .attr("transform", calculateArcCenter)
        .attr("text-anchor", "middle")
        .text(function(d,i) { return d.data.label; });
  }

  return draw;
}

