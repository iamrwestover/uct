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

  Drupal.behaviors.avbaseGeneral = {
    attach: function (context, settings) {
      var arr = [];
      // arr.push({url: Drupal.settings.basePath + 'sites/all/themes/uikit_av/images/a.mp4', isVideo: true, loop: true});
      // arr.push({url: Drupal.settings.basePath + 'sites/all/themes/uikit_av/images/b.mp4', isVideo: true, loop: true});
      for (var i = 1; i <= 12; i++) {
        arr.push(Drupal.settings.basePath + 'sites/all/themes/uikit_av/images/backstretch/' + i + '.jpg');
      }
      arr = shuffle(arr);

      $('body').once('backstretch', function () {
        $(this).backstretch(arr, {duration: 15000, transitionDuration: 500, transition: 'fade'});
      });

    }
  };
}(jQuery));
