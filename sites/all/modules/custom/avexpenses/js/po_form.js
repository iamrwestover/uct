(function ($) {
  $('#vendor-id').on('autocompleteSelect', function(e, node) {
    $('#vendor-email').val($(node).find('div#avvendor-email').html());
  });

  $('.prod-group-id').on('autocompleteSelect', function(e, node) {
    console.log(Drupal.settings);
    //$('#prod-code-' + $(this).data('row-index')).html($(node).find('div#av-prod-code').html());
  });

  $('.av-nestable-product-list-form').on('change.uk.nestable', function(e, uikit, el, action) {
    if (action == 'moved') {
      // Recalculate row numbers.
      $(this).find('.uk-nestable-item').each(function(index) {
        $(this).find('.av-nestable-row-num').html(index + 1);
      });
    }
  });



  Drupal.behaviors.avPoNestableProductForm = {
    attach: function (context, settings) {
      $('.prod-group-id').once('avNestableProdGroupIds', function() {
        $(this).focus(function() {
          if (!$(this).closest('.uk-nestable-item').next().hasClass('uk-nestable-item')) {
            $('#prod-add-btn').trigger('mousedown');
          }
        });
      });
      //$('.prod-group-uom-id').once('avponestable-autoselect-qty', function() {
      //  $(this).change(function() {
      //    console.log('s');
      //    var index = $(this).data('prod-index');
      //    $('#prod-qty-' + index).focus().select();
      //  });
      //});
    }
  }
}(jQuery));


jQuery(document).ready(function ($) {

});


jQuery(document).load(function ($) {
  // Here we will call the function with jQuery as the parameter once the entire
  // page (images or iframes), not just the DOM, is ready.
});
