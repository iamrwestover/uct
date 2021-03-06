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
              $(':focus').trigger('blur');
              $element.trigger($element.data('ks-trigger') || 'click');
            }
            e.preventDefault();
          }
        });
      });

      // Shortcut to first transaction item text.
      var $transactionDiv =  $('.av-nestable-form');
      $transactionDiv.once('avKS', function () {
        var $inputEls = $transactionDiv.find('input.form-text:not([readonly]):enabled');
        $(document).bind('keydown', 'alt+1', function (e) {
          if ($inputEls.eq(0).is(':visible')) {
            $inputEls.eq(0).focus().select();
            e.preventDefault();
          }
        });
        // Shortcut to first transaction item text.
        $(document).bind('keydown', 'alt+2', function (e) {
          var lt = $inputEls.length - 1;
          if ($inputEls.eq(lt).is(':visible')) {
            $inputEls.eq(lt).focus().select();
            e.preventDefault();
          }
        });
      });

    }
  };

  // Shortcut to home page.
  $(document).bind('keydown', 'alt+q', function (e) {
    window.location.href = '/';
    e.preventDefault();
  });
  // Shortcut to home page in a new tab.
  $(document).bind('keydown', 'alt+`', function (e) {
    window.open('/', '_blank');
    e.preventDefault();
  });
  // Shortcut to reports page.
  $(document).bind('keydown', 'alt+r', function (e) {
    window.location.href = '/av/reports';
    e.preventDefault();
  });

  // Shortcut to first input text or textarea.
  $(document).bind('keydown', 'alt+home', function (e) {
    var selector = '#region-content-wrapper';
    //if ($('.uk-modal-dialog').is(':visible')) {
    //  selector = '.uk-modal-dialog';
    //  console.log($(selector + " input:radio"));
    //  $(selector + " input:radio, " + selector + " input:checkbox").eq[0].focus();
    //}
    $(selector + " input.form-text:not([data-uk-datepicker]), " + selector + " textarea").eq(0).focus();
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
