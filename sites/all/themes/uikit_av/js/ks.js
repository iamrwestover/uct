(function ($) {
  Drupal.behaviors.avKeyboardShortcuts = {
    attach: function (context, settings) {
      $('[data-ks]', context).once('avKS', function () {
        var $element = $(this);
        //console.log($element);
        $element.prop('title', '<i class="uk-icon-keyboard-o"></i> ' + $element.data('ks'));
        $element.attr('data-uk-tooltip', "{delay:'500', cls: 'tt-ks'}");
        $(document).bind('keydown', $element.data('ks'), function (e) {
          if ($element.is('a')) {
            $element[0].click();
          }
          else if ($element.is(':visible')) {
            $element.trigger($element.data('ks-trigger') || 'click');
          }
          e.preventDefault();
        });
      });
    }
  };

  // Shortcut to home page.
  $(document).bind('keydown', 'f4', function (e) {
    window.location.href = '/';
    e.preventDefault();
  });

  // Shortcut to first input text or textarea.
  $(document).bind('keydown', 'alt+ctrl+return', function (e) {
    $("#region-content-wrapper input:text:not([data-uk-datepicker]), #region-content-wrapper textarea").eq(0).focus();
    e.preventDefault();
  });
}(jQuery));
