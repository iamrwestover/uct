(function ($) {
  if (Drupal.jsAC) {
    Drupal.jsAC.prototype.setStatus = function (status) {
      switch (status) {
        case 'begin':
                console.log('r');
          $(this.input).addClass('throbbing');
          $(this.ariaLive).html(Drupal.t('Searching for matches...'));
          break;
        case 'cancel':
        case 'error':
        case 'found':
          $(this.input).removeClass('throbbing');
          break;
      }
    };
  }
}(jQuery));


jQuery(document).ready(function ($) {
  $('#vendor-name').on('autocompleteSelect', function(e, node) {
    console.log('selected!');
    var twinID = $(this).data('twin-id');
    var $el = $('#' + twinID);
    $el.val($(node).data('autocompleteValue'));
    //console.log($(node).data('autocompleteValue'));
    //console.log($(node));
    $(this).val($(node).find('div').html());
    //console.log($(node).innerHTML());
    console.log(Drupal.ACDB.prototype);
  });

  //$('.form-autocomplete').change(function() {
  //  var twinID = $(this).data('twin-id');
  //  var $el = $('#' + twinID);
  //  if ($el.length) {
  //    $el.val('');
  //  }
  //}).keyup(function() {
  //  $(this).trigger('change');
  //});
});


jQuery(document).load(function ($) {
  // Here we will call the function with jQuery as the parameter once the entire
  // page (images or iframes), not just the DOM, is ready.
});
