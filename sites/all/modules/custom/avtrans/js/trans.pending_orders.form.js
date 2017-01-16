(function ($) {
  /**
   * Behaviors for Pending Orders Form.
   * @type {{attach: Function}}
   */
  Drupal.behaviors.avtransPendingOrdersForm = {
    attach: function (context, settings) {
      $('#uk-modal-pending-orders').once('avtransPendingOrdersModal', function() {
        // Show vendor floating POs popup when necessary.
        if ($(this).find('#avtrans-pending-orders-form-wrapper', context).html()) {
          var modal = UIkit.modal('#uk-modal-pending-orders', {center: true, bgclose: false});
          modal.show();

          $('#pos-selected-submit-btn', context).click(function() {
            modal.hide();
          });
        }
      });

      $('#client-id').once('avtransPendingOrderClientID', function() {
        // Trigger Vendor PO submit when vendor field value is changed.
        $(this).blur(function() {
          $('#check-po-btn').trigger('click');
        });
      });
    }
  };
}(jQuery));