(function ($) {

}(jQuery));


jQuery(document).ready(function ($) {
  $('#vendor-name').on('autocompleteSelect', function(e, node) {
    console.log('selected!');
    var twinID = $(this).data('twin-id');
    var $el = $('#' + twinID);
    $el.val($(node).data('autocompleteValue'));
    $(this).val($(node).find('div#a-vendor-text').html());
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
