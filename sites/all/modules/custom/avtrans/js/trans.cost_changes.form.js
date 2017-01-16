(function ($) {
  /**
   * Behaviors for Cost Changes Form.
   * @type {{attach: Function}}
   */
  Drupal.behaviors.avtransCostChangesForm = {
    attach: function (context, settings) {
      $('#uk-modal-cost-changed').once('avtransCostChangedModal', function() {
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
    }
  };

}(jQuery));