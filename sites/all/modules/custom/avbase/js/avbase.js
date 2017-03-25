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

  var $form = $('form');

  // Prevent form submission on keyboard enter.
  $('#avtxns-txn-form').on('keyup keypress', function(e) {
    var keyCode = e.keyCode || e.which;
    if (keyCode === 13) {
      e.preventDefault();
      return false;
    }
  });
  // Prevent double submission of forms.
  $form.submit(function(e) {
    if( $(this).hasClass('form-submitted') ){
      e.preventDefault();
      return;
    }

    $(this).addClass('form-submitted');
  });
  // Automatically print if url parameter is specified.
  var print = avbaseGetUrlParameter('print');
  if (print) {
    $('#av-print-btn').trigger('click');
  }
});

var avbaseGetUrlParameter = function getUrlParameter(sParam) {
  var sPageURL = decodeURIComponent(window.location.search.substring(1)),
          sURLVariables = sPageURL.split('&'),
          sParameterName,
          i;

  for (i = 0; i < sURLVariables.length; i++) {
    sParameterName = sURLVariables[i].split('=');

    if (sParameterName[0] === sParam) {
      return sParameterName[1] === undefined ? true : sParameterName[1];
    }
  }
};
