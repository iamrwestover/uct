(function ($) {
  /**
   * Behaviors for Pending Orders Form.
   * @type {{attach: Function}}
   */
  Drupal.behaviors.avtxnsPendingOrdersForm = {
    attach: function (context, settings) {
      $('#uk-modal-pending-orders').once('avtxnsPendingOrdersModal', function() {
        // Show client floating POs popup when necessary.
        if ($(this).find('#avtxns-pending-orders-form-wrapper', context).html()) {
          var modal = UIkit.modal('#uk-modal-pending-orders', {center: true, bgclose: false});
          modal.show();

          $('#pos-selected-submit-btn', context).click(function() {
            modal.hide();
          });
        }
      });

      $('#client-id').once('avtxnsPendingOrderClientID', function() {
        // Trigger client PO submit when client field value is changed.
        $(this).blur(function() {
          $('#check-po-btn').trigger('click');
        });
      });
    }
  };
}(jQuery));