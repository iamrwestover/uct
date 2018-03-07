(function ($) {
  function shuffle(array) {
    var currentIndex = array.length, temporaryValue, randomIndex;

    // While there remain elements to shuffle...
    while (0 !== currentIndex) {

      // Pick a remaining element...
      randomIndex = Math.floor(Math.random() * currentIndex);
      currentIndex -= 1;

      // And swap it with the current element.
      temporaryValue = array[currentIndex];
      array[currentIndex] = array[randomIndex];
      array[randomIndex] = temporaryValue;
    }

    return array;
  }

  var arr = [];
  for (var i = 1; i <= 11; i++) {
    arr.push('/sites/all/themes/uikit_av/images/' + i + '.jpg');
  }
  arr = shuffle(arr);

  $.backstretch(arr, {duration: 10000, transitionDuration: 500, transition: 'fade'});
}(jQuery));
