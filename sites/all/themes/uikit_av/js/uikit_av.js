(function ($) {
  // Here we immediately call the function with jQuery as the parameter.
}(jQuery));

jQuery(document).ready(function ($) {
  // Toggle offcanvas menu.
  var $panel = $('#menu-toggle');
  $panel.find('.uk-offcanvas-bar').hover(function () {
    if ($panel.hasClass('offcanvas-expanded')) {
      return;
    }
    UIkit.offcanvas.show('#menu-toggle', {mode: 'reveal'});
    $panel.removeClass('offcanvas-collapsed');
    $panel.addClass('offcanvas-expanded');
  }, function () {
    if ($panel.hasClass('offcanvas-collapsed')) {
      return;
    }
    UIkit.offcanvas.hide();
    $panel.removeClass('offcanvas-expanded');
    $panel.addClass('offcanvas-collapsed');
  });
});

jQuery(document).load(function ($) {
  // Here we will call the function with jQuery as the parameter once the entire
  // page (images or iframes), not just the DOM, is ready.
});

