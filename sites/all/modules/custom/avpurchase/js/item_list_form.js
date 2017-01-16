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

          $(this).trigger('change');
        });
      });

      $('.av-nestable-product-list-form', context).once('avNestableProductListForm', function() {
        new Drupal.avbaseNestableProductForm($('.av-nestable-product-list-form', context), settings);
      });
    }
  };
}(jQuery));
