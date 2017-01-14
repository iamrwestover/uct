(function ($) {
  /**
   * Behaviors of nestable product form.
   * @type {{attach: Function}}
   */
  Drupal.behaviors.avpurchaseGRForm = {
    attach: function (context, settings) {
      $('#uk-modal-cost-changed').once('avpurchaseCostChangedModal', function() {
        if ($(this).find('.uk-modal-body', context).html()) {
          var costChangedModal = UIkit.modal('#uk-modal-cost-changed', {center: true, bgclose: false});
          costChangedModal.show();

          $('#submit-btn', context).click(function() {
            costChangedModal.hide();
            $('#detect-cost-change-btn', context).attr('disabled', 'disabled').append('<i class="uk-icon-refresh uk-icon-spin uk-margin-small-left"></i>');
          });
        }
      });

      $('#vendor-id', context).once('avpurchaseGRVendorID', function() {
        $(this).change(function() {
          $('#check-po-btn', context).trigger('click');
        });
      });

      $('#uk-modal-vendor-pos').once('avpurchaseVendorPOsModal', function() {
        if ($(this).find('#avpurchase-gr-form-vendor-pos-wrapper', context).html()) {
          var modal = UIkit.modal('#uk-modal-vendor-pos', {center: true, bgclose: false});
          modal.show();

          $('#pos-selected-submit-btn', context).click(function() {
            modal.hide();
            //$('#detect-cost-change-btn', context).attr('disabled', 'disabled').append('<i class="uk-icon-refresh uk-icon-spin uk-margin-small-left"></i>');
          });
        }
      });
    }
  };

}(jQuery));


jQuery(document).ready(function ($) {
  //var termActions = Object.create(window.avPaymentTermActions);
  //termActions.preventChildrenHide = true;
  //termActions.init();
});


jQuery(document).load(function ($) {
  // Here we will call the function with jQuery as the parameter once the entire
  // page (images or iframes), not just the DOM, is ready.
});
