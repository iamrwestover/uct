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


//$table_rows = array(
//  array(
//    'data' => array('x' => 'z'),
//  ),
//);
//$table = array(
//  '#theme' => 'table',
//  '#header' => array('s' => 'd'),
//  '#rows' => $table_rows,
//  //'#empty' => '<div class="uk-text-muted">' . t('-') . '</div>',
//  '#attributes' => array(
//    //'id' => 'transaction-list-table',
//    'class' => array('uk-table-condensed'),
//  ),
//);
//
//print drupal_render($table);
?>
<div class="printable">
  <div class="uk-grid uk-grid-small">
    <div class="uk-width-7-10 av-ph2">

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

    <div class="uk-width-3-10 uk-form-row-compact av-ph2">
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



    <div class="uk-width-1-1" style="margin-top:12px;">
      <?php print drupal_render($form['product_rows']); ?>
      <div class="uk-grid uk-grid-collapse">
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

        <?php if (empty($view_mode)): ?>
          <div class="uk-width-7-10">
            <?php if (isset($form['prod_add_btn'])): ?>
              <div class="uk-width-1-1 uk-text-large"><?php print drupal_render($form['prod_add_btn']); ?>&nbsp;</div>
            <?php endif; ?>
          </div>
          <div class="uk-width-3-10 uk-text-right">
            <div class="uk-grid uk-grid-small">
              <?php if (isset($form['grand_total'])): ?>
                <div class="uk-width-<?php print ($view_mode ? '3' : '2'); ?>-6 uk-margin-small-top"><h3>Total</h3></div>
                <div class="uk-width-3-6 uk-margin-small-top"><h3 class="product-form-grand-total"><?php print drupal_render($form['grand_total']); ?></h3></div>
              <?php endif; ?>
              <?php if (isset($form['amount_received'])): ?>
                <div class="uk-width-<?php print ($view_mode ? '4' : '3'); ?>-6 uk-margin-top"><h5>Amount received</h5></div>
                <div class="uk-width-2-6 uk-margin-top"><?php print drupal_render($form['amount_received']); ?></div>
                <div class="uk-width-<?php print ($view_mode ? '4' : '3'); ?>-6 uk-margin-small"><h5>Amount to credit</h5></div>
                <div class="uk-width-2-6 uk-margin-small"><h5 id="amount-to-credit"><?php print drupal_render($form['amount_to_credit']); ?></h5></div>
              <?php endif; ?>
            </div>
          </div>
        <?php endif; ?>

        <?php if ($view_mode): ?>
          <?php if (isset($form['debit_total'])): ?>
            <div class="uk-width-8-10 uk-margin-small-top uk-text-right">Sub total</div>
            <div class="uk-width-2-10 uk-margin-small-top uk-text-right"><?php print drupal_render($form['debit_total']); ?></div>
          <?php elseif (isset($form['grand_total'])): ?>
            <div class="uk-width-8-10 uk-margin-small-top uk-text-right not-printable"><h3>Total</h3></div>
            <div class="uk-width-2-10 uk-margin-small-top uk-text-right not-printable" style="border-bottom: solid 1px #ddd;"><h3><?php print drupal_render($form['grand_total']); ?></h3></div>
          <?php endif; ?>
          <?php if (isset($form['apply_credits_link'])): ?>
            <div class="uk-width-8-10 uk-margin-small-top uk-text-right"></div>
            <div class="uk-width-2-10 uk-margin-small-top uk-text-right not-printable"><?php print drupal_render($form['apply_credits_link']); ?></div>
          <?php endif; ?>
          <?php if (isset($form['amount_received'])): ?>
            <div class="uk-width-8-10 uk-margin-top uk-text-right not-printable"><h5>Amount received</h5></div>
            <div class="uk-width-2-10 uk-margin-top uk-text-right not-printable"><h5><?php print drupal_render($form['amount_received']); ?></h5></div>
            <div class="uk-width-8-10 uk-text-right not-printable">Amount to credit</div>
            <div class="uk-width-2-10 uk-text-right not-printable" style="border-bottom: solid 1px #ddd;"><?php print drupal_render($form['amount_to_credit']); ?></div>
          <?php endif; ?>
        <?php endif; ?>

        <!--<div class="uk-width-3-10 uk-text-right">-->
        <!--  <div class="uk-grid uk-grid-small">-->
        <!--    --><?php //if (isset($form['debit_total'])): ?>
        <!--      <div class="uk-width---><?php //print ($view_mode ? '3' : '2'); ?><!---6 uk-margin-small-top">Sub total</div>-->
        <!--      <div class="uk-width-3-6 uk-margin-small-top">--><?php //print drupal_render($form['debit_total']); ?><!--</div>-->
        <!--    --><?php //elseif (isset($form['grand_total'])): ?>
        <!--      <div class="uk-width---><?php //print ($view_mode ? '3' : '2'); ?><!---6 uk-margin-small-top"><h3>Total</h3></div>-->
        <!--      <div class="uk-width-3-6 uk-margin-small-top"><h3 class="product-form-grand-total">--><?php //print drupal_render($form['grand_total']); ?><!--</h3></div>-->
        <!--    --><?php //endif; ?>
        <!--    --><?php //if (isset($form['apply_credits_link'])): ?>
        <!--      <div class="uk-width---><?php //print ($view_mode ? '3' : '2'); ?><!---6 uk-margin-small-top"></div>-->
        <!--      <div class="uk-width-3-6 uk-margin-small-top not-printable">--><?php //print drupal_render($form['apply_credits_link']); ?><!--</div>-->
        <!--    --><?php //endif; ?>
        <!--    --><?php //if (isset($form['amount_received'])): ?>
        <!--      <div class="uk-width---><?php //print ($view_mode ? '4' : '3'); ?><!---6 uk-margin-top"><h5>Amount received</h5></div>-->
        <!--      <div class="uk-width-2-6 uk-margin-top">--><?php //print drupal_render($form['amount_received']); ?><!--</div>-->
        <!--      <div class="uk-width---><?php //print ($view_mode ? '4' : '3'); ?><!---6 uk-margin-small"><h5>Amount to credit</h5></div>-->
        <!--      <div class="uk-width-2-6 uk-margin-small"><h5 id="amount-to-credit">--><?php //print drupal_render($form['amount_to_credit']); ?><!--</h5></div>-->
        <!--    --><?php //endif; ?>
        <!--  </div>-->
        <!--</div>-->
      </div>

    </div>

    <?php if (isset($form['credit_rows'])): ?>
      <div class="uk-width-1-1">
        <?php print drupal_render($form['credit_rows']); ?>
      </div>
      <?php if (isset($form['grand_total'])): ?>
        <div class="uk-width-8-10 uk-text-right not-printable"><h3>Total</h3></div>
        <div class="uk-width-2-10 uk-text-right not-printable" style="border-bottom: solid 1px #ddd;"><h3 class="product-form-grand-total"><?php print drupal_render($form['grand_total']); ?></h3></div>
      <?php endif; ?>
    <?php endif; ?>
    <?php if ($view_mode && isset($form['total_paid_text'])): ?>
      <div class="uk-width-8-10 uk-text-right not-printable">Paid</div>
      <div class="uk-width-2-10 uk-text-right not-printable" style="border-bottom: solid 1px #ddd;"><?php print drupal_render($form['total_paid_text']); ?></div>
      <div class="uk-width-8-10 uk-text-right not-printable">Balance</div>
      <div class="uk-width-2-10 uk-text-right not-printable" style="border-bottom: solid 1px #ddd;"><em><?php print drupal_render($form['rem_bal_text']); ?></em></div>
    <?php endif; ?>
    <?php if ($view_mode && isset($form['available_credit_text'])): ?>
      <div class="uk-width-8-10 uk-text-right not-printable">Credits used</div>
      <div class="uk-width-2-10 uk-text-right not-printable" style="border-bottom: solid 1px #ddd;"><?php print drupal_render($form['credits_used_text']); ?></div>
      <div class="uk-width-8-10 uk-text-right not-printable">Remaining credits</div>
      <div class="uk-width-2-10 uk-text-right not-printable" style="border-bottom: solid 1px #ddd;"><em><?php print drupal_render($form['available_credit_text']); ?></em></div>
    <?php endif; ?>

    <div class="uk-width-7-10 uk-margin-small-top">
      <div class="uk-grid">
        <?php if (isset($form['memo'])): ?>
          <div class="uk-width-<?php print (!isset($form['message']) ? ($view_mode ? '10' : '4') : 4); ?>-10"><?php print drupal_render($form['memo']); ?></div>
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

    <!--<div class="uk-width-1-4 uk-margin-top uk-hidden printable">--><?php //print drupal_render($form['received_by']); ?><!--</div>-->
    <!--<div class="uk-width-1-4 uk-margin-top uk-hidden printable">--><?php //print drupal_render($form['received_by_date']); ?><!--</div>-->
  </div>

  <?php //if (isset($form['txn_footer_notes'])): ?>
  <!--  --><?php //print drupal_render($form['txn_footer_notes']); ?>
  <?php //endif; ?>

  <?php if ($view_mode): ?>
    <div class="uk-grid uk-grid-collapse no-page-break-inside uk-hidden printable">
      <div class="uk-width-1-1">
        <div class="uk-grid">
          <div class="uk-width-7-10">
            <div class="uk-grid uk-grid-small">
              <?php
                $payable_to = strtoupper(variable_get('av_checks_payable_to', 'Ultimate Care Trading'));
                $hide_payable_to = $payable_to == 'HIDE';
              ?>
              <?php if (arg(1) == AVTXNS_TXN_TYPE_DELIVERY): ?>
                <div class="uk-width-2-3 av-ph2">Received the above items in good order and condition</div>
                <div class="uk-width-1-3 uk-text-center"><?php print ($hide_payable_to ? '' : 'All checks payable to'); ?></div>
                <div class="uk-width-2-3">&nbsp;</div>
                <div class="uk-width-1-3 uk-text-center uk-text-bold"><?php print ($hide_payable_to ? '' : $payable_to); ?></div>
              <?php else: ?>
                <div class="uk-width-1-1">&nbsp;</div>
              <?php endif; ?>
              <div class="uk-width-1-3"><hr style="border-color: #000;" /></div>
              <div class="uk-width-1-3"><hr style="border-color: #000;" /></div>
              <div class="uk-width-1-3"></div>
              <div class="uk-width-1-3 uk-text-center av-ph2">Signature Over Printed Name</div>
              <div class="uk-width-1-3 uk-text-center av-ph2">Date</div>
              <div class="uk-width-1-3"></div>
            </div>
          </div>
          <div class="uk-width-3-10">
            <div class="uk-grid uk-grid-collapse">
              <?php if (isset($form['grand_total'])): ?>
                <div class="uk-width-1-3 uk-text-right"><h4>Total</h4></div>
                <div class="uk-width-2-3 uk-text-right" style="border-bottom: solid 1px #ddd;"><h4><?php print render($form['grand_total']); ?></h4></div>
              <?php endif; ?>
              <?php if ($view_mode && isset($form['total_paid_text'])): ?>
                <div class="uk-width-1-3 uk-text-right">Paid</div>
                <div class="uk-width-2-3 uk-text-right" style="border-bottom: solid 1px #ddd;"><?php print render($form['total_paid_text']); ?></div>
                <div class="uk-width-1-3 uk-text-right">Balance</div>
                <div class="uk-width-2-3 uk-text-right" style="border-bottom: solid 1px #ddd;"><em><?php print render($form['rem_bal_text']); ?></em></div>
              <?php endif; ?>
              <?php if (isset($form['amount_received'])): ?>
                <div class="uk-width-1-3 uk-margin-top uk-text-right uk-text-bold">Amount received</div>
                <div class="uk-width-2-3 uk-margin-top uk-text-right uk-text-bold"><?php print render($form['amount_received']); ?></div>
                <div class="uk-width-1-3 uk-text-right">Amount to credit</div>
                <div class="uk-width-2-3 uk-text-right" style="border-bottom: solid 1px #ddd;"><?php print render($form['amount_to_credit']); ?></div>
              <?php endif; ?>
              <?php if ($view_mode && isset($form['available_credit_text'])): ?>
                <div class="uk-width-1-3 uk-text-right">Credits used</div>
                <div class="uk-width-2-3 uk-text-right" style="border-bottom: solid 1px #ddd;"><?php print render($form['credits_used_text']); ?></div>
                <div class="uk-width-1-3 uk-text-right">Remaining credits</div>
                <div class="uk-width-2-3 uk-text-right" style="border-bottom: solid 1px #ddd;"><em><?php print render($form['available_credit_text']); ?></em></div>
              <?php endif; ?>
            </div>
          </div>
        </div>
      </div>


      <?php if (arg(1) == AVTXNS_TXN_TYPE_DELIVERY): ?>
        <div class="uk-width-1-1">
          <br />
          <span class="uk-text-bold av-ph2">IMPORTANT:</span> Count goods carefully before signing.<br />Complaints on defective delivery will not be entertained unless the same are noted and informed herein by the hauler's representative.
        </div>
      <?php endif; ?>
    </div>
  <?php endif; ?>
</div>

<?php

//$close_btn_label = empty($form['id']['#value']) ? 'Cancel' : 'Close';
////$form['buttons']['submit']['#attributes']['class'][] = 'uk-button-primary';
//$form['buttons']['cancel']['#markup'] = l($close_btn_label, 'av/expenses', array('attributes' => array('class' => array('uk-button'))));
print drupal_render_children($form);