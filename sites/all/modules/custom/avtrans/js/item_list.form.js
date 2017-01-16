(function ($) {



  /**
   * Behaviors of nestable product form.
   * @type {{attach: Function}}
   */
  Drupal.behaviors.avtransGRNestableProductForm = {
    attach: function (context, settings) {
      $('#client-id', context).once('avVendorAutocomplete', function() {
        $(this).on('autocompleteSelect', function(e, node) {
          var clientID = $(this).data('selected-row-id');
          var groupName = $(this).data('avbase-entity-group');
          var client = Drupal.settings.avbase[groupName][clientID] || {};
          var email = client.email || '';

          if (client.id) {
            $('#vendor-email').val(Drupal.checkPlain(email));
          }

          //$(this).trigger('change');
        });
      });

      $('.av-nestable-product-list-form', context).once('avNestableProductListForm', function() {
        new Drupal.avbaseNestableProductForm($('.av-nestable-product-list-form', context), settings);
      });
    }
  };
}(jQuery));
