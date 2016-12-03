(function ($) {
  /**
   * Prepare the Ajax request before it is sent.
   */
  if (Drupal.ajax) {
    Drupal.ajax.prototype.beforeSend = function (xmlhttprequest, options) {
      if (this.form) {
        options.extraData = options.extraData || {};
        options.extraData.ajax_iframe_upload = '1';
        var v = $.fieldValue(this.element);
        if (v !== null) {
          options.extraData[this.element.name] = Drupal.checkPlain(v);
        }
      }
      $(this.element).addClass('progress-disabled').attr('disabled', true);
      if (this.progress.type == 'bar') {
        var progressBar = new Drupal.progressBar('ajax-progress-' + this.element.id, eval(this.progress.update_callback), this.progress.method, eval(this.progress.error_callback));
        if (this.progress.message) {
          progressBar.setProgress(-1, this.progress.message);
        }
        if (this.progress.url) {
          progressBar.startMonitoring(this.progress.url, this.progress.interval || 1500);
        }
        this.progress.element = $(progressBar.element).addClass('ajax-progress ajax-progress-bar');
        this.progress.object = progressBar;
        $(this.element).after(this.progress.element);
      }
      else if (this.progress.type == 'throbber') {
        if (!$(this.element).hasClass('av-ajax-button')) {
          this.progress.element = $('<div class="ajax-progress ajax-progress-throbber"><div class="throbber">&nbsp;</div></div>');
          if (this.progress.message) {
            $('.throbber', this.progress.element).after('<div class="message">' + this.progress.message + '</div>');
          }
          $(this.element).after(this.progress.element);
        }
        else {
          var marginClass = ' uk-margin-small-left';
          if ($(this.element).hasClass('av-ajax-button-icon-only')) {
            marginClass = '';
            $(this.element).html('');
          }
          $(this.element).append('<i class="uk-icon-refresh uk-icon-spin' + marginClass + '"></i>');
        }
      }
    };
  }
}(jQuery));
