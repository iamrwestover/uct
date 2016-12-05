/**
 * Created by Aaron on 12/2/2016.
 */

(function ($) {
  function avbaseCategoryGetname(UOMID, isPlural) {
    var uoms = Drupal.settings.avbaseUOMCategories;
    var UOMName = '';
    if (uoms[UOMID]) {
      if (isPlural) {
        UOMName = uoms[UOMID].data.plural_form ? uoms[UOMID].data.plural_form : UOMName;
      }
      UOMName = UOMName ? UOMName : ((uoms[UOMID].title) ? uoms[UOMID].title : '');
    }
    return UOMName;
  }

  var $baseUOMEl = $('#uom-id');
  $baseUOMEl.change(function() {
    var $firstUOMEl = $('#uom-select-0');
    var $firstQtyEl = $('#uom-qty-0');
    if ($firstUOMEl.length) {
      var UOMName = avbaseCategoryGetname($(this).val(), true);
      var firstUOMName = avbaseCategoryGetname($firstUOMEl.val());
      UOMName = UOMName ? UOMName.toLowerCase() : '(base uom)';
      firstUOMName = firstUOMName ? firstUOMName.toLowerCase() : 'uom';
      $firstQtyEl.attr('title', Drupal.t('number of @uom1 per @uom2', {'@uom1': UOMName, '@uom2': firstUOMName}));
    }
  });

  /**
   * General behaviors on Product Add/Edit form.
   * @type {{attach: Function}}
   */
  Drupal.behaviors.avproductsEditForm = {
    attach: function (context, settings) {
      $('.uom-group-select:not(.processed)').change(function() {
        var uomIndex = $(this).data('uom-index');
        var prevUOMID = $baseUOMEl.val();
        var currentUOMID = $(this).val();
        var $qtyEl = $('#uom-qty-' + uomIndex);
        var $nextUOMEl = $('#uom-select-' + (uomIndex + 1));
        var $nextQtyEl = $('#uom-qty-' + (uomIndex + 1));
        var $prevUOMEl = $('#uom-select-' + (uomIndex - 1));

        // Get current and previous UOM name.
        if ($prevUOMEl.length) {
          prevUOMID = $prevUOMEl.val();
        }

        var prevUOMName = avbaseCategoryGetname(prevUOMID, true);
        var currentUOMName = avbaseCategoryGetname(currentUOMID);
        var currentUOMNamePlural = avbaseCategoryGetname(currentUOMID, true);
        prevUOMName = prevUOMName ? prevUOMName.toLowerCase() : ($prevUOMEl.length ? 'uom' : '(base uom)');
        currentUOMName = currentUOMName ? currentUOMName.toLowerCase() : 'uom';
        currentUOMNamePlural = currentUOMNamePlural ? currentUOMNamePlural.toLowerCase() : 'uom';


        $qtyEl.attr('title', Drupal.t('number of @uom1 per @uom2', {'@uom1': prevUOMName, '@uom2': currentUOMName}));
        $qtyEl.focus().select();

        if ($nextQtyEl.length) {
          var nextUOMName = avbaseCategoryGetname($nextUOMEl.val(), true);
          nextUOMName = nextUOMName ? nextUOMName.toLowerCase() : 'uom';
          $nextQtyEl.attr('title', Drupal.t('number of @uom1 per @uom2', {'@uom1': currentUOMNamePlural, '@uom2': nextUOMName}));
        }
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
