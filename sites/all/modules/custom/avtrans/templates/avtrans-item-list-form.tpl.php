<?php
hide($form['header_total']);
hide($form['footer_total']);
$form['prod_add_btn']['#attributes']['class'][] = 'av-ajax-trigger';
//$form['transaction_date']['#attributes']['data-uk-datepicker'] = "{format:'MMM. DD, YYYY'}";
//$form['buttons']['submit']['#attributes']['class'][] = 'uk-button-primary';
$form['buttons']['submit_and_send']['#attributes']['class'][] = 'uk-button-primary';
//$form['buttons']['submit_and_send']['#attributes']['disabled'] = TRUE;
$form['discount_value']['#attributes']['class'][] = 'uk-text-right';

$form['message']['#attributes']['rows'] = 2;
//$form['client_id']['#attributes']['placeholder'] = 'Enter vendor name or company name';

$email_html = drupal_render($form['email']);
$term_html = drupal_render($form['term_id']);
?>

<div class="uk-grid">
  <div class="uk-width-2-4">

    <div class="uk-grid uk-grid-small">
      <div class="uk-width-1-1 uk-margin-bottom"><?php print drupal_render($form['item_list_date']); ?></div>

      <div class="uk-width-3-5"><?php print drupal_render($form['client_name']); ?></div>
      <?php if ($email_html): ?>
        <div class="uk-width-2-5"><?php print $email_html; ?></div>
      <?php endif; ?>
      <?php if ($term_html): ?>
        <div class="uk-width-2-5"><?php print $term_html; ?></div>
      <?php endif; ?>
    </div>

  </div>

  <div class="uk-width-2-4 uk-text-right">
    <div class="uk-text-bold uk-text-uppercase">Grand Total</div>
    <div><h1 class="product-form-grand-total" style="font-size: 50px;">0.00</h1></div>
  </div>


  <!--<div class="uk-width-1-1">--><?php //print drupal_render($form['address']); ?><!--</div>-->



  <div class="uk-width-1-1 uk-margin-large-top">
    <?php print drupal_render($form['product_rows']); ?>

    <div class="uk-grid uk-grid-collapse uk-margin-small-top">
      <div class="uk-width-7-10">

        <div class="uk-grid">
          <div class="uk-width-1-1 uk-text-large"><?php print drupal_render($form['prod_add_btn']); ?>&nbsp;</div>
          <div class="uk-width-3-6 uk-margin-top"><?php print drupal_render($form['message']); ?></div>
          <div class="uk-width-2-6 uk-margin-top"><?php print drupal_render($form['address']); ?></div>
          <div class="uk-width-1-6 uk-margin-small-top uk-text-right"><div class="uk-margin-right uk-margin-small-top">Discount:</div></div>
          <!--<div class="uk-width-1-2"></div>-->
        </div>
      </div>

      <div class="uk-width-3-10 uk-text-right">

        <div class="uk-grid uk-grid-small uk-margin-small-top">
          <div class="uk-width-3-6"><h5>Subtotal</h5></div>
          <div class="uk-width-2-6"><h3 class="product-form-sub-total">0.00</h3></div>
          <div class="uk-width-1-6"></div>
        <!--</div>-->
        <!--<div class="uk-grid uk-grid-small uk-margin-small-top">-->
          <div class="uk-width-2-6 uk-margin-small-top"><?php print drupal_render($form['discount_type']); ?></div>
          <div class="uk-width-1-6 uk-margin-small-top"><?php print drupal_render($form['discount_value']); ?></div>
          <div class="uk-width-2-6 uk-margin-small-top"><h3 class="product-form-discount-total">0.00</h3></div>
          <div class="uk-width-1-6 uk-margin-small-top"></div>
        <!--</div>-->
        <!--<div class="uk-grid uk-grid-collapse uk-margin-small-top">-->
          <div class="uk-width-3-6 uk-margin-small-top"><h3>Total</h3></div>
          <div class="uk-width-2-6 uk-margin-small-top"><h3 class="product-form-grand-total">0.00</h3></div>
          <div class="uk-width-1-6 uk-margin-small-top"></div>
        </div>
      </div>

      <!--<div class="uk-width-1-10 uk-text-right uk-text-bold">-->
      <!--  <h3 class="product-form-grand-total">0.00</h3>-->
      <!--</div>-->
    </div>

  </div>


  <div class="uk-width-3-5"><?php print drupal_render($form['message']); ?></div>
  <div class="uk-width-2-5"><div style="padding-right: 24px;"><?php print drupal_render($form['address']); ?></div></div>
</div>
<?php

$close_btn_label = empty($form['id']['#value']) ? 'Cancel' : 'Close';
//$form['buttons']['submit']['#attributes']['class'][] = 'uk-button-primary';
$form['buttons']['cancel']['#markup'] = l($close_btn_label, 'av/expenses', array('attributes' => array('class' => array('uk-button'))));
print drupal_render_children($form);
