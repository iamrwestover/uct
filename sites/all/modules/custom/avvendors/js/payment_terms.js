
(function ($) {
  window.avPaymentTermActions = {
    preventChildrenHide: false,
    $termEl: $('#term-id'),
    $discountTypeEl: $('#discount-type'),
    $discountValEl: $('#discount-value'),

    init: function() {
      var superParent = this;
      if (superParent.$termEl.val() == 0) {
        superParent.toggleChildrenVisibility();
      }

      superParent.bindEvents();
    },

    bindEvents: function() {
      var superParent = this;
      superParent.$termEl.change(function() {
        var paymentTerms = Drupal.settings.avbasePaymentTerms || {};
        var vendors = Drupal.settings.avbase.vendors || {};
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
          superParent.$discountTypeEl.val(type);
          superParent.$discountValEl.val(val);
          superParent.$discountValEl.trigger('change');
          superParent.toggleChildrenVisibility(true);
        }
        else {
          superParent.toggleChildrenVisibility();
        }
      });
    },

    toggleChildrenVisibility: function(visible) {
      if (visible) {
        this.$discountTypeEl.closest('div').show();
        this.$discountValEl.closest('div').show();
      }
      else if(!this.preventChildrenHide) {
        this.$discountTypeEl.closest('div').hide();
        this.$discountValEl.closest('div').hide();
      }
    }
  };

}(jQuery));

jQuery(document).ready(function ($) {

});
