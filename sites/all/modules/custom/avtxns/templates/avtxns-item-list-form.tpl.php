<?php
$view_mode = !empty($form['#av_view_mode']);
hide($form['header_total']);
hide($form['footer_total']);
$form['prod_add_btn']['#attributes']['class'][] = 'av-ajax-trigger';
//$form['transaction_date']['#attributes']['data-uk-datepicker'] = "{format:'MMM. DD, YYYY'}";
//$form['buttons']['submit']['#attributes']['class'][] = 'uk-button-primary';
//$form['buttons']['submit_and_send']['#attributes']['class'][] = 'uk-button-primary';
//$form['buttons']['submit_and_send']['#attributes']['disabled'] = TRUE;
//$form['discount_value']['#attributes']['class'][] = 'uk-text-right';

$form['message']['#attributes']['rows'] = 2;
$form['memo']['#attributes']['rows'] = 2;
//$form['client_id']['#attributes']['placeholder'] = 'Enter vendor name or company name';

$email_html = drupal_render($form['email']);
$term_html = drupal_render($form['term_id']);
?>
<div class="printable">
  <div class="uk-grid">
    <div class="uk-width-5-6">

      <div class="uk-grid uk-grid-small">
        <div class="uk-width-2-6 uk-margin-bottom"><?php print drupal_render($form['client_name']); ?></div>
        <?php if ($email_html): ?>
          <div class="uk-width-1-6"><?php print $email_html; ?></div>
        <?php endif; ?>
        <?php if ($term_html): ?>
          <div class="uk-width-1-6"><?php print $term_html; ?></div>
          <div class="uk-width-1-6"><?php print drupal_render($form['due_date']); ?></div>
        <?php endif; ?>
        <div class="uk-width-2-6"><?php print drupal_render($form['agent_name']); ?></div>
        <div class="uk-width-1-6"><?php print drupal_render($form['reference_id']); ?></div>
        <div class="uk-width-1-6"><?php print drupal_render($form['return_type']); ?></div>
      </div>

    </div>

    <div class="uk-width-1-6 uk-text-right">
      <div class="uk-text-right">
        <?php
          $transaction_id_html = drupal_render($form['transaction_id']);
          print ($transaction_id_html ? $transaction_id_html : '');
        ?>
        <?php print drupal_render($form['transaction_date']); ?>
      </div>
      <!--<div class="uk-text-bold uk-text-uppercase">Grand Total</div>-->
      <!--<div><h1 class="product-form-grand-total" style="font-size: 50px; margin-bottom: 0;">--><?php //print $form['grand_total']['#value'] ?><!--</h1></div>-->
    </div>


    <!--<div class="uk-width-1-1">--><?php //print drupal_render($form['address']); ?><!--</div>-->



    <div class="uk-width-1-1 uk-margin-top">
      <?php print drupal_render($form['product_rows']); ?>

      <div class="uk-grid uk-grid-collapse uk-margin-small-top">
        <div class="uk-width-7-10">

          <div class="uk-grid">
            <div class="uk-width-1-1 uk-text-large"><?php print drupal_render($form['prod_add_btn']); ?>&nbsp;</div>
            <div class="uk-width-3-10 uk-margin-top"><?php print drupal_render($form['memo']); ?></div>
            <div class="uk-width-3-10 uk-margin-top"><?php print drupal_render($form['message']); ?></div>
            <div class="uk-width-3-10 uk-margin-top"><?php print drupal_render($form['address']); ?></div>
          </div>
        </div>

        <div class="uk-width-3-10 uk-text-right">

          <div class="uk-grid uk-grid-small uk-margin-small-top">
              <!--<div class="uk-width-3-6"><h5>Subtotal</h5></div>-->
              <!--<div class="uk-width-2-6"><h3 class="product-form-sub-total">--><?php //print $form['subtotal']['#value'] ?><!--</h3></div>-->
              <!--<div class="uk-width-1-6"></div>-->
          <!--</div>-->
          <!--<div class="uk-grid uk-grid-small uk-margin-small-top">-->

              <!--<div class="uk-width-2-6 uk-margin-small-top">--><?php //print drupal_render($form['discount_type']); ?><!--</div>-->
              <!--<div class="uk-width-1-6 uk-margin-small-top">--><?php //print drupal_render($form['discount_value']); ?><!--</div>-->
              <!--<div class="uk-width-2-6 uk-margin-small-top"><h3 class="product-form-discount-total">- --><?php //print $form['discounted_value']['#value'] ?><!--</h3></div>-->
              <!--<div class="uk-width-1-6 uk-margin-small-top"></div>-->
          <!--</div>-->
          <!--<div class="uk-grid uk-grid-collapse uk-margin-small-top">-->
            <div class="uk-width-<?php print ($view_mode ? '4' : '3'); ?>-6 uk-margin-small-top"><h3>Total</h3></div>
            <div class="uk-width-2-6 uk-margin-small-top"><h3 class="product-form-grand-total"><?php print $form['grand_total']['#value'] ?></h3></div>
            <!--<div class="uk-width-1-6 uk-margin-small-top"></div>-->
          </div>
        </div>

        <!--<div class="uk-width-1-10 uk-text-right uk-text-bold">-->
        <!--  <h3 class="product-form-grand-total">0.00</h3>-->
        <!--</div>-->
      </div>

    </div>
  </div>

  <?php print drupal_render($form['balance_ages']); ?>
</div>
<?php

//$close_btn_label = empty($form['id']['#value']) ? 'Cancel' : 'Close';
////$form['buttons']['submit']['#attributes']['class'][] = 'uk-button-primary';
//$form['buttons']['cancel']['#markup'] = l($close_btn_label, 'av/expenses', array('attributes' => array('class' => array('uk-button'))));
print drupal_render_children($form);