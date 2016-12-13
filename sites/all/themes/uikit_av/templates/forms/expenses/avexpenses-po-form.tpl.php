<?php
hide($form['header_total']);
hide($form['footer_total']);
$form['prod_add_btn']['#attributes']['class'][] = 'av-ajax-button';
//$form['po_date']['#attributes']['data-uk-datepicker'] = "{format:'MMM. DD, YYYY'}";
//$form['buttons']['submit']['#attributes']['class'][] = 'uk-button-primary';
$form['buttons']['submit_and_send']['#attributes']['class'][] = 'uk-button-primary';
$form['buttons']['submit_and_send']['#attributes']['disabled'] = TRUE;

$form['message']['#attributes']['rows'] = 2;
//$form['vendor_id']['#attributes']['placeholder'] = 'Enter vendor name or company name';
?>
<div class="uk-grid">
  <div class="uk-width-2-4">

    <div class="uk-grid uk-grid-small">
      <div class="uk-width-1-1 uk-margin-bottom"><?php print drupal_render($form['po_date']); ?></div>

      <div class="uk-width-1-2"><?php print drupal_render($form['vendor_id']); ?></div>
      <div class="uk-width-1-2"><?php print drupal_render($form['email']); ?></div>
    </div>

  </div>

  <div class="uk-width-2-4 uk-text-right">
    <div class="uk-text-bold uk-text-uppercase">Grand Total</div>
    <div><h1 class="product-form-grand-total" style="font-size: 50px;">0.00</h1></div>
  </div>


  <!--<div class="uk-width-1-1">--><?php //print drupal_render($form['address']); ?><!--</div>-->



  <div class="uk-width-1-1 uk-margin-large">
    <?php print drupal_render($form['product_rows']); ?>

    <div class="uk-grid uk-grid-collapse uk-margin-small-top">
      <div class="uk-width-7-10">
        <?php print drupal_render($form['prod_add_btn']); ?>
      </div>

      <div class="uk-width-3-10 uk-text-right uk-text-bold">

        <div class="uk-grid uk-grid-collapse uk-margin-small-top">
          <div class="uk-width-2-6"></div>
          <div class="uk-width-1-6"><h3>Total:</h3></div>
          <div class="uk-width-2-6"><h3 class="product-form-grand-total">0.00</h3></div>
          <div class="uk-width-1-6"></div>
        </div>

      </div>

      <!--<div class="uk-width-1-10 uk-text-right uk-text-bold">-->
      <!--  <h3 class="product-form-grand-total">0.00</h3>-->
      <!--</div>-->
    </div>

  </div>


  <div class="uk-width-3-5"><?php print drupal_render($form['message']); ?></div>
  <div class="uk-width-2-5 uk-text-right"><?php print drupal_render($form['address']); ?></div>
</div>
<?php

$close_btn_label = empty($form['id']['#value']) ? 'Cancel' : 'Close';
//$form['buttons']['submit']['#attributes']['class'][] = 'uk-button-primary';
$form['buttons']['cancel']['#markup'] = l($close_btn_label, 'av/expenses', array('attributes' => array('class' => array('uk-button'))));
print drupal_render_children($form);
