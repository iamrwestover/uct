(function ($) {

  //Drupal.ajax.prototype.commands.enoughQtyValidationResult = function (ajax, response) {
  //  console.log(response);
  //  var $qtyEl = $('#product-qty-' + response.triggerEl['#item_row_key']);
  //  $qtyEl.prop('title', '');
  //  if (!response.itemID) {
  //    return;
  //  }
  //
  //  $qtyEl.prop('title', 'Available: ' + response.availableQty);
  //  $qtyEl.removeClass('uk-form-danger');
  //  if (!response.valid) {
  //    $qtyEl.addClass('uk-form-danger');
  //  }
  //
  //  //$qtyEl.trigger('mouseenter');
  //};

  /**
   * An avbaseNestableProductForm object.
   * @param $wrapper
   * @param settings
   */
  Drupal.avbaseNestableProductForm = function ($wrapper, settings) {
    var self = this;
    //$wrapper.once('avNestableProductForm', function() {
    //
    //
    //});

    // Initialize uikit nestable component.
    UIkit.nestable($wrapper, {handleClass:'uk-nestable-handle', maxDepth: 1});
    //Drupal.settings.avNestableProductForm = Drupal.settings.avNestableProductForm || {};
    Drupal.settings.avbase = Drupal.settings.avbase || {};
    Drupal.settings.avbase.products = Drupal.settings.avbase.products || {};


    // Do stuff when nestable form is changed.
    $wrapper.on('change.uk.nestable', function(e, uikit, el, action) {
      if (action == 'moved' || action == 'removed') {
        // Recalculate row numbers.
        self.refreshRowNumbers();
      }
    });


    self.$wrapper = $wrapper;
    self.$grandTotalEl = $('.product-form-grand-total');
    self.$subTotalEl = $('.product-form-sub-total');
    self.$discountTotalEl = $('.product-form-discount-total');
    self.$discountTypeEl = $('#discount-type');
    self.$discountValEl = $('#discount-value');


    self.bindEvents();
    self.refreshSubTotalText();

    // Make sure newly added rows gets bound with events and delete btn refreshes totals.
    $(document).ajaxComplete(function(event, xhr, settings) {
      var extraData = settings.extraData || {};
      var triggerName = extraData._triggering_element_name || '';
      if (triggerName == 'prod_add_btn') {
        self.bindEvents();
        self.refreshRowNumbers();
      }
      else if (~triggerName.indexOf('prod_delete_btn')) {
        //self.$productRows = self.$wrapper.find('.uk-nestable-item');
        self.refreshRowNumbers();
        self.refreshSubTotalText();
      }
    });
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

    self.$productRows = this.$wrapper.find('.uk-nestable-item');
    self.$productRows.once('avNestableProductRows', function() {
      var $thisRow = $(this);
      var $productTitleEl = $thisRow.find('.prod-column-title');
      var $UOMEl = $thisRow.find('.prod-column-uom-title');
      var $qtyPerUOMEl = $thisRow.find('.prod-column-qty-per-uom');
      var $costEl = $thisRow.find('.prod-column-cost');
      var $qtyEl = $thisRow.find('.prod-column-qty');
      //var $qtyCheckEl = $thisRow.find('.prod-column-qty-check');
      var $discountEl = $thisRow.find('.prod-column-discount');
      var $totalEl = $thisRow.find('.prod-column-total');
      var uoms = Drupal.settings.avbase.uoms;

      // Trigger actions for autocomplete product field.
      $productTitleEl.on('autocompleteSelect', function(e, node) {
        // Get product details if not yet set.
        var productID = $(node).find('div#av-prod-id').html();
        $(this).data('selected-product-id', productID);
        if (!Drupal.settings.avbase.products[productID]) {
          Drupal.settings.avbase.products[productID] = $.parseJSON($(node).find('div#av-prod-json').html());
        }

        var productDetails = Drupal.settings.avbase.products[productID];
        if (productDetails.uom_id) {
          var uom = uoms[productDetails.uom_id];
          $qtyPerUOMEl.val(1);
          $UOMEl.val(uom.title);
          //$qtyCheckEl.trigger('change');
          $UOMEl.trigger('change');
        }
      });
      // Trigger adding new row when last Product field is reached.
      $productTitleEl.focus(function() {
        if (!$(this).closest('.uk-nestable-item').next().hasClass('uk-nestable-item')) {
          $('#prod-add-btn').trigger('click');
        }
        //$(this).select();
      });

      // Trigger actions for UOM select field.
      var $uomWrapperEl = $('#' + ($UOMEl.attr('id') + '-wrapper'));
      var avDropdown = $.UIkit.autocomplete($uomWrapperEl, {minLength: 0});
      var avDropdownData = function(productID) {
        var productDetails = Drupal.settings.avbase.products[productID];
        var data = [];
        if (productDetails) {
          var uom = uoms[productDetails.uom_id];
          if (uom) {
            var baseUOMPluralForm = uom.data.plural_form || uom.title;
            data.push({"value": uom.title, 'qtyPerUOM': 1, "title": Drupal.checkPlain(uom.title), "text": Drupal.t('base uom')});
            var productData = productDetails.data || {};
            var otherUOMs = productData.uoms || {};
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
          avDropdown.render(avDropdownData($productTitleEl.data('selected-product-id')));
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
        if (!$productTitleEl.length) {
          return;
        }

        // Get selected product details
        var selectedProductID = $productTitleEl.data('selected-product-id');
        var productDetails = Drupal.settings.avbase.products[selectedProductID] || {};
        if (productDetails.title != $productTitleEl.val()) {
          return;
        }

        // Get uom details.
        var uomTitle = $(this).val();
        var uomID = 0;
        var productData = productDetails.data || {};
        var otherUOMs = productData.uoms || {};
        $.each(uoms, function(i, uom) {
          if (uomTitle.toLowerCase() == uom.title.toLowerCase()) {
            uomID = uom.id;
            return;
          }
        });

        var transaction = Drupal.settings.avtxns.transaction || '';
        var itemCost = productDetails.cost;
        if (transaction == 'sales') {
          itemCost = productDetails.price;
        }
        // Auto-fill cost fields.
        var cost = 0;
        if (uomID == 0) {
          return;
        }
        else if (uomID == productDetails.uom_id) {
          // Selected UOM is the same as base UOM.
          cost = parseFloat(itemCost);
          $costEl.val(cost.toFixed(2));
        }
        else if (otherUOMs[uomID]) {
          // Selected UOM is a data UOM.
          if (transaction == 'sales' && $.isNumeric(otherUOMs[uomID].price)) {
            cost = parseFloat(otherUOMs[uomID].price);

          }
          else {
            cost = parseFloat(itemCost) * parseFloat($qtyPerUOMEl.val());
          }
          $costEl.val(cost.toFixed(2));
        }
        else {
          $costEl.val('');
        }

        $qtyEl.trigger('change');
      });

      // Trigger actions for cost field.
      $costEl.change(function() {
        var total = self.getRowTotal($thisRow);
        if ($(this).data('is-negative')) {
          total = total * -1;
        }

        if (total !== '') {
          $totalEl.val(total.toFixed(2));
          self.refreshSubTotalText();
        }
      });
      $costEl.keydown(function(e) {
        self.switchRowFocus(e, $thisRow);
      });

      // Trigger actions for qty field.
      $qtyEl.change(function() {
        // Compute costs.
        $costEl.trigger('change');

        // Check if item qty is valid and not greater than user available qty.
        var itemID = $productTitleEl.data('selected-product-id');
        if (!itemID) {
          return;
        }

        var request = $.ajax({
          url: Drupal.settings.basePath + 'av/transactions/qty-check-and-reserve',
          method: 'POST',
          data: {entered_item_id: itemID, entered_qty: $(this).val(), entered_qty_per_uom: $qtyPerUOMEl.val()},
          dataType: 'json',
          beforeSend: function() {
            // Reset tooltip.
            $qtyEl.prop('title', '<i class="uk-icon-refresh uk-icon-spin uk-margin-small-right"></i>checking available qty');
            $qtyEl.addClass('qty-checking');
          },
          complete: function() {
            $qtyEl.removeClass('qty-checking');
            if ($qtyEl.is(':focus')) {
              $qtyEl.trigger('mouseenter');
            }
          }
        });
        request.done(function(response) {
          Drupal.settings.avbase.availableQty = Drupal.settings.avbase.availableQty || {};
          if ($.isNumeric(response.user_available)) {
            Drupal.settings.avbase.availableQty[itemID] = response.user_available;
          }
        });
        request.fail(function(jqXHR, textStatus) {
          alert('Request failed: ' + textStatus );
        });
      });
      $qtyEl.focus(function() {
        $(this).trigger('mouseenter');
      });
      // Mouse enter.
      $qtyEl.mouseenter(function() {
        $(this).removeClass('uk-form-danger');
        if ($(this).hasClass('qty-checking')) {
          return;
        }
        var totalEnteredBaseQty = 0;
        var itemID = $productTitleEl.data('selected-product-id');
        //var matchingQtyEls = [];
        self.$productRows.each(function() {
          if ($(this).find('.prod-column-title').data('selected-product-id') == itemID) {
            var $thisQtyEl = $(this).find('.prod-column-qty');
            var enteredQty = $thisQtyEl.val();
            var enteredQtyPerUOM = $(this).find('.prod-column-qty-per-uom').val();
            if ($.isNumeric(enteredQty) && $.isNumeric(enteredQtyPerUOM)) {
              totalEnteredBaseQty += enteredQty * enteredQtyPerUOM;
              //matchingQtyEls.push($thisQtyEl);
              //$thisQtyEl.addClass('qty-check-tagged');
              //$thisQtyEl.addClass('uk-text-muted');
              //$thisQtyEl.removeClass('prod-column-qty');
              //console.log($thisQtyEl);
            }
          }
        });

        var qtyPerUOM = $qtyPerUOMEl.val();
        var UOMTitle = $UOMEl.val();
        //var $taggedQtyEls = $('.qty-check-tagged');
        //console.log($(this).find('.prod-column-qty'));
        //console.log($taggedQtyEls);
        if ($.isNumeric(qtyPerUOM)) {
          Drupal.settings.avbase.availableQty = Drupal.settings.avbase.availableQty || {};
          var availableQty = Drupal.settings.avbase.availableQty[itemID] || 0;
          availableQty = Math.floor((availableQty - totalEnteredBaseQty) / qtyPerUOM);


          //$taggedQtyEls.removeClass('uk-form-danger');
          //console.log(availableQty);
          if (availableQty < 0) {
            //$taggedQtyEls.addClass('uk-form-danger');
            $(this).addClass('uk-form-danger');
            availableQty = 0;
          }
          $(this).prop('title', availableQty + ' ' + UOMTitle);
        }
        //$taggedQtyEls.removeClass('qty-check-tagged');
      });

      // Key up.
      var typingTimer;
      var doneTypingInterval = 500;
      $qtyEl.keyup(function() {
        clearTimeout(typingTimer);
        typingTimer = setTimeout(function () {
          if ($qtyEl.is(':focus')) {
            $qtyEl.trigger('mouseenter');
          }
        }, doneTypingInterval);
      });
      $qtyEl.keydown(function(e) {
        self.switchRowFocus(e, $thisRow);
      });

      // Trigger actions for qty field.
      $discountEl.change(function() {
        $costEl.trigger('change');
      });
      //$discountEl.keydown(function(e) {
      //  self.switchRowFocus(e, $thisRow);
      //});

      // Trigger actions for total field.
      $totalEl.change(function() {
        //var cost = self.getRowPrice($thisRow);
        //if (cost != '') {
        //  $costEl.val(parseFloat(cost.toFixed(6)));
        //}

        var thisTotal = $(this).val();
        if ($.isNumeric(thisTotal)) {
          thisTotal = parseFloat(thisTotal).toFixed(2);
          $(this).val(thisTotal);
        }
        self.refreshSubTotalText();
      });
      $totalEl.keydown(function(e) {
        self.switchRowFocus(e, $thisRow);
      });

      $discountEl.keydown(function(e) {
        self.switchRowFocus(e, $thisRow);
      });
    });
  };

  /**
   * Compute and return row total.
   */
  Drupal.avbaseNestableProductForm.prototype.getRowTotal = function($row) {
    var $costEl = $row.find('.prod-column-cost');
    var $qtyEl = $row.find('.prod-column-qty');
    var $discountEl = $row.find('.prod-column-discount');
    var qty = Drupal.checkPlain($qtyEl.val());
    var cost = Drupal.checkPlain($costEl.val());
    var discountVal = $discountEl.length ? Drupal.checkPlain($discountEl.val()) : '';
    var discount = [];

    var error = false;
    $costEl.removeClass('uk-form-danger');
    $qtyEl.removeClass('uk-form-danger');
    $discountEl.removeClass('uk-form-danger');
    if (!$.isNumeric(cost)) {
      if (cost != '') {
        $costEl.addClass('uk-form-danger');
      }
      error = true;
    }
    if (!$.isNumeric(qty)) {
      if (qty != '') {
        $qtyEl.addClass('uk-form-danger');
      }
      error = true;
    }

    if (discountVal && discountVal != '') {
      discount = discountVal.split('/');
      $.each(discount, function (i, v) {
        if (!$.isNumeric(v)) {
          $discountEl.addClass('uk-form-danger');
          error = true;
        }
        else {
          discount[i] = parseFloat(v);
        }
      });
      // Replace discount element value with a cleaner version (no spaces).
      $discountEl.val(discount.join('/'));
    }

    if (error) {
      return '';
    }

    var total = parseInt(qty) * parseFloat(cost);
    $.each(discount, function(i, v) {
      total = total - ((total *  v) / 100);
    });
    return total;
  };

  /**
   * Compute and return row cost.
   */
  Drupal.avbaseNestableProductForm.prototype.getRowPrice = function($row) {
    var $totalEl = $row.find('.prod-column-total');
    var $qtyEl = $row.find('.prod-column-qty');
    var qty = Drupal.checkPlain($qtyEl.val());
    var total = Drupal.checkPlain($totalEl.val());

    var error = false;
    $totalEl.removeClass('uk-form-danger');
    $qtyEl.removeClass('uk-form-danger');
    if (!$.isNumeric(total)) {
      if (total != '') {
        $totalEl.addClass('uk-form-danger');
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

    var cost = total / qty;
    return cost;
  };

  /**
   * Refresh row numbers on row column.
   */
  Drupal.avbaseNestableProductForm.prototype.refreshRowNumbers = function () {
    this.$productRows = this.$wrapper.find('.uk-nestable-item');
    this.$productRows.each(function(index) {
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
    var $totalEls = this.$productRows.find('.prod-column-total');
    var rowTotal = 0;
    var subTotal = 0;
    $totalEls.each(function() {
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
    //grandTotal = grandTotal < 0 ? 0 : grandTotal;
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
}(jQuery));