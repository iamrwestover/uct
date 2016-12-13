(function ($) {
  Drupal.settings.avNestableProductForm = Drupal.settings.avNestableProductForm || {};
  Drupal.settings.avNestableProductForm.products = Drupal.settings.avNestableProductForm.products || {};

  $('#vendor-id').on('autocompleteSelect', function(e, node) {
    $('#vendor-email').val($(node).find('div#avvendor-email').html());
  });

  $('.prod-group-id').on('autocompleteSelect', function(e, node) {
    // Get product details if not yet set.
    var productID = $(node).find('div#av-prod-id').html();
    $(this).data('selectedProductID', productID);
    if (!Drupal.settings.avNestableProductForm.products[productID]) {
      Drupal.settings.avNestableProductForm.products[productID] = $.parseJSON($(node).find('div#av-prod-json').html());
    }

    var productKey = $(this).data('prod-key');
    var productDetails = Drupal.settings.avNestableProductForm.products[productID];
    var $uomEl = $('#uom-id-' + productKey);
    if (productDetails.uom_id) {
      $uomEl.val(productDetails.uom_id);
      $uomEl.trigger('change');
    }
  });

  $('.av-nestable-product-list-form').on('change.uk.nestable', function(e, uikit, el, action) {
    if (action == 'moved' || action == 'removed') {
      // Recalculate row numbers.
      $(this).find('.uk-nestable-item').each(function(index) {
        $(this).find('.av-nestable-row-num').html(index + 1);
      });
    }
  });



  Drupal.behaviors.avPoNestableProductForm = {
    attach: function (context, settings) {
      if (settings.avNestableProductForm && settings.avNestableProductForm.ajaxAction == 'remove') {
        // Trigger nestable 'removed' event.
        $('.av-nestable-product-list-form').trigger('change.uk.nestable', [this, null, 'removed']);
      }

      // Trigger adding new row when last Product field is reached.
      $('.prod-group-id').once('avNestableProdGroupIds', function() {
        $(this).focus(function() {
          if (!$(this).closest('.uk-nestable-item').next().hasClass('uk-nestable-item')) {
            $('#prod-add-btn').trigger('mousedown');
          }
          $(this).select();
        });
      });

      $('.prod-group-uom-id').once('avNestableProdGroupUOMIDs', function() {
        $(this).change(function() {
          var productKey = $(this).data('prod-key');
          var $productEl = $('#product-id-' + productKey);
          var $priceEl = $('#product-price-' + productKey);
          if (!$productEl.length) {
            return;
          }
          var selectedProductID = $productEl.data('selectedProductID');
          var productDetails = Drupal.settings.avNestableProductForm.products[selectedProductID] || {};
          if (productDetails.title != $productEl.val()) {
            return;
          }
          var uomID = $(this).val();
          var uoms = productDetails.data.uoms || {};
          console.log(productDetails.price);
          var price = 0;
          if (uomID == productDetails.uom_id) {
            price = parseFloat(productDetails.price);
            $priceEl.val(parseFloat(price.toFixed(2)));
          }
          else if (uoms[uomID]) {
            price = parseFloat(uoms[uomID]['price']);
            $priceEl.val(parseFloat(price.toFixed(2)));
          }
          else {
            $priceEl.val('');
          }
        });
      });
    }
  }
}(jQuery));


jQuery(document).ready(function ($) {

});


jQuery(document).load(function ($) {
  // Here we will call the function with jQuery as the parameter once the entire
  // page (images or iframes), not just the DOM, is ready.
});
