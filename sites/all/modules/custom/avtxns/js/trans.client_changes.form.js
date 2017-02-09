(function ($) {
  /**
   * Behaviors for Client Changes Form.
   * @type {{attach: Function}}
   */
  Drupal.behaviors.avtxnsCientChangesForm = {
    attach: function (context, settings) {
      $('#uk-modal-client-changed').once('avtxnsClientChangedModal', function() {
        // Show client-changes popup when necessary.
        if ($(this).find('.uk-modal-body', context).html()) {
          var clientChangedModal = UIkit.modal('#uk-modal-client-changed', {center: true, bgclose: false});
          clientChangedModal.show();

          $('#client-changed-submit-btn', context).click(function() {
            clientChangedModal.hide();
            $('#detect-cost-change-btn', context).attr('disabled', 'disabled').append('<i class="uk-icon-refresh uk-icon-spin uk-margin-small-left"></i>');
          });
        }
      });
    }
  };

}(jQuery));