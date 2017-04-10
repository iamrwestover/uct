<?php
$view_mode = !empty($form['#av_view_mode']);
$element_bottom_margin = $view_mode ? '' : ' uk-margin-small-bottom';

$special_discount_html = drupal_render($form['special_discount']);
$balance_ages_html = drupal_render($form['balance_ages']);
?>
<div class="uk-grid uk-grid-collapse">
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
          <?php endif; ?>
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
  <?php if (isset($form['txn_footer_notes'])): ?>
    <div class="uk-width-2-5 uk-margin-top suk-hidden printable"><?php print drupal_render($form['txn_footer_notes']); ?></div>
  <?php endif; ?>
  <!--<div class="uk-width-1-4 uk-margin-top suk-hidden printable">--><?php //print drupal_render($form['received_by_date']); ?><!--</div>-->
</div>
