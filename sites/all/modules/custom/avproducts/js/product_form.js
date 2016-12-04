/**
 * Created by Aaron on 12/2/2016.
 */

(function ($) {
  /**
   * General behaviors on Product Add/Edit form.
   * @type {{attach: Function}}
   */
  Drupal.behaviors.avproductsEditForm = {
    attach: function (context, settings) {
      $('.uom-group-select:not(.processed)').change(function() {
        var uomIndex = $(this).data('uom-index');
        var $qtyElement = $('#edit-uom-group-uoms-' + uomIndex + '-qty');
        $qtyElement.attr('title', 'x');
      }).addClass('processed');
    }
  };
}(jQuery));

jQuery(document).ready(function ($) {

});



jQuery(document).load(function ($) {
  // Here we will call the function with jQuery as the parameter once the entire
  // page (images or iframes), not just the DOM, is ready.
});
