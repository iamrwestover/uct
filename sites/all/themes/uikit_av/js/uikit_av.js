(function ($) {
  //console.log('loading');
  //var avLoadingTimeout = setTimeout (function() {
  //  console.log('pagein timeout reached.');
  //  $('#loading-center').fadeIn();
  //}, 1000);
  //
  $(window).on('load', function() {
    //console.log('page loaded');
    //clearTimeout(avLoadingTimeout);
    $('#loading').delay(0).fadeOut('fast');
  });

  $(window).on('beforeunload', function() {
    //console.log('unloading');
    $('#loading').fadeIn('fast');
    setTimeout (function() {
      //console.log('pageout timeout reached.');
      $('#loading-center').fadeIn()
    }, 500);
  });
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

//jQuery(window).load(function ($) {
//  console.log('ff');
//  $('#status').fadeOut(); // will first fade out the loading animation
//  $('#preloader').delay(350).fadeOut('slow'); // will fade out the white DIV that covers the website.
//  $('body').delay(350).css({'overflow':'visible'});
//});

