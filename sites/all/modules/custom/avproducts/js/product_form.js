/**
 * Created by Aaron on 12/2/2016.
 */

(function ($) {
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
      $('#multiple-uom-wrapper').once('avproductsAmtAutoComputeInit', function() {
        // Compute cost on first page load.
        $('#uom-cost').trigger('keyup');
      });

      // Update UOM qty field tooltips whenever UOM dropdown value is changed.
      $('.uom-group-select').once('avproductsTooltipUpdate', function() {
        $(this).change(function() {
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
            var nextUOMName = avbaseCategoryGetname($nextUOMEl.val());
            nextUOMName = nextUOMName ? nextUOMName.toLowerCase() : 'uom';
            $nextQtyEl.attr('title', Drupal.t('number of @uom1 per @uom2', {'@uom1': currentUOMNamePlural, '@uom2': nextUOMName}));
          }
        });
      });

      // Auto-compute and update UOM cost and price values (only if empty) whenever base cost and price values are changed.
      $('#uom-cost, #uom-price').once('avproductsAmtAutoCompute', function() {
        $(this).change(function(e, UOMIndex) {
          var amount = Drupal.checkPlain($(this).val());

          // @todo compute based on child UOM
          var fieldId = $(this).attr('id');
          var $uomIDs;
          if (typeof(UOMIndex) != 'undefined' && UOMIndex !== null) {
            $uomIDs = $('#uom-select-' + UOMIndex);
          }
          else {
            $uomIDs = $('.uom-group-select');
          }
          $uomIDs.each(function() {
            var uomIndex = $(this).data('uom-index');
            var $qtyEl = $('#uom-qty-' + uomIndex);
            var $amountEl = $('#' + fieldId + '-' + uomIndex);

            var qty = Drupal.checkPlain($qtyEl.val());
            if (fieldId == 'uom-price' && $amountEl.val() != '') {
              // If qty isn't numeric or;
              // If $amountEl is not empty, user already specified desired price, so no need to auto-compute.
              //return;
            }
            if (!$.isNumeric(amount) || !$.isNumeric(qty)) {
              $amountEl.val('');
              return;
            }

            var computedAmount = amount * qty;
            $amountEl.val(parseFloat(computedAmount.toFixed(2)));
          });
        });

        $(this).keyup(function(e, qtyTriggered) {
          $(this).trigger('change');
        });
      });

      $('.uom-group-qty').once('avproductsAmtAutoCompute', function() {
        $(this).change(function() {
          $('#uom-cost, #uom-price').trigger('change', $(this).data('uom-index'));
        });
        $(this).keyup(function() {
          $(this).trigger('change');
        });
      });
    }
  };

  /**
   * Get category name.
   * @param UOMID
   * @param isPlural
   * @returns {string}
   */
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
}(jQuery));

jQuery(document).ready(function ($) {
  // Auto-compute cost on first page load.
  $('#uom-cost').trigger('change');
});



jQuery(document).load(function ($) {
  // Here we will call the function with jQuery as the parameter once the entire
  // page (images or iframes), not just the DOM, is ready.
});
