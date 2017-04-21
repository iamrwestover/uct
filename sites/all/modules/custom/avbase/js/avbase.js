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

      // Auto scroll
      //console.log('z');
      $('#avtxns-txn-form-wrapper').find('#avstatus-messages').once('avbaseGeneral', function () {
        $("html, body").animate({ scrollTop: 0 }, "fast");
        //window.scrollTo(0, 0);
      });

      // Autoclick checkbox/radio when clicking on a table tr.
      $('.av-frs tbody tr').once('avbaseFRS', function() {
        var $input = $(this).find('input').eq(0);
        var $row = $(this);
        $row.find('td:not(:first-child)').click(function(e) {
          $input.trigger('click');
          //$input.prop("checked", !$input.is(':checked'))
        });

        $input.keydown(function (e) {
          if (e.keyCode === 38) {
            // UP.
            var $prevRow = $row.prev();
            $prevRow.find('input').focus();
            e.preventDefault();
            e.stopPropagation();
          }
          else if(e.keyCode === 40) {
            // DOWN.
            var $nextRow = $row.next();
            $nextRow.find('input').focus().select();
            e.preventDefault();
            e.stopPropagation();
          }
        });
      });
    }
  };
}(jQuery));


jQuery(document).ready(function ($) {
  var $form = $('form');

  // Prevent double submission of forms.
  $form.submit(function(e) {
    if( $(this).hasClass('form-submitted') ){
      e.preventDefault();
      return;
    }

    if (!$('#autocomplete').length) {
      $(this).addClass('form-submitted');
    }
  });
  // Automatically print if url parameter is specified.
  var print = avbaseGetUrlParameter('print');
  if (print) {
    $('#av-print-btn').trigger('click');
  }

  //// Auto focus on first textbox or textarea.
  //console.log($("input:text"));
  $("#region-content-wrapper input.form-text:not([data-uk-datepicker]):not(.trigger-ajax-search), #region-content-wrapper textarea, #region-content-wrapper number").eq(0).focus()
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
