
function setStepsTitle(idPrefix, titles) {

  function resetSteps() {
    $(".steps div").removeClass("active");
    $(".steps div").each(function(index, div) { 
      var defaultName = $(div).attr("data-defaultName");
      $(div).text(defaultName);
    })
  }

  resetSteps();

  // Active first item.
  $("#" + idPrefix + "0").addClass('active');

  for (var i = 0; i < titles.length; i++) {
    var stepNode = "#" + idPrefix + (i + 1);
    $(stepNode).addClass('active');
    $(stepNode).text(titles[i]);
  }

}

