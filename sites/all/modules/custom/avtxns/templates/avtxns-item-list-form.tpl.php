<?php
$view_mode = !empty($form['#av_view_mode']);
hide($form['header_total']);
hide($form['footer_total']);
//$form['prod_add_btn']['#attributes']['class'][] = 'av-ajax-trigger';
//$form['transaction_date']['#attributes']['data-uk-datepicker'] = "{format:'MMM. DD, YYYY'}";
//$form['buttons']['submit']['#attributes']['class'][] = 'uk-button-primary';
//$form['buttons']['submit_and_send']['#attributes']['class'][] = 'uk-button-primary';
//$form['buttons']['submit_and_send']['#attributes']['disabled'] = TRUE;
//$form['discount_value']['#attributes']['class'][] = 'uk-text-right';

//$form['message']['#attributes']['rows'] = 2;
//$form['memo']['#attributes']['rows'] = 2;
//$form['client_id']['#attributes']['placeholder'] = 'Enter vendor name or company name';

$email_html = drupal_render($form['email']);
$term_html = drupal_render($form['term_id']);
$due_date_html = drupal_render($form['due_date']);
$address_html = drupal_render($form['client_address_string']);
$agent_html = drupal_render($form['agent_name']);
$return_type_html = drupal_render($form['return_type']);
$reference_id_html = drupal_render($form['reference_id']);
$pmt_method_html = drupal_render($form['pmt_method_id']);
$element_bottom_margin = $view_mode ? '' : ' uk-margin-small-bottom';

$special_discount_html = drupal_render($form['special_discount']);
$balance_ages_html = drupal_render($form['balance_ages']);
?>
<div class="printable">
  <div class="uk-grid uk-grid-small">
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


    <!--<div class="uk-width-1-1">--><?php //print drupal_render($form['address']); ?><!--</div>-->



    <div class="uk-width-1-1 uk-margin-small-top">
      <?php print drupal_render($form['product_rows']); ?>
      <div class="uk-grid uk-grid-collapse uk-margin-small-top">
        <?php if (isset($form['journal_debit_total']) && isset($form['journal_credit_total'])): ?>
          <div class="uk-width-4-10 uk-text-right">
            <div class="av-nestable-cell">
              Totals<span class="uk-margin-small-left">&nbsp;</span>
            </div>
          </div>
          <div class="uk-width-6-10 uk-text-<?php print ($view_mode ? 'right' : 'center'); ?>">
            <div class="uk-grid uk-grid-collapse">
              <div class="av-nestable-cell uk-width-1-10">
                <div class="transaction-debit-total">
                  <?php print $form['journal_debit_total']['#value'] ?>
                </div>
              </div>
              <div class="av-nestable-cell uk-width-1-10">
                <div class="transaction-credit-total">
                  <?php print $form['journal_credit_total']['#value'] ?>
                </div>
              </div>
            </div>
          </div>
        <?php endif; ?>

        <div class="uk-width-7-10">

          <div class="uk-grid">
            <?php if (isset($form['prod_add_btn'])): ?>
              <div class="uk-width-1-1 uk-text-large"><?php print drupal_render($form['prod_add_btn']); ?>&nbsp;</div>
            <?php endif; ?>;
          </div>
        </div>

        <div class="uk-width-3-10 uk-text-right">

          <div class="uk-grid uk-grid-small uk-margin-small-top">
            <?php if (isset($form['debit_total'])): ?>
              <div class="uk-width-<?php print ($view_mode ? '3' : '2'); ?>-6 uk-margin-small-top">Sub total</div>
              <div class="uk-width-3-6 uk-margin-small-top"><?php print drupal_render($form['debit_total']); ?></div>
            <?php elseif (isset($form['grand_total'])): ?>
              <div class="uk-width-<?php print ($view_mode ? '3' : '2'); ?>-6 uk-margin-small-top"><h4>Total</h4></div>
              <div class="uk-width-3-6 uk-margin-small-top"><h3 class="product-form-grand-total"><?php print drupal_render($form['grand_total']); ?></h3></div>
            <?php endif; ?>
            <?php if (isset($form['apply_credits_link'])): ?>
              <div class="uk-width-<?php print ($view_mode ? '3' : '2'); ?>-6 uk-margin-small-top"></div>
              <div class="uk-width-3-6 uk-margin-small-top"><?php print drupal_render($form['apply_credits_link']); ?></div>
            <?php endif; ?>
            <?php if (isset($form['amount_received'])): ?>
              <div class="uk-width-<?php print ($view_mode ? '4' : '3'); ?>-6 uk-margin-top"><h5>Amount received</h5></div>
              <div class="uk-width-2-6 uk-margin-top"><?php print drupal_render($form['amount_received']); ?></div>
              <div class="uk-width-<?php print ($view_mode ? '4' : '3'); ?>-6 uk-margin-small"><h5>Amount to credit</h5></div>
              <div class="uk-width-2-6 uk-margin-small"><h5 id="amount-to-credit"><?php print drupal_render($form['amount_to_credit']); ?></h5></div>
            <?php endif; ?>
          </div>
        </div>
      </div>

    </div>

    <?php if (isset($form['credit_rows'])): ?>
      <div class="uk-width-1-1">
        <?php print drupal_render($form['credit_rows']); ?>
        <div class="uk-grid uk-grid-collapse uk-margin-small-top">
          <div class="uk-width-7-10">
            <!--  -->
          </div>

          <div class="uk-width-3-10 uk-text-right">
            <div class="uk-grid uk-grid-small uk-margin-small-top">
              <?php if (isset($form['grand_total'])): ?>
                <div class="uk-width-<?php print ($view_mode ? '3' : '2'); ?>-6 uk-margin-small-top"><h4>Total</h4></div>
                <div class="uk-width-3-6 uk-margin-small-top"><h3 class="product-form-grand-total"><?php print drupal_render($form['grand_total']); ?></h3></div>
              <?php endif; ?>
            </div>
          </div>
        </div>
      </div>
    <?php endif; ?>

    <div class="uk-width-7-10">

      <div class="uk-grid">
        <?php if (isset($form['memo'])): ?>
          <div class="uk-width-3-10"><?php print drupal_render($form['memo']); ?></div>
        <?php endif; ?>
        <?php if (isset($form['message'])): ?>
          <div class="uk-width-3-10"><?php print drupal_render($form['message']); ?></div>
        <?php endif; ?>
        <?php if (isset($form['address'])): ?>
          <div class="uk-width-3-10"><?php print drupal_render($form['address']); ?></div>
        <?php endif; ?>
      </div>
    </div>

    <div class="uk-width-3-10 uk-text-right">
    <!--  -->
    </div>

    <?php if ($special_discount_html): ?>
      <div class="uk-width-1-1 uk-margin-top"><?php print $special_discount_html; ?></div>
    <?php endif; ?>
    <?php if ($balance_ages_html): ?>
      <div class="uk-width-1-1 uk-margin-top"><?php print $balance_ages_html; ?></div>
    <?php endif; ?>
    <div class="uk-width-1-4 uk-margin-top uk-hidden printable"><?php print drupal_render($form['received_by']); ?></div>
    <div class="uk-width-1-4 uk-margin-top uk-hidden printable"><?php print drupal_render($form['received_by_date']); ?></div>
  </div>
</div>
<?php

//$close_btn_label = empty($form['id']['#value']) ? 'Cancel' : 'Close';
////$form['buttons']['submit']['#attributes']['class'][] = 'uk-button-primary';
//$form['buttons']['cancel']['#markup'] = l($close_btn_label, 'av/expenses', array('attributes' => array('class' => array('uk-button'))));
print drupal_render_children($form);