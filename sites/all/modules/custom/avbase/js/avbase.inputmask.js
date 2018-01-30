
(function ($) {

  Drupal.behaviors.avbaseInputMask = {
    attach: function (context, settings) {
      $(':input[data-inputmask]', context).once('avbaseInputMask', function () {
        $(this).inputmask();
      });
    }
  };

}(jQuery));
