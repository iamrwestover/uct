(function ($) {
  Drupal.behaviors.avKeyboardShortcuts = {
    attach: function (context, settings) {
      $('[data-ks]', context).once('avKS', function () {
        var $element = $(this);
        if (!$element.is('input') && $element.prop('id') != 'prod-add-btn') {
          $element.attr('data-uk-tooltip', "{delay:'500', cls: 'tt-ks'}");
          $element.prop('title', '<i class="uk-icon-keyboard-o"></i> ' + $element.data('ks'));
        }
        else {
          $element.prop('title', $element.data('ks'));
        }

        $(document).bind('keydown', $element.data('ks'), function (e) {
          if ($element.is(':visible')) {
            if ($element.is('input')) {
              $element.focus().select();
            }
            else if ($element.is('a')) {
              $element[0].click();
            }
            else {
              $element.trigger($element.data('ks-trigger') || 'click');
            }
          }
          e.preventDefault();
        });
      });
    }
  };

  // Shortcut to home page.
  $(document).bind('keydown', 'f4', function (e) {
    window.location.href = '/';
    e.preventDefault();
  });

  // Shortcut to first input text or textarea.
  $(document).bind('keydown', 'alt+ctrl+return', function (e) {
    var selector = '#region-content-wrapper';
    if ($('.uk-modal-dialog').is(':visible')) {
      selector = '.uk-modal-dialog';
    }
    $(selector + " input:text:not([data-uk-datepicker]), " + selector + " textarea").eq(0).focus();
    e.preventDefault();
  });
  // Shortcut to first input text or textarea.
  $(document).bind('keydown', 'alt+ctrl+left', function (e) {
    $(".av-nestable-form input:text:not([data-uk-datepicker])").eq(0).focus();
    e.preventDefault();
  });

  var $quickTxnIdEl = $('#av-quick-txn-id');
  $quickTxnIdEl.bind('keydown', 'shift+return', function(e) {
    if (!$(this).val()) {
      return;
    }
    avTriggerEsc();
    window.open('/av/transaction-search/' + $(this).val(), '_blank');
    e.preventDefault();
  });
  $quickTxnIdEl.bind('keydown', 'return', function(e) {
    if (!$(this).val()) {
      return;
    }
    avTriggerEsc();
    window.location.href = '/av/transaction-search/' + $(this).val();
    e.preventDefault();
  });
  function avTriggerEsc() {
    var esc = $.Event('keydown', { keyCode: 27 });
    $('body').trigger(esc);
    console.log('s');
  }
}(jQuery));
