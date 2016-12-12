<?php
hide($form['header_total']);
$form['prod_add_btn']['#attributes']['class'][] = 'av-ajax-button';
//$form['po_date']['#attributes']['data-uk-datepicker'] = "{format:'MMM. DD, YYYY'}";
?>
<div class="uk-grid uk-grid-small">
  <div class="uk-width-1-4"><?php print drupal_render($form['vendor_id']); ?></div>
  <div class="uk-width-1-4"><?php print drupal_render($form['email']); ?></div>

  <div class="uk-width-2-4 uk-text-right">
    <div class="uk-text-bold uk-text-uppercase">Total</div>
    <div><h1 style="font-size: 50px;">0.00</h1></div>
  </div>

  <div class="uk-width-2-4"><?php print drupal_render($form['address']); ?></div>
  <div class="uk-width-2-4 uk-text-right"><?php print drupal_render($form['po_date']); ?></div>


  <div class="uk-width-1-1 uk-margin-large">
    <?php print drupal_render($form['product_rows']); ?>
    <div class="uk-margin-small-top"><?php print drupal_render($form['prod_add_btn']); ?></div>
  </div>


  <div class="uk-width-3-5"><?php print drupal_render($form['message']); ?></div>

  <div class="uk-width-1-1 uk-margin-top"><?php print drupal_render($form['buttons']); ?></div>
</div>
<?php
print drupal_render_children($form);
