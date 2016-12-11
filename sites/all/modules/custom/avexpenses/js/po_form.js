(function ($) {
  $('#vendor-id').on('autocompleteSelect', function(e, node) {
    $('#vendor-email').val($(node).find('div#avvendor-email').html());
  });
}(jQuery));


jQuery(document).ready(function ($) {

});


jQuery(document).load(function ($) {
  // Here we will call the function with jQuery as the parameter once the entire
  // page (images or iframes), not just the DOM, is ready.
});
