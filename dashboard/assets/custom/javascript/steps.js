
function resetSteps() {

  $(".steps div").removeClass("active");

  $(".steps div").each(function(index, div) { 
    var defaultName = $(div).attr("data-defaultName");
    $(div).text(defaultName);
  })

}

function setStepsTitle(idPrefix, titles) {
  for (var i = 0; i < titles.length; i++) {
    var stepNode = "#" + idPrefix + (i + 1);
    $(stepNode).text(titles[i]);
  }
}

