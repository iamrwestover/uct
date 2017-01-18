(function ($) {
  /**
   * Behaviors of entity overview form.
   * @type {{attach: Function}}
   */
  Drupal.behaviors.avbaseEntityOverviewForm = {
    attach: function (context, settings) {
      new Drupal.avbaseEntityOverviewForm();
    }
  };

  Drupal.avbaseEntityOverviewForm = function() {
    var self = this;
    $('#entity-search-form').once('avbaseEntityOverviewForm', function() {
      self.$searchEl = $(this).find('#entity-search-text');
      self.$searchBtn = $(this).find('#entity-search-btn');
      self.searchEvents();
    });

    this.tableEvents();
  };

  /**
   * Bind events to search feature.
   */
  Drupal.avbaseEntityOverviewForm.prototype.searchEvents = function () {
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

  /**
   * Bind events to table.
   */
  Drupal.avbaseEntityOverviewForm.prototype.tableEvents = function () {
    var self = this;
    $('#entity-list-table').once('avbaseEntityOverviewForm', function() {
      $(this).find('tr').click(function() {
        $('#clicked-entity-id').val($(this).data('entity-id'));
        $('#load-entity-btn').trigger('click');
      });
    });
  };
}(jQuery));
