(function ($) {


}(jQuery));


jQuery(document).ready(function ($) {
  //var termActions = Object.create(window.avPaymentTermActions);
  //termActions.preventChildrenHide = true;
  //termActions.init();

  /**
   * Behaviors of nestable product form.
   * @type {{attach: Function}}
   */
  Drupal.behaviors.avpurchaseGRForm = {
    attach: function (context, settings) {
      $('#uk-modal-cost-changed').once('avpurchaseCostChangedModal', function() {
        if ($(this).find('.uk-modal-body').html()) {
          var costChangedModal = UIkit.modal('#uk-modal-cost-changed', {center: true, bgclose: false});
          costChangedModal.show();
        }
        else {
          //$(this).find('button.form-submit').trigger('click');
          //$('.uk-button').attr('disabled', 'disabled');
        }
        //$('#uk-modal-toggle-cost-changed').click(function(e) {
        //  e.preventDefault();
        //  costChangedModal.show();
        //});
        //$('#uk-modal-cost-changed').find('.uk-button-modal-ok').click(function(e) {
        //  e.preventDefault();
        //  costChangedModal.hide();
        //  $('#submit-btn').trigger('click');
        //});
      });
    }
  };


});


jQuery(document).load(function ($) {
  // Here we will call the function with jQuery as the parameter once the entire
  // page (images or iframes), not just the DOM, is ready.
});
