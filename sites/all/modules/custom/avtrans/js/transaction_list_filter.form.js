(function ($) {
  /**
   * Behaviors of entity overview form.
   * @type {{attach: Function}}
   */
  Drupal.behaviors.avtransTransactionListForm = {
    attach: function (context, settings) {
      new Drupal.avtransTransactionListForm();
    }
  };

  Drupal.avtransTransactionListForm = function() {
    var self = this;
    $('#avtrans-transaction-list-filter-form').once('avtransTransactionListForm', function() {
      self.$searchEls = $(this).find('.trigger-ajax-search');
      self.$searchBtn = $(this).find('#transaction-search-btn');
      self.$resetBtn = $(this).find('#transaction-reset-btn');
      self.searchEvents();
    });

    //this.tableEvents();
  };

  /**
   * Bind events to search feature.
   */
  Drupal.avtransTransactionListForm.prototype.searchEvents = function () {
    var self = this;
    var typingTimer;
    var doneTypingInterval = 500;

    self.$searchEls.on('change', function () {
      if ($(this).hasClass('trigger-search-on-keyup')) {
        return;
      }
      self.$searchBtn.trigger('click');
    });

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
          //console.log($(this).attr('value'));
          $(this).val($(this).attr('value'));
          return;
        }
        $(this).val('');
      });
      self.$searchBtn.trigger('click');
      e.preventDefault();
    });
  };

}(jQuery));
