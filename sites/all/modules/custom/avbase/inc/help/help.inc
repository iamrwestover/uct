<?php

function avbase_help_home() {
  $output = '';
  $output .= <<<HTML
  <div class="uk-panel uk-panel-box">
    <div class="uk-badge uk-badge-danger uk-text-uppercase" style="position: absolute;top:-1px;left:-1px;">Notable changes from recent v1.170420 update:</div>
    <ul class="uk-list">
      <li>- &uarr; and &darr; now also switches focus on UOM fields.</li>
      <li>- ALT+&darr; now opens UOM dropdown list.</li>
      <li>- &uarr; and &darr; now also switches focus on checkboxes/radios, e.g. when adding pending invoices on a payment transaction.</li>
      <li>- Full rows can now be clicked to mark checkboxes/radios, e.g. when adding pending invoices on a payment transaction.</li>
      <li>- Stock transfer to and from branches is now available.</li>
      <li>- Products can now be returned easily via "Return items" link (F6) on CLOSED Deliveries.</li>
      <li>- Products can now be ordered easily using the Stock Status Report.</li>
      <li>- New columns on Stock Status Report.</li>
      <li>- Fixed bug Agent field loses focus when tabbing from Customer/Vendor field.</li>
      <li>- Various bugs and performance fixes.</li>
    </ul>
  </div>
  <hr />
HTML;
  $_SESSION['seen_recent_updates'] = TRUE;

  $help_items = array();
  $help_items[] = array(
    'data' => '<a href="#help-txn-status" data-uk-smooth-scroll>Transaction statuses</a>',
  );
  $help_items[] = array(
    //'data' => '<a href=""></a>', l('Keyboard shortcuts', '#av/help', array('attributes' => array('data-uk-smooth-scroll' => ''), 'fragment' => 'help-ks')),
    'data' => '<a href="#help-ks" data-uk-smooth-scroll>Keyboard shortcuts</a>',
  );
  $help_items[] = array(
    //'data' => l('Printing with dot matrix printers', '#av/help', array('attributes' => array('data-uk-smooth-scroll' => ''), 'fragment' => 'help-print')),
    'data' => '<a href="#help-print" data-uk-smooth-scroll>Printing with dot matrix printers</a>',
  );

  $item_list = array('items' => $help_items);
  $output .= theme('item_list', $item_list);

  $output .= theme('avbase_help_txn_status', array('help_title' => 'Transaction statuses'));
  $output .= theme('avbase_help_ks', array('help_title' => 'Keyboard shortcuts'));
  $output .= theme('avbase_help_print', array('help_title' => 'Printing with dot matrix printers'));
  return '<div class="printable">' . $output . '</div>';
}