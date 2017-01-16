(function ($) {
  /**
   * Behaviors of nestable product form.
   * @type {{attach: Function}}
   */
  Drupal.behaviors.avpurchaseGRForm = {
    attach: function (context, settings) {
      $('#uk-modal-cost-changed').once('avpurchaseCostChangedModal', function() {
        // Show cost-changes popup when necessary.
        if ($(this).find('.uk-modal-body', context).html()) {
          var costChangedModal = UIkit.modal('#uk-modal-cost-changed', {center: true, bgclose: false});
          costChangedModal.show();

          $('#cost-changed-submit-btn', context).click(function() {
            costChangedModal.hide();
            $('#detect-cost-change-btn', context).attr('disabled', 'disabled').append('<i class="uk-icon-refresh uk-icon-spin uk-margin-small-left"></i>');
          });
        }
      });

      $('#uk-modal-vendor-pos').once('avpurchaseVendorPOsModal', function() {
        // Show vendor floating POs popup when necessary.
        if ($(this).find('#avpurchase-gr-form-vendor-pos-wrapper', context).html()) {
          var modal = UIkit.modal('#uk-modal-vendor-pos', {center: true, bgclose: false});
          modal.show();

          $('#pos-selected-submit-btn', context).click(function() {
            modal.hide();
            //$('#detect-cost-change-btn', context).attr('disabled', 'disabled').append('<i class="uk-icon-refresh uk-icon-spin uk-margin-small-left"></i>');
          });
        }
      });

      $('#vendor-id').once('avpurchaseGRVendorID', function() {
        // Trigger Vendor PO submit when vendor field value is changed.
        $(this).blur(function() {
          $('#check-po-btn').trigger('click');
        });
      });
    }
  };

}(jQuery));