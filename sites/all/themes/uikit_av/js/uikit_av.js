(function ($) {
  var $loader = $('#loading'), ajaxTimer;
  //console.log('loading');
  //var avLoadingTimeout = setTimeout (function() {
  //  console.log('pagein timeout reached.');
  //  $('#loading-center').fadeIn();
  //}, 3000);
  //
  //$(window).on('load', function() {
  //  clearTimeout(avLoadingTimeout);
  //  $('#loading').delay(500).fadeOut('fast');
  //});
  //
  $(window).on('beforeunload', function(e) {
    if (window.location.pathname == '/batch') {
      // e = e || window.event;
      // var message = 'Scan is running. Are you sure you want to leave?';
      // // For IE and Firefox prior to version 4
      // if (e) {
      //   e.returnValue = message;
      // }
      // return message;
    }
    else {
      $('#loading').fadeIn('fast');
    }
  });



  if (window.location.pathname != '/batch') {
    $(document).ajaxStart(function () {
      ajaxTimer && clearTimeout(ajaxTimer);
      ajaxTimer = setTimeout(function () {
        $loader.fadeIn('fast');
      }, 1000);
    }).ajaxStop(function () {
      clearTimeout(ajaxTimer);
      $loader.fadeOut('fast');
    });
  }



}(jQuery));

jQuery(document).ready(function ($) {
  //$('form').dirtyForms();
  //$(document).bind('proceed.dirtyforms', function() {
  //  console.log('xr');
  //});
  //console.log('srd');

  // Toggle offcanvas menu.
  var $panel = $('#menu-toggle');
  var offcanvasTimeout;
  $panel.find('.uk-offcanvas-bar').hover(function () {
    clearTimeout(offcanvasTimeout);
    if ($panel.hasClass('offcanvas-expanded')) {
      return;
    }
    offcanvasTimeout = setTimeout (function() {
      UIkit.offcanvas.show('#menu-toggle', {mode: 'reveal'});
      $panel.removeClass('offcanvas-collapsed');
      $panel.addClass('offcanvas-expanded');
    }, 3000);
  }, function () {
    clearTimeout(offcanvasTimeout);
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

