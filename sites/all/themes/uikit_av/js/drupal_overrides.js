(function ($) {

  if (Drupal.ajax) {
    /**
     * Prepare the Ajax request before it is sent.
     */
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

    /**
     * Handler for the form redirection completion.
     */
    Drupal.ajax.prototype.success = function (response, status) {
      // Remove the progress element.
      if (this.progress.element) {
        $(this.progress.element).remove();
      }
      if (this.progress.object) {
        this.progress.object.stopMonitoring();
      }
      $(this.element).removeClass('progress-disabled').removeAttr('disabled');
      $(this.element).find('i').remove();

      Drupal.freezeHeight();

      for (var i in response) {
        if (response.hasOwnProperty(i) && response[i]['command'] && this.commands[response[i]['command']]) {
          this.commands[response[i]['command']](this, response[i], status);
        }
      }

      // Reattach behaviors, if they were detached in beforeSerialize(). The
      // attachBehaviors() called on the new content from processing the response
      // commands is not sufficient, because behaviors from the entire form need
      // to be reattached.
      if (this.form) {
        var settings = this.settings || Drupal.settings;
        Drupal.attachBehaviors(this.form, settings);
      }

      Drupal.unfreezeHeight();

      // Remove any response-specific settings so they don't get used on the next
      // call by mistake.
      this.settings = null;
    };
  }


  if (Drupal.jsAC) {
    /**
     * Override autocomplete status animations.
     * @param status
     */
    Drupal.jsAC.prototype.setStatus = function (status) {
      switch (status) {
        case 'begin':
          console.log('autocomplete-begin');
          //$(this.input).addClass('throbbing');
          $(this.input).siblings('i').addClass('uk-icon-spin uk-text-primary');
          $(this.ariaLive).html(Drupal.t('Searching for matches...'));
          break;
        case 'cancel':
        case 'error':
        case 'found':
          $(this.input).siblings('i').removeClass('uk-icon-spin uk-text-primary');
          //$(this.input).removeClass('throbbing');
          break;
      }
    };
  }
}(jQuery));
