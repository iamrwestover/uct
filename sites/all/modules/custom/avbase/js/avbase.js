(function ($) {

  if (Drupal.ajax) {
    // Redirect user accordingly.
    Drupal.ajax.prototype.commands.redirectUser = function (ajax, response) {
      window.location = response.path;
    };
  }

  Drupal.behaviors.avbaseGeneral = {
    attach: function (context, settings) {
      $('#av-print-btn', context).once('avbaseGeneral', function () {
        $(this).click(function(e) {
          window.print();
          e.preventDefault();
        });
      });

      // Auto select all text when an autocomplete textfield gets focused.
      $('.form-autocomplete:text', context).once('avbaseGeneral', function () {
        $(this).focus(function() {
          $(this).select();
        });
      });

      // Enable full row select on necessary tables.
      //$('table.av-table-tableselect tr', context).once('avbaseGeneral', function () {
      //  $(this).find
      //});
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

