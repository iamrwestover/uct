
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
    self.$clientEl = $('#client-id');
    self.client = {a:'a'};

    if (self.$termEl.val() === 0) {
      self.toggleChildrenVisibility();
    }
    self.bindEvents(avbase);
  };

  Drupal.avbasePaymentTermsJS.prototype.bindEvents = function (avbase) {
    var self = this;
    self.$clientEl.on('autocompleteSelect', function(e, node) {
      var entityGroupName = $(this).data('avbase-entity-group') || '';
      var clientID = $(this).data('selected-row-id') || '';
      var clients = Drupal.settings.avbase[entityGroupName] || {};
      self.client = clients[clientID] || {};

      self.$termEl.val((self.client.term_id || 0));
      self.$termEl.trigger('change');
    });

    self.$termEl.change(function() {
      var paymentTerms = avbase.paymentTerms || {};
      var termID = $(this).val();
      var term = paymentTerms[termID] || {};
      if (term.id) {
        var type;
        var val;
        if (term.id == (self.client.term_id)) {
          type = self.client.discount_type || 1;
          val = self.client.discount_value || '';
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
  //      var clients = avbase.clients || {};
  //      var selectedClientID = $(this).data('selectedClientID');
  //      var client = clients[selectedClientID] || {};
  //
  //      var termID = $(this).val();
  //      var term = paymentTerms[termID] || {};
  //      if (term.id) {
  //        var type;
  //        var val;
  //        if (term.id == (client.term_id)) {
  //          type = client.discount_type || 1;
  //          val = client.discount_value || '';
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