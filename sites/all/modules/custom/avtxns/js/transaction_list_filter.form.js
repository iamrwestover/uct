(function ($) {
  /**
   * Behaviors of entity overview form.
   * @type {{attach: Function}}
   */
  Drupal.behaviors.avtxnsTransactionListForm = {
    attach: function (context, settings) {
      new Drupal.avtxnsTransactionListForm();
    }
  };

  Drupal.avtxnsTransactionListForm = function() {
    var self = this;
    self.$searchBtn = $('#transaction-search-btn');
    $('#avtxns-txn-list-filter-form').once('avtxnsTransactionListForm', function() {
      var $thisForm = $(this);
      $thisForm.on('keyup keypress', function(e) {
        var keyCode = e.keyCode || e.which;
        if (keyCode === 13) {
          e.preventDefault();
          return false;
        }
      });

      self.$searchEls = $(this).find('.trigger-ajax-search');
      self.$resetBtn = $(this).find('#transaction-reset-btn');
      self.searchEvents();

      // Date auto field.
      self.$dateAutoEl = $(this).find('#date-auto');
      self.$dateFromEl = $(this).find('#date-from');
      self.$dateToEl = $(this).find('#date-to');
      self.$dateAutoEl.change(function () {
        if ($(this).val() == 'custom') {
          return;
        }
        var avbase = Drupal.settings.avbase || {};
        var dateAuto = avbase.date_auto || {};
        var dateAutoDetails = dateAuto[$(this).val()] || {};
        self.$dateFromEl.val(dateAutoDetails.date_from || '');
        self.$dateToEl.val(dateAutoDetails.date_to || '');
        self.$searchBtn.trigger('mousedown');
      });

      self.$dateFromEl.change(function () {
        self.$dateAutoEl.val('custom');
      });
      self.$dateToEl.change(function () {
        self.$dateAutoEl.val('custom');
      });
    });

    self.pagerLinkEvents();


  };

  /**
   * Bind events to search feature.
   */
  Drupal.avtxnsTransactionListForm.prototype.searchEvents = function () {
    var self = this;
    var typingTimer;
    var doneTypingInterval = 500;

    self.$searchEls.on('change', function () {
      if ($(this).hasClass('trigger-search-on-keyup') || $(this).hasClass('form-autocomplete')) {
        return;
      }
      self.$searchBtn.trigger('mousedown');
    });

    // For autocomplete fields.
    self.$searchEls.on('blur', function () {
      if (!$(this).hasClass('form-autocomplete')) {
        return;
      }
      self.$searchBtn.trigger('mousedown');
    });

    //self.$searchEls.on('click', function () {
    //  if ($(this).hasClass('trigger-search-on-click')) {
    //    return;
    //  }
    //  self.$searchBtn.trigger('mousedown');
    //});

    self.$searchEls.on('keyup', function () {
      if (!$(this).hasClass('trigger-search-on-keyup')) {
        return;
      }
      clearTimeout(typingTimer);
      typingTimer = setTimeout(function() {
        self.$searchBtn.trigger('mousedown');
      }, doneTypingInterval);
    });

    //self.$searchEls.on('autocompleteSelect', function(e, node) {
    //  self.$searchBtn.trigger('mousedown');
    //});

    self.$resetBtn.click(function(e) {
      self.$searchEls.each(function() {
        if ($(this).prop('readonly')) {
          $(this).val($(this).attr('value'));
          return;
        }
        if ($(this).attr('type') == 'checkbox') {
          $(this).prop('checked', function () {
            return this.getAttribute('checked') == 'checked';
          });
        }
        else if ($(this).attr('type') == 'radio') {
          $(this).prop('checked', function () {
            return this.getAttribute('checked') == 'checked';
          });
        }
        else if ($(this).prop('type') == 'select-one') {
          $(this).find('option').prop('selected', function() {
            return this.defaultSelected;
          });
        }
        else {
          $(this).val($(this).attr('value'));
        }
      });

      self.$dateAutoEl.find('option').prop('selected', function() {
        return this.defaultSelected;
      });
      self.$searchBtn.trigger('mousedown');
      e.preventDefault();
    });
  };

  /**
   * Bind events to pager links feature.
   */
  Drupal.avtxnsTransactionListForm.prototype.pagerLinkEvents = function () {
    var self = this;
    var ajax = Drupal.ajax[self.$searchBtn.attr('id')] || {};
    self.$targetWrapper = $(ajax.wrapper);
    self.$targetWrapper.once('avtxnsTransactionListForm', function() {
      $(this).find('.uk-pagination a, th a').click(function(e) {
        var href = $(this).attr('href');
        $('#transaction-list-table-href').val(href);
        self.$searchBtn.trigger('mousedown');
        e.preventDefault();
      });
    });
  };

}(jQuery));
