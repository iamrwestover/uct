(function ($) {
  window.avNestableProductForm = {
    $nestableEl: null,
    $productRows: {},
    $grandTotalEl: null,
    $subTotalEl: null,
    $discountTotalEl: null,
    $discountTypeEl: null,
    $discountValEl: null,
    init: function(options) {
      var self = this;
      self.$grandTotalEl = $('.product-form-grand-total');
      self.$subTotalEl = $('.product-form-sub-total');
      self.$discountTotalEl = $('.product-form-discount-total');
      self.$discountTypeEl = $('#discount-type');
      self.$discountValEl = $('#discount-value');

      // Initialize uikit nestable component.
      UIkit.nestable(self.$nestableEl, options);



      // Do stuff when nestable form is changed.
      self.$nestableEl.on('change.uk.nestable', function(e, uikit, el, action) {
        if (action == 'moved' || action == 'removed') {
          // Recalculate row numbers.
          self.refreshRowNumbers();
        }
      });

      self.bindEvents();
      self.refreshGrandTotalText();
    },

    refreshRowNumbers: function() {
      this.$nestableEl.find('.uk-nestable-item').each(function(index) {
        $(this).find('.av-nestable-row-num').html(index + 1);
      });
    },

    bindEvents: function() {
      var self = this;

      // Trigger actions for discount fields.
      self.$discountValEl.once('avPODiscountVal', function() {
        $(this).change(function() {
          //self.refreshDiscountTotalText();
        });
      });
      self.$discountTypeEl.once('avPODiscountType', function() {
        self.$discountValEl.trigger('change');
      });

      this.$productRows = self.$nestableEl.find('.uk-nestable-item');
      this.$productRows.each(function() {
        var $thisRow = $(this);
        var $productIDEl = $thisRow.find('.prod-column-id');
        var $UOMEl = $thisRow.find('.prod-column-uom-id');
        var $priceEl = $thisRow.find('.prod-column-price');
        var $qtyEl = $thisRow.find('.prod-column-qty');
        var $amountEl = $thisRow.find('.prod-column-amt');

        // Trigger actions for autocomplete product field.
        $productIDEl.once('avNestableProdGroupProdID', function() {
          $(this).on('autocompleteSelect', function(e, node) {
            // Get product details if not yet set.
            var productID = $(node).find('div#av-prod-id').html();
            $(this).data('selectedProductID', productID);
            if (!Drupal.settings.avNestableProductForm.products[productID]) {
              Drupal.settings.avNestableProductForm.products[productID] = $.parseJSON($(node).find('div#av-prod-json').html());
            }

            var productDetails = Drupal.settings.avNestableProductForm.products[productID];
            if (productDetails.uom_id) {
              $UOMEl.val(productDetails.uom_id);
              $UOMEl.trigger('change');
            }
          });

          // Trigger adding new row when last Product field is reached.
          $(this).focus(function() {
            if (!$(this).closest('.uk-nestable-item').next().hasClass('uk-nestable-item')) {
              $('#prod-add-btn').trigger('mousedown');
            }
            $(this).select();
          });
        });

        // Trigger actions for UOM select field.
        $UOMEl.once('avNestableProdGroupUOMID', function() {
          $(this).change(function() {
            // Get current row details.
            if (!$productIDEl.length) {
              return;
            }

            // Get selected product details
            var selectedProductID = $productIDEl.data('selectedProductID');
            var productDetails = Drupal.settings.avNestableProductForm.products[selectedProductID] || {};
            if (productDetails.title != $productIDEl.val()) {
              return;
            }

            // Get uom details.
            var uomID = $(this).val();
            var uoms = productDetails.data.uoms || {};

            // Auto-fill price fields.
            var price = 0;
            if (uomID == 0) {
              return;
            }
            else if (uomID == productDetails.uom_id) {
              // Selected UOM is the same as base UOM.
              price = parseFloat(productDetails.cost);
              $priceEl.val(parseFloat(price.toFixed(2)));
            }
            else if (uoms[uomID]) {
              // Selected UOM is a data UOM.
              price = parseFloat(productDetails.cost) * parseFloat(uoms[uomID]['qty']);
              $priceEl.val(parseFloat(price.toFixed(2)));
            }
            else {
              $priceEl.val('');
            }

            $priceEl.trigger('change');
          });
        });

        // Trigger actions for price field.
        $priceEl.once('avNestableProdGroupPrice', function() {
          $(this).change(function() {
            var amount = self.getRowAmount($thisRow);
            if (amount) {
              $amountEl.val(amount.toFixed(2));
              self.refreshGrandTotalText();
            }
          });
          $(this).keydown(function(e) {
            self.switchRowFocus(e, $thisRow);
          });
        });

        // Trigger actions for qty field.
        $qtyEl.once('avNestableProdGroupQty', function() {
          $(this).change(function() {
            $priceEl.trigger('change');
          });

          $(this).keydown(function(e) {
            self.switchRowFocus(e, $thisRow);
          });
        });

        // Trigger actions for amount field.
        $amountEl.once('avNestableProdGroupAmount', function() {
          $(this).change(function() {
            var price = self.getRowPrice($thisRow);
            if (price) {
              $priceEl.val(parseFloat(price.toFixed(6)));
            }
            //console.log($(this).val().toFixed(2));
            $(this).val(parseFloat($(this).val()).toFixed(2));
            self.refreshGrandTotalText();
          });

          $(this).keydown(function(e) {
            self.switchRowFocus(e, $thisRow);
          });
        });
      });
    },

    getRowAmount: function($row) {
      var $priceEl = $row.find('.prod-column-price');
      var $qtyEl = $row.find('.prod-column-qty');
      var qty = Drupal.checkPlain($qtyEl.val());
      var price = Drupal.checkPlain($priceEl.val());

      var error = false;
      $priceEl.removeClass('uk-form-danger');
      $qtyEl.removeClass('uk-form-danger');
      if (!$.isNumeric(price)) {
        if (price != '') {
          $priceEl.addClass('uk-form-danger');
        }
        error = true;
      }
      if (!$.isNumeric(qty)) {
        if (qty != '') {
          $qtyEl.addClass('uk-form-danger');
        }
        error = true;
      }

      if (error) {
        return '';
      }

      var amount = qty * price;
      return amount;
    },

    getRowPrice: function($row) {
      var $amountEl = $row.find('.prod-column-amt');
      var $qtyEl = $row.find('.prod-column-qty');
      var qty = Drupal.checkPlain($qtyEl.val());
      var amount = Drupal.checkPlain($amountEl.val());

      var error = false;
      $amountEl.removeClass('uk-form-danger');
      $qtyEl.removeClass('uk-form-danger');
      if (!$.isNumeric(amount)) {
        if (amount != '') {
          $amountEl.addClass('uk-form-danger');
        }
        error = true;
      }
      if (!$.isNumeric(qty)) {
        if (qty != '') {
          $qtyEl.addClass('uk-form-danger');
        }
        error = true;
      }

      if (error) {
        return '';
      }

      var price = amount / qty;
      return price;
    },

    refreshSubTotalText: function() {
      this.$subTotalEl.html(this.moneyFormat(this.getSubTotal()));
    },

    refreshDiscountTotalText: function() {
      this.$discountTotalEl.html(this.moneyFormat(this.getDiscountTotal()));
    },

    refreshGrandTotalText: function() {
      this.$grandTotalEl.html(this.moneyFormat(this.getGrandTotal()));
    },

    getSubTotal: function() {
      var $amountEls = this.$productRows.find('.prod-column-amt');
      var rowTotal = 0;
      var subTotal = 0;
      $amountEls.each(function() {
        rowTotal = $(this).val();
        if (!$.isNumeric(rowTotal)) {
          return;
        }
        subTotal += parseFloat(rowTotal);
      });

      return parseFloat(subTotal).toFixed(2);
    },

    getDiscountTotal: function() {
      var subTotal = this.getSubTotal();
      var discountType = this.$discountTypeEl.val();
      var discountValue = $('#discount-value').val();
      var discountTotal = 0;
      if (discountType && discountValue && $.isNumeric(discountValue)) {
        if (discountType == 1) {
          discountTotal = (subTotal * discountValue) / 100;
        }
        else if(discountType == 2) {
          discountTotal = subTotal - discountValue;
        }
      }
      return parseFloat(discountTotal).toFixed(2);
    },

    getGrandTotal: function() {
      var subTotal = this.getSubTotal();
      var discountTotal = this.getDiscountTotal();
      console.log(subTotal);
      console.log(discountTotal);
      var grandTotal = subTotal - discountTotal;
      return parseFloat(grandTotal).toFixed(2);
    },

    switchRowFocus: function(e, $row) {
      var columnName = $(e.target).data('column-name');
      if (e.keyCode === 38) {
        // UP.
        var $prevRow = $row.closest('.uk-nestable-item').prev();
        if ($prevRow.hasClass('uk-nestable-item')) {
          $prevRow.find('.prod-column-' + columnName).focus().select();
          e.preventDefault();
          e.stopPropagation();
        }
      }
      else if(e.keyCode === 40) {
        // DOWN.
        var $nextRow = $row.closest('.uk-nestable-item').next();
        if ($nextRow.hasClass('uk-nestable-item')) {
          $nextRow.find('.prod-column-' + columnName).focus().select();
          e.preventDefault();
          e.stopPropagation();
        }
      }
    },

    moneyFormat: function(nStr) {
      nStr += '';
      x = nStr.split('.');
      x1 = x[0];
      x2 = x.length > 1 ? '.' + x[1] : '';
      var rgx = /(\d+)(\d{3})/;
      while (rgx.test(x1)) {
        x1 = x1.replace(rgx, '$1' + ',' + '$2');
      }
      return x1 + x2;
    }
  };
}(jQuery));

jQuery(document).ready(function ($) {
  //console.log('x');
  //console.log(avNestableForm2);
});