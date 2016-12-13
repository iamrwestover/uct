(function ($) {

  $('#vendor-id').on('autocompleteSelect', function(e, node) {
    $('#vendor-email').val($(node).find('div#avvendor-email').html());
  });
}(jQuery));


jQuery(document).ready(function ($) {
  Drupal.settings.avNestableProductForm = Drupal.settings.avNestableProductForm || {};
  Drupal.settings.avNestableProductForm.products = Drupal.settings.avNestableProductForm.products || {};

  var nestableProdForm = Object.create(avNestableProductForm);
  nestableProdForm.init($('.av-nestable-product-list-form'), {handleClass:'uk-nestable-handle', maxDepth: 5});

  /**
   * Behaviors of nestable product form.
   * @type {{attach: Function}}
   */
  Drupal.behaviors.avPoNestableProductForm = {
    attach: function (context, settings) {
      nestableProdForm.bindEvents();
      if (settings.avNestableProductForm && settings.avNestableProductForm.ajaxAction == 'remove') {
        // Trigger nestable 'removed' event.
        nestableProdForm.refreshRowNumbers();
        nestableProdForm.refreshGrandTotalText();
      }
    }
  }
});


jQuery(document).load(function ($) {
  // Here we will call the function with jQuery as the parameter once the entire
  // page (images or iframes), not just the DOM, is ready.
});
