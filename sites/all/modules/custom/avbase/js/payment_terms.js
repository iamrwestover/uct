
(function ($) {

  Drupal.behaviors.avbasePaymentTermsJS = {
    attach: function (context, settings) {
      $('select.avbase-payment-terms-js', context).once('avbasePaymentTermsJS', function () {
        settings.avbase = settings.avbase || {};
        var $input = $(this);
        new Drupal.avbasePaymentTermsJS($input, settings.avbase);
      });
    }
  };

  Drupal.avbasePaymentTermsJS = function ($input, avbase) {
    var self = this;
    self.preventChildrenHide = $input.data('prevent-children-hide');
    self.$termEl = $input;
    self.$discountTypeEl = $('#discount-type');
    self.$discountValEl = $('#discount-value');

    if (self.$termEl.val() === 0) {
      self.toggleChildrenVisibility();
    }

    self.bindEvents(avbase);
  };

  Drupal.avbasePaymentTermsJS.prototype.bindEvents = function (avbase) {
    var self = this;
    self.$termEl.change(function() {
      var paymentTerms = avbase.paymentTerms || {};
      var vendors = avbase.vendors || {};
      var selectedVendorID = $(this).data('selectedVendorID');
      var vendor = vendors[selectedVendorID] || {};

      var termID = $(this).val();
      var term = paymentTerms[termID] || {};
      if (term.id) {
        var type;
        var val;
        if (term.id == (vendor.term_id)) {
          type = vendor.discount_type || 1;
          val = vendor.discount_value || '';
        }
        else {
          var term_data = term['data'] || {};
          type = term_data['discount_type'] || 1;
          val = term_data['discount_value'] || '';
        }

        if (val != '') {
          val = parseFloat(val);
          val = parseFloat(val.toFixed(2));
        }
        self.$discountTypeEl.val(type);
        self.$discountValEl.val(val);
        self.$discountValEl.trigger('change');
        self.toggleChildrenVisibility(true);
      }
      else {
        self.toggleChildrenVisibility();
      }
    });
  };

  Drupal.avbasePaymentTermsJS.prototype.toggleChildrenVisibility = function(visible) {
    if (visible) {
      this.$discountTypeEl.closest('div').show();
      this.$discountValEl.closest('div').show();
    }
    else if(!this.preventChildrenHide) {
      this.$discountTypeEl.closest('div').hide();
      this.$discountValEl.closest('div').hide();
    }
  };

  //window.avPaymentTermActions = {
  //  preventChildrenHide: false,
  //  $termEl: $('#term-id'),
  //  $discountTypeEl: $('#discount-type'),
  //  $discountValEl: $('#discount-value'),
  //
  //  init: function() {
  //    var superParent = this;
  //    if (superParent.$termEl.val() === 0) {
  //      superParent.toggleChildrenVisibility();
  //    }
  //
  //    superParent.bindEvents();
  //  },
  //
  //  bindEvents: function() {
  //    var superParent = this;
  //    superParent.$termEl.change(function() {
  //      var paymentTerms = Drupal.settings.avbasePaymentTerms || {};
  //      var avbase = Drupal.settings.avbase || {};
  //      var vendors = avbase.vendors || {};
  //      var selectedVendorID = $(this).data('selectedVendorID');
  //      var vendor = vendors[selectedVendorID] || {};
  //
  //      var termID = $(this).val();
  //      var term = paymentTerms[termID] || {};
  //      if (term.id) {
  //        var type;
  //        var val;
  //        if (term.id == (vendor.term_id)) {
  //          type = vendor.discount_type || 1;
  //          val = vendor.discount_value || '';
  //        }
  //        else {
  //          var term_data = term['data'] || {};
  //          type = term_data['discount_type'] || 1;
  //          val = term_data['discount_value'] || '';
  //        }
  //
  //        if (val != '') {
  //          val = parseFloat(val);
  //          val = parseFloat(val.toFixed(2));
  //        }
  //        superParent.$discountTypeEl.val(type);
  //        superParent.$discountValEl.val(val);
  //        superParent.$discountValEl.trigger('change');
  //        superParent.toggleChildrenVisibility(true);
  //      }
  //      else {
  //        superParent.toggleChildrenVisibility();
  //      }
  //    });
  //  },
  //
  //  toggleChildrenVisibility: function(visible) {
  //    if (visible) {
  //      this.$discountTypeEl.closest('div').show();
  //      this.$discountValEl.closest('div').show();
  //    }
  //    else if(!this.preventChildrenHide) {
  //      this.$discountTypeEl.closest('div').hide();
  //      this.$discountValEl.closest('div').hide();
  //    }
  //  }
  //};

}(jQuery));

jQuery(document).ready(function ($) {

});
