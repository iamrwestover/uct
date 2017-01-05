(function ($) {

}(jQuery));


jQuery(document).ready(function ($) {

  // Prevent double submission of forms.
  $('form').submit(function(e) {
    if( $(this).hasClass('form-submitted') ){
      e.preventDefault();
      return;
    }
    $(this).addClass('form-submitted');
  });
});

