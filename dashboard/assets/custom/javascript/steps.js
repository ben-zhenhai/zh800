
function resetSteps() {

  $(".steps div").removeClass("active");

  $(".steps div").each(function(index, div) { 
    var defaultName = $(div).attr("data-defaultName");
    $(div).text(defaultName);
  })

}

function setStepsTitle(idPrefix, titles, shouldSelectLast) {
  resetSteps();

  $("#" + idPrefix + "0").addClass('active');

  for (var i = 0; i < titles.length; i++) {
    var stepNode = "#" + idPrefix + (i + 1);
    $(stepNode).addClass('active');
    $(stepNode).text(titles[i]);
  }

  if (shouldSelectLast) {
    var lastStepNode = '#' + idPrefix + (titles.length + 1);
    $(lastStepNode).addClass("active");
  }

}

