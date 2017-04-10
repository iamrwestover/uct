<?php
$view_mode = !empty($form['#av_view_mode']);

$email_html = drupal_render($form['email']);
$term_html = drupal_render($form['term_id']);
$due_date_html = drupal_render($form['due_date']);
$address_html = drupal_render($form['client_address_string']);
$agent_html = drupal_render($form['agent_name']);
$return_type_html = drupal_render($form['return_type']);
$reference_id_html = drupal_render($form['reference_id']);
$pmt_method_html = drupal_render($form['pmt_method_id']);

$element_bottom_margin = $view_mode ? '' : ' uk-margin-small-bottom';
?>
<div class="uk-grid uk-grid-collapse">
  <div class="uk-width-7-10">

    <div class="uk-grid uk-grid-small">
      <div class="uk-width-1-2">
        <div class="uk-grid uk-grid-small">
          <div class="uk-width-1-1<?php print ($view_mode ? ' uk-text-bold' : ''); ?><?php print $element_bottom_margin; ?>"><?php print drupal_render($form['client_name']); ?></div>
          <?php if ($address_html): ?>
            <div class="uk-width-1-1<?php print $element_bottom_margin; ?>"><?php print $address_html; ?></div>
          <?php endif; ?>
          <?php if ($email_html): ?>
            <div class="uk-width-1-2<?php print $element_bottom_margin; ?>"><?php print $email_html; ?></div>
          <?php endif; ?>
        </div>
      </div>
      <div class="uk-width-1-2">
        <div class="uk-grid uk-grid-small">
          <?php if ($agent_html): ?>
            <div class="uk-width-1-1<?php print $element_bottom_margin; ?>"><?php print $agent_html; ?></div>
          <?php endif; ?>
          <?php if ($return_type_html): ?>
            <div class="uk-width-1-2<?php print $element_bottom_margin; ?>"><?php print $return_type_html; ?></div>
          <?php endif; ?>
          <?php if ($term_html): ?>
            <div class="uk-width-1-2<?php print $element_bottom_margin; ?>"><?php print $term_html; ?></div>
          <?php endif; ?>
          <?php if ($due_date_html): ?>
            <div class="uk-width-1-2<?php print $element_bottom_margin; ?>"><?php print $due_date_html; ?></div>
          <?php endif; ?>
          <?php if ($pmt_method_html): ?>
            <div class="uk-width-1-2<?php print $element_bottom_margin; ?>"><?php print $pmt_method_html; ?></div>
          <?php endif; ?>
          <?php if ($reference_id_html): ?>
            <div class="uk-width-1-2<?php print $element_bottom_margin; ?>"><?php print $reference_id_html; ?></div>
          <?php endif; ?>
        </div>
      </div>
    </div>

  </div>

  <div class="uk-width-3-10 uk-form-row-compact">
    <div class="<?php print ($view_mode ? 'uk-text-right' : ''); ?>">
      <?php print drupal_render($form['transaction_date']); ?>
      <?php
      $transaction_id_html = drupal_render($form['transaction_id']);
      print ($transaction_id_html ? $transaction_id_html : '');
      ?>
      <div class="uk-hidden printable">
        <?php print (isset($form['encoded_by']) ? drupal_render($form['encoded_by']) : ''); ?>
        <?php print (isset($form['prepared_by']) ? drupal_render($form['prepared_by']) : ''); ?>
        <?php print (isset($form['checked_by']) ? drupal_render($form['checked_by']) : ''); ?>
        <?php print (isset($form['num_box']) ? drupal_render($form['num_box']) : ''); ?>
      </div>
    </div>
    <!--<div class="uk-text-bold uk-text-uppercase">Grand Total</div>-->
    <!--<div><h1 class="product-form-grand-total" style="font-size: 50px; margin-bottom: 0;">--><?php //print $form['grand_total']['#value'] ?><!--</h1></div>-->
  </div>
</div>