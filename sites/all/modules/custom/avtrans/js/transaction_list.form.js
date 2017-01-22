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
    $('#avtrans-transaction-list-form').once('avtransTransactionListForm', function() {
      self.$searchEl = $(this).find('.ajax-search-filter');
      self.$searchBtn = $(this).find('#search-btn');
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
    var doneTypingInterval = 400;

    self.$searchEl.on('keyup', function () {
      clearTimeout(typingTimer);
      typingTimer = setTimeout(function() {
        self.$searchBtn.trigger('click');
      }, doneTypingInterval);
    });
  };

}(jQuery));
