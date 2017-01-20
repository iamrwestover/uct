(function ($) {
  /**
   * Behaviors of nestable product form.
   * @type {{attach: Function}}
   */
  Drupal.behaviors.avimport = {
    attach: function (context, settings) {
      $('#edit-import-type', context).once('avimportImportType', function() {
        var $importTypeEl = $(this).find('input[type="radio"]');
        $('#' + $importTypeEl.val() + '-valid-headers').removeClass('uk-hidden');
        $importTypeEl.click(function() {
          $('.import-valid-headers', context).addClass('uk-hidden');
          $('#' + $(this).val() + '-valid-headers').removeClass('uk-hidden');
        });
      });
    }
  };
}(jQuery));