(function ($) {
  /**
   * Behaviors of entity overview form.
   * @type {{attach: Function}}
   */
  Drupal.behaviors.avproductsProductListForm = {
    attach: function (context, settings) {
      new Drupal.avproductsProductListForm();
    }
  };

  Drupal.avproductsProductListForm = function() {
    var self = this;
    self.$searchBtn = $('#product-list-search-btn');
    $('#avproducts-product-list-filter-form').once('avproductsProductListForm', function() {
      self.$searchEls = $(this).find('.trigger-ajax-search');
      self.$resetBtn = $(this).find('#product-list-reset-btn');

      //
      //var ajax = Drupal.ajax[self.searchBtnID] || {};
      //self.$targetWrapper = $(ajax.wrapper);


      self.searchEvents();
    });

    self.pagerLinkEvents();
  };

  /**
   * Bind events to search feature.
   */
  Drupal.avproductsProductListForm.prototype.searchEvents = function () {
    var self = this;
    var typingTimer;
    var doneTypingInterval = 500;

    self.$searchEls.on('change', function () {
      if ($(this).hasClass('trigger-search-on-keyup')) {
        return;
      }
      $('#product-list-page-num').val('');
      self.$searchBtn.trigger('click');
    });

    self.$searchEls.on('keyup', function () {
      if (!$(this).hasClass('trigger-search-on-keyup')) {
        return;
      }
      clearTimeout(typingTimer);
      typingTimer = setTimeout(function() {
        $('#product-list-page-num').val('');
        self.$searchBtn.trigger('click');
      }, doneTypingInterval);
    });

    self.$searchEls.on('autocompleteSelect', function(e, node) {
      $('#product-list-page-num').val('');
      self.$searchBtn.trigger('click');
    });

    self.$resetBtn.click(function(e) {
      self.$searchEls.each(function() {
        if ($(this).prop('readonly')) {
          $(this).val($(this).attr('value'));
          return;
        }
        if ($(this).attr('type') == 'checkbox') {
          $(this).prop('checked', '');
          if ($(this).attr('checked')) {
            $(this).prop('checked', 'checked');
          }
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
  Drupal.avproductsProductListForm.prototype.pagerLinkEvents = function () {
    var self = this;
    var ajax = Drupal.ajax[self.$searchBtn.attr('id')] || {};
    self.$targetWrapper = $(ajax.wrapper);
    self.$targetWrapper.once('avproductsProductListForm', function() {
      $(this).find('.uk-pagination a').click(function(e) {
        var page = $(this).data('page') || '';
        $('#product-list-page-num').val(page);
        self.$searchBtn.trigger('click');
        e.preventDefault();
      });
    });
  };

}(jQuery));
