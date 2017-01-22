(function ($) {

  if (Drupal.ajax) {
    // Redirect user accordingly.
    Drupal.ajax.prototype.commands.redirectUser = function (ajax, response) {
      window.location = response.path;
    };
  }

  Drupal.behaviors.avbaseGeneral = {
    attach: function (context, settings) {
      $('#av-print-btn', context).once('avbasePrintButton', function () {
        $(this).click(function(e) {
          window.print();
          e.preventDefault();
        });
      });
    }
  };
}(jQuery));


jQuery(document).ready(function ($) {

  // Prevent double submission of forms.
  $('form').submit(function(e) {
    if( $(this).hasClass('form-submitted') ){
      e.preventDefault();
      return;
    }

    $(this).addClass('form-submitted');
  });




});

