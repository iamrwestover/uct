(function ($) {


}(jQuery));


jQuery(document).ready(function ($) {
  var termActions = Object.create(window.avPaymentTermActions);
  termActions.preventChildrenHide = true;
  termActions.init();
});


jQuery(document).load(function ($) {
  // Here we will call the function with jQuery as the parameter once the entire
  // page (images or iframes), not just the DOM, is ready.
});
