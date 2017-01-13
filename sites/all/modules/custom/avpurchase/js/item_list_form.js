(function ($) {



  /**
   * Behaviors of nestable product form.
   * @type {{attach: Function}}
   */
  Drupal.behaviors.avpurchaseGRNestableProductForm = {
    attach: function (context, settings) {
      $('#vendor-id', context).once('avVendorAutocomplete', function() {
        var $termEl = $('#term-id');
        $(this).on('autocompleteSelect', function(e, node) {
          var vendorID = $(node).find('#av-row-id').html();
          var vendor = Drupal.settings.avbase.vendors[vendorID] || {};
          var email = vendor.email || '';

          if (vendor.id) {
            $('#vendor-email').val(Drupal.checkPlain(email));
            $termEl.val((vendor.term_id || 0));
            $termEl.data('selectedVendorID', vendorID);
            $termEl.trigger('change');
          }
        });
      });

      new Drupal.avbaseNestableProductForm($('.av-nestable-product-list-form', context), settings);


      //$('.avpurchase-gr-form').once('avpurchaseGRNestableProductForm', function() {
      //  console.log('1');
      //
      //  Drupal.settings.avNestableProductForm = Drupal.settings.avNestableProductForm || {};
      //  Drupal.settings.avNestableProductForm.products = Drupal.settings.avNestableProductForm.products || {};
      //
      //  var nestableProdForm = Object.create(avNestableProductForm);
      //  nestableProdForm.$nestableEl = $('.av-nestable-product-list-form');
      //  nestableProdForm.init({handleClass:'uk-nestable-handle', maxDepth: 5});
      //
      //  var $termEl = $('#term-id');
      //  $('#vendor_id').on('autocompleteSelect', function(e, node) {
      //    var vendorID = $(node).find('#av-row-id').html();
      //    var vendor = Drupal.settings.avbase.vendors[vendorID] || {};
      //    var email = vendor.email || '';
      //
      //    if (vendor.id) {
      //      $('#vendor-email').val(Drupal.checkPlain(email));
      //      $termEl.val((vendor.term_id || 0));
      //      $termEl.data('selectedVendorID', vendorID);
      //      $termEl.trigger('change');
      //    }
      //  });
      //
      //  nestableProdForm.bindEvents();
      //
      //  // Trigger nestable 'removed' event.
      //  if (settings.avNestableProductForm && settings.avNestableProductForm.ajaxAction == 'remove') {
      //    nestableProdForm.refreshRowNumbers();
      //    nestableProdForm.refreshGrandTotalText();
      //  }
      //});
    }
  };
}(jQuery));


jQuery(document).ready(function ($) {

});



jQuery(document).load(function ($) {
  // Here we will call the function with jQuery as the parameter once the entire
  // page (images or iframes), not just the DOM, is ready.
});
