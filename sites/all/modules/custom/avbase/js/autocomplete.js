(function ($) {

  Drupal.behaviors.avbaseAutocompleteActions = {
    attach: function (context, settings) {
      $('input.avbase-autocomplete-actions', context).once('avbaseAutocompleteActions', function () {
        settings.avbase = settings.avbase || {};
        var $input = $(this);
        var entityGroup = $input.data('avbase-entity-group');
        settings.avbase[entityGroup] = settings.avbase[entityGroup] || {};
        new Drupal.avbaseAutocompleteActions($input, settings.avbase[entityGroup]);
      });
    }
  };

  Drupal.avbaseAutocompleteActions = function ($input, entityGroup) {
    $input.on('autocompleteSelect', function(e, node) {
      var rowID = $(node).find('#av-row-id').text();
      $(this).data('selectedRowID', rowID);
      if (!entityGroup[rowID]) {
        entityGroup[rowID] = $.parseJSON($(node).find('#av-row-json').text());
      }
    });
  };

//
//  window.avbaseAutocompleteActions = {
//    $autocompleteEl: null,
//    rowGroupName: null,
//    moduleName: 'avbase',
//    rowIDSelector: '#av-row-id',
//    rowJSONSelector: '#av-row-json',
//
//    init: function() {
//      var superParent = this;
//      superParent.rowGroupName = superParent.rowGroupName || superParent.$autocompleteEl.attr('name');
//      Drupal.settings[superParent.moduleName] = Drupal.settings[superParent.moduleName] || {};
//      Drupal.settings[superParent.moduleName][superParent.rowGroupName] = Drupal.settings[superParent.moduleName][superParent.rowGroupName] || {};
//      superParent.bindEvents();
//    },
//
//    bindEvents: function() {
//      var superParent = this;
//      var $autocompleteEl = superParent.$autocompleteEl;
//      var moduleName = superParent.moduleName;
//      var rowGroupName = superParent.rowGroupName;
//      $autocompleteEl.once('avBaseAutocompleteActions', function() {
//        console.log('rrxx');
//        $(this).on('autocompleteSelect', function(e, node) {
//          // Get product details if not yet set.
//          var rowID = $(node).find(superParent.rowIDSelector).text();
//          $(this).data('selectedRowID', rowID);
//console.log('sssssssssssssssssssss');
//          if (!Drupal.settings[moduleName][rowGroupName][rowID]) {
//            Drupal.settings[moduleName][rowGroupName][rowID] = $.parseJSON($(node).find(superParent.rowJSONSelector).text());
//          }
//        });
//      });
//    }
//  }
}(jQuery));