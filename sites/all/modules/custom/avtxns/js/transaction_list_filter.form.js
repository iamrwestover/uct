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
      self.$searchEls = $(this).find('.trigger-ajax-search');
      self.$resetBtn = $(this).find('#transaction-reset-btn');
      self.searchEvents();
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
      if ($(this).hasClass('trigger-search-on-keyup')) {
        return;
      }
      self.$searchBtn.trigger('click');
    });

    //self.$searchEls.on('click', function () {
    //  if ($(this).hasClass('trigger-search-on-click')) {
    //    return;
    //  }
    //  self.$searchBtn.trigger('click');
    //});

    self.$searchEls.on('keyup', function () {
      if (!$(this).hasClass('trigger-search-on-keyup')) {
        return;
      }
      clearTimeout(typingTimer);
      typingTimer = setTimeout(function() {
        self.$searchBtn.trigger('click');
      }, doneTypingInterval);
    });

    self.$searchEls.on('autocompleteSelect', function(e, node) {
      self.$searchBtn.trigger('click');
    });

    self.$resetBtn.click(function(e) {
      self.$searchEls.each(function() {
        if ($(this).prop('readonly')) {
          $(this).val($(this).attr('value'));
          return;
        }
        if ($(this).attr('type') == 'checkbox') {
          //$(this).prop('checked', '');
          //if ($(this).attr('checked')) {
          //  $(this).prop('checked', 'checked');
          //}
          $(this).prop('checked', function () {
            return this.getAttribute('checked') == 'checked';
          });
        }
        else if ($(this).attr('type') == 'radio') {
          $(this).prop('checked', function () {
            return this.getAttribute('checked') == 'checked';
          });
        }
        else {
          $(this).val('');
        }
      });
      self.$searchBtn.trigger('click');
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
        self.$searchBtn.trigger('click');
        e.preventDefault();
      });
    });
  };

}(jQuery));
