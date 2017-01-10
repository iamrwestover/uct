(function ($) {


}(jQuery));


jQuery(document).ready(function ($) {
  var xxauto = $.UIkit.autocomplete($('#xxaa'), {});
  var data = [
    {"value":"Piece", "title":"Piece", "text":"base uom"},
    {"value":"Box", "title":"Box"},
    {"value":"Dozen", "title":"Dozen", "text":"12 pieces per box"}
  ];
  xxauto.render(data);

  var $x = $('#xxaa').find('input');
  $x.attr('readonly', true);
  $x.click(function(e) {
    e.preventDefault();
    xxauto.render(data);
  });

  Drupal.settings.avNestableProductForm = Drupal.settings.avNestableProductForm || {};
  Drupal.settings.avNestableProductForm.products = Drupal.settings.avNestableProductForm.products || {};

  var nestableProdForm = Object.create(avNestableProductForm);
  nestableProdForm.$nestableEl = $('.av-nestable-product-list-form');
  nestableProdForm.init({handleClass:'uk-nestable-handle', maxDepth: 5});

  var vendorAutocomplete = Object.create(avbaseAutocompleteActions);
  vendorAutocomplete.$autocompleteEl = $('#vendor-id');
  vendorAutocomplete.rowGroupName = 'vendors';
  vendorAutocomplete.init();

  var $termEl = $('#term-id');
  vendorAutocomplete.$autocompleteEl.on('autocompleteSelect', function(e, node) {
    var vendorID = $(node).find('#av-row-id').html();
    var vendor = Drupal.settings.avBase.vendors[vendorID] || {};
    var email = vendor.email || '';

    if (vendor.id) {
      $('#vendor-email').val(Drupal.checkPlain(email));
      $termEl.val((vendor.term_id || 0));
      $termEl.data('selectedVendorID', vendorID);
      $termEl.trigger('change');
    }
  });
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
