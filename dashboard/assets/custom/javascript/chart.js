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

    function getDataValue(data) { return options.extractValue(data); }

    var totalWidth = getXOffset(dataSet.length);
    var minimalY = options.bottomMargin;
    var maximalY = options.totalHeight - 
  		 (options.bottomMargin + options.topMargin);

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

    bar.append("rect").
        attr("width", options.barWidth - 5).
        attr("height", function(d) { return scalar(getDataValue(d)) }).
        attr("x", function(d, i) { return getXOffset(i) }).
        attr("y", function(d) { return options.totalHeight - scalar(getDataValue(d)) - options.bottomMargin })

    if ($.isFunction(onClickCallback)) {
      bar.on("click", onClickCallback);
    }

    bar.append("text").
        attr("x", function(d, i) { return getXOffset(i) }).
        attr("y", function(d, i) { return options.totalHeight - scalar(getDataValue(d)) - options.topMargin} ).
        attr("dy", "-0.5em").
        attr("dx", "20px").
        text(options.extractValue)

    bar.append("text").
        attr("x", function(d, i) { return getXOffset(i) }).
        attr("y", options.totalHeight).
        attr("dx", "25px").
        text(function(d) { return d.name } )

  }

  return draw;
}

