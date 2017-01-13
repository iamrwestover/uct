(function ($) {

  /**
   * An avbaseNestableProductForm object.
   * @param $wrapper
   * @param settings
   */
  Drupal.avbaseNestableProductForm = function ($wrapper, settings) {
    var self = this;
    $wrapper.once('avNestableProductForm', function() {

      // Initialize uikit nestable component.
      UIkit.nestable($wrapper, {handleClass:'uk-nestable-handle', maxDepth: 1});
      Drupal.settings.avNestableProductForm = Drupal.settings.avNestableProductForm || {};
      Drupal.settings.avNestableProductForm.products = Drupal.settings.avNestableProductForm.products || {};


      // Do stuff when nestable form is changed.
      $wrapper.on('change.uk.nestable', function(e, uikit, el, action) {
        if (action == 'moved' || action == 'removed') {
          // Recalculate row numbers.
          self.refreshRowNumbers();
        }
      });


    });

    self.$wrapper = $wrapper;
    self.$grandTotalEl = $('.product-form-grand-total');
    self.$subTotalEl = $('.product-form-sub-total');
    self.$discountTotalEl = $('.product-form-discount-total');
    self.$discountTypeEl = $('#discount-type');
    self.$discountValEl = $('#discount-value');


    self.bindEvents();
    self.refreshSubTotalText();

    // Trigger nestable 'removed' event.
    if (settings.avNestableProductForm && settings.avNestableProductForm.ajaxAction == 'remove') {
      self.refreshRowNumbers();
      self.refreshGrandTotalText();
    }
  };

  /**
   * Bind events to necessary elements within the wrapper.
   */
  Drupal.avbaseNestableProductForm.prototype.bindEvents = function () {
    var self = this;
    // Trigger actions for discount fields.
    self.$discountValEl.once('avPODiscountVal', function() {
      $(this).change(function() {
        self.refreshDiscountTotalText();
      });
    });
    self.$discountTypeEl.once('avPODiscountType', function() {
      $(this).change(function() {
        self.$discountValEl.trigger('change');
      });
    });

    self.$productRows = self.$wrapper.find('.uk-nestable-item');
    self.$productRows.once('avNestableProductRows', function() {
      var $thisRow = $(this);
      var $productIDEl = $thisRow.find('.prod-column-id');
      var $UOMEl = $thisRow.find('.prod-column-uom-id');
      var $qtyPerUOMEl = $thisRow.find('.prod-column-qty-per-uom');
      var $priceEl = $thisRow.find('.prod-column-price');
      var $qtyEl = $thisRow.find('.prod-column-qty');
      var $amountEl = $thisRow.find('.prod-column-amt');
      var uoms = Drupal.settings.avbase.uoms;

      // Trigger actions for autocomplete product field.
      $productIDEl.on('autocompleteSelect', function(e, node) {
        // Get product details if not yet set.
        var productID = $(node).find('div#av-prod-id').html();
        $(this).data('selected-product-id', productID);
        if (!Drupal.settings.avNestableProductForm.products[productID]) {
          Drupal.settings.avNestableProductForm.products[productID] = $.parseJSON($(node).find('div#av-prod-json').html());
        }

        var productDetails = Drupal.settings.avNestableProductForm.products[productID];
        if (productDetails.uom_id) {
          var uom = uoms[productDetails.uom_id];
          $qtyPerUOMEl.val(1);
          $UOMEl.val(uom.title);
          $UOMEl.trigger('change');
        }
      });
      // Trigger adding new row when last Product field is reached.
      $productIDEl.focus(function() {
        if (!$(this).closest('.uk-nestable-item').next().hasClass('uk-nestable-item')) {
          $('#prod-add-btn').trigger('mousedown');
        }
        $(this).select();
      });

      // Trigger actions for UOM select field.
      var $uomWrapperEl = $('#' + ($UOMEl.attr('id') + '-wrapper'));
      var avDropdown = $.UIkit.autocomplete($uomWrapperEl, {minLength: 0});
      var avDropdownData = function(productID) {
        var productDetails = Drupal.settings.avNestableProductForm.products[productID];
        var data = [];
        if (productDetails) {
          var uom = uoms[productDetails.uom_id];
          if (uom) {
            var baseUOMPluralForm = uom.data.plural_form || uom.title;
            data.push({"value": uom.title, 'qtyPerUOM': 1, "title": Drupal.checkPlain(uom.title), "text": Drupal.t('base uom')});
            var otherUOMs = productDetails.data.uoms || {};
            $.each(otherUOMs, function(i, otherUOM) {
              var uom = uoms[otherUOM.uom_id];
              if (uom) {
                data.push({"value": uom.title, 'qtyPerUOM': otherUOM.qty, "title": Drupal.checkPlain(uom.title), "text": Drupal.t('@qty @plural_form per @title', {'@qty': otherUOM.qty, '@plural_form': baseUOMPluralForm.toLowerCase(), '@title': uom.title.toLowerCase()})});
              }
            });
          }
        }
        return data;
      };
      $UOMEl.click(function() {
        if (!avDropdown.visible) {
          avDropdown.render(avDropdownData($productIDEl.data('selected-product-id')));
        }
        else {
          avDropdown.hide();
        }
      });
      $UOMEl.keydown(function(e) {
        if ((e.keyCode === 40) && !avDropdown.visible) {
          // DOWN.
          $UOMEl.trigger('click');
          e.preventDefault();
          e.stopPropagation();
        }
        else if (e.keyCode === 9) {
          avDropdown.select();
        }
      });
      $uomWrapperEl.on('selectitem.uk.autocomplete', function(e, data) {
        $qtyPerUOMEl.val(data.qtyperuom);
        $qtyEl.focus().select();
      });
      $UOMEl.change(function() {
        // Get current row details.
        if (!$productIDEl.length) {
          return;
        }

        // Get selected product details
        var selectedProductID = $productIDEl.data('selected-product-id');
        var productDetails = Drupal.settings.avNestableProductForm.products[selectedProductID] || {};
        if (productDetails.title != $productIDEl.val()) {
          return;
        }

        // Get uom details.
        var uomTitle = $(this).val();
        var uomID = 0;
        var otherUOMs = productDetails.data.uoms || {};
        $.each(uoms, function(i, uom) {
          if (uomTitle.toLowerCase() == uom.title.toLowerCase()) {
            uomID = uom.id;
            return;
          }
        });

        // Auto-fill price fields.
        var price = 0;
        if (uomID == 0) {
          return;
        }
        else if (uomID == productDetails.uom_id) {
          // Selected UOM is the same as base UOM.
          price = parseFloat(productDetails.cost);
          $priceEl.val(price.toFixed(2));
        }
        else if (otherUOMs[uomID]) {
          // Selected UOM is a data UOM.
          price = parseFloat(productDetails.cost) * parseFloat($qtyPerUOMEl.val());
          $priceEl.val(price.toFixed(2));
        }
        else {
          $priceEl.val('');
        }

        $priceEl.trigger('change');
      });

      // Trigger actions for price field.
      $priceEl.change(function() {
        var amount = self.getRowAmount($thisRow);
        if (amount != '') {
          $amountEl.val(amount.toFixed(2));
          self.refreshSubTotalText();
        }
      });
      $priceEl.keydown(function(e) {
        self.switchRowFocus(e, $thisRow);
      });

      // Trigger actions for qty field.
      $qtyEl.change(function() {
        $priceEl.trigger('change');
      });
      $qtyEl.keydown(function(e) {
        self.switchRowFocus(e, $thisRow);
      });

      // Trigger actions for amount field.
      $amountEl.change(function() {
        var price = self.getRowPrice($thisRow);
        if (price != '') {
          $priceEl.val(parseFloat(price.toFixed(6)));
        }

        var thisAmount = $(this).val();
        if ($.isNumeric(thisAmount)) {
          thisAmount = parseFloat(thisAmount).toFixed(2);
          $(this).val(thisAmount);
        }
        self.refreshSubTotalText();
      });
      $amountEl.keydown(function(e) {
        self.switchRowFocus(e, $thisRow);
      });
    });
  };

  /**
   * Compute and return row total.
   */
  Drupal.avbaseNestableProductForm.prototype.getRowAmount = function($row) {
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
  };

  /**
   * Compute and return row price.
   */
  Drupal.avbaseNestableProductForm.prototype.getRowPrice = function($row) {
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
  };

  /**
   * Refresh row numbers on row column.
   */
  Drupal.avbaseNestableProductForm.prototype.refreshRowNumbers = function () {
    this.$wrapper.find('.uk-nestable-item').each(function(index) {
      $(this).find('.av-nestable-row-num').html(index + 1);
    });
  };

  /**
   * Refresh subtotal html text.
   */
  Drupal.avbaseNestableProductForm.prototype.refreshSubTotalText = function () {
    this.$subTotalEl.text(this.moneyFormat(this.getSubTotal()));
    this.refreshDiscountTotalText();
  };

  /**
   * Refresh subtotal html text.
   */
  Drupal.avbaseNestableProductForm.prototype.refreshDiscountTotalText = function () {
    this.$discountTotalEl.text('- ' + this.moneyFormat(this.getDiscountTotal()));
    this.refreshGrandTotalText();
  };

  /**
   * Refresh subtotal html text.
   */
  Drupal.avbaseNestableProductForm.prototype.refreshGrandTotalText = function() {
    this.$grandTotalEl.text(this.moneyFormat(this.getGrandTotal()));
  };

  /**
   * Compute and return subtotal.
   * @returns {string}
   */
  Drupal.avbaseNestableProductForm.prototype.getSubTotal = function() {
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
  };

  /**
   * Compute and return discount total.
   * @returns {string}
   */
  Drupal.avbaseNestableProductForm.prototype.getDiscountTotal = function() {
    var discountType = this.$discountTypeEl.val();
    var discountValue = $('#discount-value').val();
    var discountTotal = 0;
    if (discountType && discountValue && $.isNumeric(discountValue)) {
      if (discountType == 1) {
        var subTotal = this.getSubTotal();
        discountTotal = (discountValue * subTotal) / 100;
      }
      else if(discountType == 2) {
        discountTotal = discountValue;
      }
    }
    return parseFloat(discountTotal).toFixed(2);
  };

  /**
   * Compute and return grand total.
   * @returns {string}
   */
  Drupal.avbaseNestableProductForm.prototype.getGrandTotal = function() {
    var subTotal = this.getSubTotal();
    var discountTotal = this.getDiscountTotal();
    var grandTotal = subTotal - discountTotal;
    grandTotal = grandTotal < 0 ? 0 : grandTotal;
    return parseFloat(grandTotal).toFixed(2);
  };

  /**
   * Switch row focus when up/down on keyboard is used.
   * @param e
   * @param $row
   */
  Drupal.avbaseNestableProductForm.prototype.switchRowFocus = function(e, $row) {
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
  };

  /**
   * Beautify numbers.
   * @param nStr
   * @returns {*}
   */
  Drupal.avbaseNestableProductForm.prototype.moneyFormat = function(nStr) {
    nStr += '';
    x = nStr.split('.');
    x1 = x[0];
    x2 = x.length > 1 ? '.' + x[1] : '';
    var rgx = /(\d+)(\d{3})/;
    while (rgx.test(x1)) {
      x1 = x1.replace(rgx, '$1' + ',' + '$2');
    }
    return x1 + x2;
  };


  //window.avNestableProductForm = {
  //  $nestableEl: null,
  //  $productRows: {},
  //  init: function(options) {
  //    var self = this;
  //    self.$grandTotalEl = $('.product-form-grand-total');
  //    self.$subTotalEl = $('.product-form-sub-total');
  //    self.$discountTotalEl = $('.product-form-discount-total');
  //    self.$discountTypeEl = $('#discount-type');
  //    self.$discountValEl = $('#discount-value');
  //
  //    // Initialize uikit nestable component.
  //    UIkit.nestable(self.$nestableEl, options);
  //
  //
  //
  //    // Do stuff when nestable form is changed.
  //    self.$nestableEl.on('change.uk.nestable', function(e, uikit, el, action) {
  //      if (action == 'moved' || action == 'removed') {
  //        // Recalculate row numbers.
  //        self.refreshRowNumbers();
  //      }
  //    });
  //
  //    self.bindEvents();
  //    self.refreshSubTotalText();
  //  },
  //
  //  refreshRowNumbers: function() {
  //    this.$nestableEl.find('.uk-nestable-item').each(function(index) {
  //      $(this).find('.av-nestable-row-num').html(index + 1);
  //    });
  //  },
  //
  //  bindEvents: function() {
  //    var self = this;
  //
  //    // Trigger actions for discount fields.
  //    self.$discountValEl.once('avPODiscountVal', function() {
  //      $(this).change(function() {
  //        self.refreshDiscountTotalText();
  //      });
  //    });
  //    self.$discountTypeEl.once('avPODiscountType', function() {
  //      $(this).change(function() {
  //        self.$discountValEl.trigger('change');
  //      });
  //    });
  //
  //    this.$productRows = self.$nestableEl.find('.uk-nestable-item');
  //    this.$productRows.once('avNestableProductRows', function() {
  //      $(this).each(function() {
  //        var $thisRow = $(this);
  //        var $productIDEl = $thisRow.find('.prod-column-id');
  //        var $UOMEl = $thisRow.find('.prod-column-uom-id');
  //        var $qtyPerUOMEl = $thisRow.find('.prod-column-qty-per-uom');
  //        var $priceEl = $thisRow.find('.prod-column-price');
  //        var $qtyEl = $thisRow.find('.prod-column-qty');
  //        var $amountEl = $thisRow.find('.prod-column-amt');
  //        var uoms = Drupal.settings.avBase.uoms;
  //
  //        // Trigger actions for autocomplete product field.
  //        $productIDEl.on('autocompleteSelect', function(e, node) {
  //          // Get product details if not yet set.
  //          var productID = $(node).find('div#av-prod-id').html();
  //          $(this).data('selected-product-id', productID);
  //          if (!Drupal.settings.avNestableProductForm.products[productID]) {
  //            Drupal.settings.avNestableProductForm.products[productID] = $.parseJSON($(node).find('div#av-prod-json').html());
  //          }
  //
  //          var productDetails = Drupal.settings.avNestableProductForm.products[productID];
  //          if (productDetails.uom_id) {
  //            var uom = uoms[productDetails.uom_id];
  //            $qtyPerUOMEl.val(1);
  //            $UOMEl.val(uom.title);
  //            $UOMEl.trigger('change');
  //          }
  //        });
  //        // Trigger adding new row when last Product field is reached.
  //        $productIDEl.focus(function() {
  //          if (!$(this).closest('.uk-nestable-item').next().hasClass('uk-nestable-item')) {
  //            $('#prod-add-btn').trigger('mousedown');
  //          }
  //          $(this).select();
  //        });
  //
  //        // Trigger actions for UOM select field.
  //        var $uomWrapperEl = $('#' + ($UOMEl.attr('id') + '-wrapper'));
  //        var avDropdown = $.UIkit.autocomplete($uomWrapperEl, {minLength: 0});
  //        var avDropdownData = function(productID) {
  //          var productDetails = Drupal.settings.avNestableProductForm.products[productID];
  //          var data = [];
  //          if (productDetails) {
  //            var uom = uoms[productDetails.uom_id];
  //            if (uom) {
  //              var baseUOMPluralForm = uom.data.plural_form || uom.title;
  //              data.push({"value": uom.title, 'qtyPerUOM': 1, "title": Drupal.checkPlain(uom.title), "text": Drupal.t('base uom')});
  //              var otherUOMs = productDetails.data.uoms || {};
  //              $.each(otherUOMs, function(i, otherUOM) {
  //                var uom = uoms[otherUOM.uom_id];
  //                if (uom) {
  //                  data.push({"value": uom.title, 'qtyPerUOM': otherUOM.qty, "title": Drupal.checkPlain(uom.title), "text": Drupal.t('@qty @plural_form per @title', {'@qty': otherUOM.qty, '@plural_form': baseUOMPluralForm.toLowerCase(), '@title': uom.title.toLowerCase()})});
  //                }
  //              });
  //            }
  //          }
  //          return data;
  //        };
  //        $UOMEl.click(function() {
  //          if (!avDropdown.visible) {
  //            avDropdown.render(avDropdownData($productIDEl.data('selected-product-id')));
  //          }
  //          else {
  //            avDropdown.hide();
  //          }
  //        });
  //        $UOMEl.keydown(function(e) {
  //          if ((e.keyCode === 40) && !avDropdown.visible) {
  //            // DOWN.
  //            $UOMEl.trigger('click');
  //            e.preventDefault();
  //            e.stopPropagation();
  //          }
  //          else if (e.keyCode === 9) {
  //            avDropdown.select();
  //          }
  //        });
  //        $uomWrapperEl.on('selectitem.uk.autocomplete', function(e, data) {
  //          $qtyPerUOMEl.val(data.qtyperuom);
  //          $qtyEl.focus().select();
  //        });
  //        $UOMEl.change(function() {
  //          // Get current row details.
  //          if (!$productIDEl.length) {
  //            return;
  //          }
  //
  //          // Get selected product details
  //          var selectedProductID = $productIDEl.data('selected-product-id');
  //          var productDetails = Drupal.settings.avNestableProductForm.products[selectedProductID] || {};
  //          if (productDetails.title != $productIDEl.val()) {
  //            return;
  //          }
  //
  //          // Get uom details.
  //          var uomTitle = $(this).val();
  //          var uomID = 0;
  //          var otherUOMs = productDetails.data.uoms || {};
  //          $.each(uoms, function(i, uom) {
  //            if (uomTitle.toLowerCase() == uom.title.toLowerCase()) {
  //              uomID = uom.id;
  //              return;
  //            }
  //          });
  //
  //          // Auto-fill price fields.
  //          var price = 0;
  //          if (uomID == 0) {
  //            return;
  //          }
  //          else if (uomID == productDetails.uom_id) {
  //            // Selected UOM is the same as base UOM.
  //            price = parseFloat(productDetails.cost);
  //            $priceEl.val(price.toFixed(2));
  //          }
  //          else if (otherUOMs[uomID]) {
  //            // Selected UOM is a data UOM.
  //            price = parseFloat(productDetails.cost) * parseFloat($qtyPerUOMEl.val());
  //            $priceEl.val(price.toFixed(2));
  //          }
  //          else {
  //            $priceEl.val('');
  //          }
  //
  //          $priceEl.trigger('change');
  //        });
  //
  //        // Trigger actions for price field.
  //        $priceEl.change(function() {
  //          var amount = self.getRowAmount($thisRow);
  //          if (amount != '') {
  //            $amountEl.val(amount.toFixed(2));
  //            self.refreshSubTotalText();
  //          }
  //        });
  //        $priceEl.keydown(function(e) {
  //          self.switchRowFocus(e, $thisRow);
  //        });
  //
  //        // Trigger actions for qty field.
  //       $qtyEl.change(function() {
  //          $priceEl.trigger('change');
  //        });
  //        $qtyEl.keydown(function(e) {
  //          self.switchRowFocus(e, $thisRow);
  //        });
  //
  //        // Trigger actions for amount field.
  //          $amountEl.change(function() {
  //          var price = self.getRowPrice($thisRow);
  //          if (price != '') {
  //            $priceEl.val(parseFloat(price.toFixed(6)));
  //          }
  //
  //          var thisAmount = $(this).val();
  //          if ($.isNumeric(thisAmount)) {
  //            thisAmount = parseFloat(thisAmount).toFixed(2);
  //            $(this).val(thisAmount);
  //          }
  //          self.refreshSubTotalText();
  //        });
  //        $amountEl.keydown(function(e) {
  //          self.switchRowFocus(e, $thisRow);
  //        });
  //      });
  //    });
  //
  //  },
  //
  //  getRowAmount: function($row) {
  //    var $priceEl = $row.find('.prod-column-price');
  //    var $qtyEl = $row.find('.prod-column-qty');
  //    var qty = Drupal.checkPlain($qtyEl.val());
  //    var price = Drupal.checkPlain($priceEl.val());
  //
  //    var error = false;
  //    $priceEl.removeClass('uk-form-danger');
  //    $qtyEl.removeClass('uk-form-danger');
  //    if (!$.isNumeric(price)) {
  //      if (price != '') {
  //        $priceEl.addClass('uk-form-danger');
  //      }
  //      error = true;
  //    }
  //    if (!$.isNumeric(qty)) {
  //      if (qty != '') {
  //        $qtyEl.addClass('uk-form-danger');
  //      }
  //      error = true;
  //    }
  //
  //    if (error) {
  //      return '';
  //    }
  //
  //    var amount = qty * price;
  //    return amount;
  //  },
  //
  //  getRowPrice: function($row) {
  //    var $amountEl = $row.find('.prod-column-amt');
  //    var $qtyEl = $row.find('.prod-column-qty');
  //    var qty = Drupal.checkPlain($qtyEl.val());
  //    var amount = Drupal.checkPlain($amountEl.val());
  //
  //    var error = false;
  //    $amountEl.removeClass('uk-form-danger');
  //    $qtyEl.removeClass('uk-form-danger');
  //    if (!$.isNumeric(amount)) {
  //      if (amount != '') {
  //        $amountEl.addClass('uk-form-danger');
  //      }
  //      error = true;
  //    }
  //    if (!$.isNumeric(qty)) {
  //      if (qty != '') {
  //        $qtyEl.addClass('uk-form-danger');
  //      }
  //      error = true;
  //    }
  //
  //    if (error) {
  //      return '';
  //    }
  //
  //    var price = amount / qty;
  //    return price;
  //  },
  //
  //  refreshSubTotalText: function() {
  //    this.$subTotalEl.text(this.moneyFormat(this.getSubTotal()));
  //    this.refreshDiscountTotalText();
  //  },
  //
  //  refreshDiscountTotalText: function() {
  //    this.$discountTotalEl.text('- ' + this.moneyFormat(this.getDiscountTotal()));
  //    this.refreshGrandTotalText();
  //  },
  //
  //  refreshGrandTotalText: function() {
  //    this.$grandTotalEl.text(this.moneyFormat(this.getGrandTotal()));
  //  },
  //
  //  getSubTotal: function() {
  //    var $amountEls = this.$productRows.find('.prod-column-amt');
  //    var rowTotal = 0;
  //    var subTotal = 0;
  //    $amountEls.each(function() {
  //      rowTotal = $(this).val();
  //      if (!$.isNumeric(rowTotal)) {
  //        return;
  //      }
  //      subTotal += parseFloat(rowTotal);
  //    });
  //
  //    return parseFloat(subTotal).toFixed(2);
  //  },
  //
  //  getDiscountTotal: function() {
  //    var discountType = this.$discountTypeEl.val();
  //    var discountValue = $('#discount-value').val();
  //    var discountTotal = 0;
  //    if (discountType && discountValue && $.isNumeric(discountValue)) {
  //      if (discountType == 1) {
  //        var subTotal = this.getSubTotal();
  //        discountTotal = (discountValue * subTotal) / 100;
  //      }
  //      else if(discountType == 2) {
  //        discountTotal = discountValue;
  //      }
  //    }
  //    return parseFloat(discountTotal).toFixed(2);
  //  },
  //
  //  getGrandTotal: function() {
  //    var subTotal = this.getSubTotal();
  //    var discountTotal = this.getDiscountTotal();
  //    var grandTotal = subTotal - discountTotal;
  //    grandTotal = grandTotal < 0 ? 0 : grandTotal;
  //    return parseFloat(grandTotal).toFixed(2);
  //  },
  //
  //  switchRowFocus: function(e, $row) {
  //    var columnName = $(e.target).data('column-name');
  //    if (e.keyCode === 38) {
  //      // UP.
  //      var $prevRow = $row.closest('.uk-nestable-item').prev();
  //      if ($prevRow.hasClass('uk-nestable-item')) {
  //        $prevRow.find('.prod-column-' + columnName).focus().select();
  //        e.preventDefault();
  //        e.stopPropagation();
  //      }
  //    }
  //    else if(e.keyCode === 40) {
  //      // DOWN.
  //      var $nextRow = $row.closest('.uk-nestable-item').next();
  //      if ($nextRow.hasClass('uk-nestable-item')) {
  //        $nextRow.find('.prod-column-' + columnName).focus().select();
  //        e.preventDefault();
  //        e.stopPropagation();
  //      }
  //    }
  //  },
  //
  //  moneyFormat: function(nStr) {
  //    nStr += '';
  //    x = nStr.split('.');
  //    x1 = x[0];
  //    x2 = x.length > 1 ? '.' + x[1] : '';
  //    var rgx = /(\d+)(\d{3})/;
  //    while (rgx.test(x1)) {
  //      x1 = x1.replace(rgx, '$1' + ',' + '$2');
  //    }
  //    return x1 + x2;
  //  }
  //};
}(jQuery));

jQuery(document).ready(function ($) {
  //console.log(Drupal.settings);
});