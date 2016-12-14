(function ($) {
  window.avbaseAutocompleteActions = {
    $autocompleteEl: null,
    rowGroupName: null,
    moduleName: 'avbase',
    rowIDSelector: '#av-row-id',
    rowJSONSelector: '#av-row-json',

    init: function() {
      var superParent = this;
      superParent.rowGroupName = superParent.rowGroupName || superParent.$autocompleteEl.attr('name');
      Drupal.settings[superParent.moduleName] = Drupal.settings[superParent.moduleName] || {};
      Drupal.settings[superParent.moduleName][superParent.rowGroupName] = Drupal.settings[superParent.moduleName][superParent.rowGroupName] || {};

      superParent.bindEvents();
    },

    bindEvents: function() {
      var superParent = this;
      var $autocompleteEl = superParent.$autocompleteEl;
      var moduleName = superParent.moduleName;
      var rowGroupName = superParent.rowGroupName;
      $autocompleteEl.once('avbaseAutocompleteActions', function() {
        $(this).on('autocompleteSelect', function(e, node) {
          // Get product details if not yet set.
          var rowID = $(node).find(superParent.rowIDSelector).text();
          $(this).data('selectedRowID', rowID);

          if (!Drupal.settings[moduleName][rowGroupName][rowID]) {
            Drupal.settings[moduleName][rowGroupName][rowID] = $.parseJSON($(node).find(superParent.rowJSONSelector).text());
          }
        });
      });
    }
  }
}(jQuery));