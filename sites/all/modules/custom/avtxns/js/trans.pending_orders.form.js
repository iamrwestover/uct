(function ($) {
  /**
   * Behaviors for Pending Orders Form.
   * @type {{attach: Function}}
   */
  Drupal.behaviors.avtxnsPendingOrdersForm = {
    attach: function (context, settings) {
      var $clientEl = $('#client-id');
      $('#uk-modal-pending-orders').once('avtxnsPendingOrdersModal', function() {
        if ($clientEl.val() != '') {
          $('input.form-text:not([readonly]):enabled').eq(1).focus();
        }
        // Show client floating POs popup when necessary.
        if ($(this).find('#avtxns-pending-orders-form-wrapper', context).html()) {
          var modal = UIkit.modal('#uk-modal-pending-orders', {center: true, bgclose: false});
          modal.show();
          $(this).find('input').focus();

          $('#pos-selected-submit-btn', context).click(function() {
            modal.hide();
          });
        }
        $(this).on('hide.uk.modal', function() {
          $('input.form-text:not([readonly]):enabled').eq(1).focus();
        });
      });

      $clientEl.once('avtxnsPendingOrderClientID', function() {
        // Trigger client PO submit when client field value is changed.
        $(this).blur(function() {
          if ($(this).val() != '') {
            $('#check-po-btn').trigger('mousedown');
          }
        });
      });
    }
  };
}(jQuery));