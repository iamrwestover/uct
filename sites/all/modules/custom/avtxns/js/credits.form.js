(function ($) {

  Drupal.behaviors.avtxnsCreditsForm = {
    attach: function (context, settings) {
      $('#avtxns-txn-credits-form', context).once('avtxnsCreditsForm', function() {
        new Drupal.avtxnsCreditsForm($(this), settings);
      });
    }
  };

  Drupal.avtxnsCreditsForm = function ($form, settings) {
    this.$totalCreditsUsedEl = $('#total-credits-used');
    this.$billTotalEl = $("input[name='bill_total']");
    this.$billTotalTextEl = $('#bill-total-text');

    this.bindEvents($form);
    this.refreshSubTotalText();
  };

  Drupal.avtxnsCreditsForm.prototype.bindEvents = function ($form) {
    var self = this;
    self.$creditRows = $form.find('.av-credit-row');
    self.$creditRows.once('avtxnsCreditsForm', function () {
      var $thisRow = $(this);
      var $amtToUseEl = $thisRow.find('.amt-to-use');
      var $tickEl = $thisRow.find('.tick-box');

      $amtToUseEl.change(function (e) {
        self.refreshSubTotalText(e, $thisRow);
      });
      $amtToUseEl.keydown(function (e) {
        self.switchRowFocus(e, $thisRow);
      });

      // Tick box.
      $tickEl.change(function () {
        var ticked = $(this).is(":checked");
        var tickedAmount = '';
        if (ticked) {
          var billTotal = self.$billTotalEl.val();
          var subTotal = self.getSubTotal();
          var remainingBalance = billTotal - subTotal;
          var openBalance = $(this).data('open-balance');
          tickedAmount = remainingBalance > openBalance ? openBalance : remainingBalance;
          tickedAmount = parseFloat(tickedAmount).toFixed(2);
        }
        $amtToUseEl.val(tickedAmount);
        $amtToUseEl.trigger('change');
      });
    });
  };

  /**
   * Compute and return subtotal.
   * @returns {string}
   */
  Drupal.avtxnsCreditsForm.prototype.getSubTotal = function() {
    var $totalEls = this.$creditRows.find('.amt-to-use');
    var rowTotal = 0;
    var subTotal = 0;
    $totalEls.each(function() {
      rowTotal = $(this).val();

      if (!$.isNumeric(rowTotal)) {
        return;
      }
      subTotal += parseFloat(rowTotal);
    });

    subTotal = parseFloat(subTotal);
    return subTotal;
  };

  /**
   * Refresh subtotal html text.
   */
  Drupal.avtxnsCreditsForm.prototype.refreshSubTotalText = function () {
    var subTotal = this.getSubTotal();
    this.$totalCreditsUsedEl.text(this.moneyFormat(parseFloat(subTotal).toFixed(2)));

    var billTotal = parseFloat(this.$billTotalEl.val());
    var newBillTotal = billTotal.toFixed(2) - parseFloat(subTotal).toFixed(2);
    this.$billTotalTextEl.text(this.moneyFormat(newBillTotal.toFixed(2)));
  };

  /**
   * Switch row focus when up/down on keyboard is used.
   * @param e
   * @param $row
   */
  Drupal.avtxnsCreditsForm.prototype.switchRowFocus = function(e, $row) {
    var columnName = $(e.target).data('column-name');
    if (e.keyCode === 38) {
      // UP.
      var $prevRow = $row.closest('.av-credit-row').prev();
      if ($prevRow.hasClass('av-credit-row')) {
        $prevRow.find('.' + columnName).focus().select();
        e.preventDefault();
        e.stopPropagation();
      }
    }
    else if(e.keyCode === 40) {
      // DOWN.
      var $nextRow = $row.closest('.av-credit-row').next();
      if ($nextRow.hasClass('av-credit-row')) {
        $nextRow.find('.' + columnName).focus().select();
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
  Drupal.avtxnsCreditsForm.prototype.moneyFormat = function(nStr) {
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
