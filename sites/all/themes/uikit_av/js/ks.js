(function ($) {
  Drupal.behaviors.avKeyboardShortcuts = {
    attach: function (context, settings) {
      $('[data-ks]', context).once('avKS', function () {
        var $element = $(this);
        //console.log($element);
        $element.prop('title', '<i class="uk-icon-keyboard-o"></i> ' + $element.data('ks'));
        $element.attr('data-uk-tooltip', "{delay:'500', cls: 'tt-ks'}");
        $(document).bind('keydown', $element.data('ks'), function (e) {
          if ($element.is(':visible')) {
            $element.trigger($element.data('ks-trigger') || 'click');
          }
          e.preventDefault();
        });
      });
    }
  };

  $(document).bind('keydown', 'f4', function (e) {
    window.location.href = '/';
    e.preventDefault();
  });
}(jQuery));
//
//jQuery(document).ready(function ($) {
//
//  $('[data-ks]').each(function (i, element) {
//    var $element = $(element);
//    $element.prop('title', '<i class="uk-icon-keyboard-o"></i> ' + $element.data('ks'));
//    $element.attr('data-uk-tooltip', "{delay:'500', cls: 'tt-ks'}");
//    $(document).bind('keydown', $element.data('ks'), function (e) {
//      $element.trigger('click');
//      e.preventDefault();
//    });
//  });
//});