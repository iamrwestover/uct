<?php

function avbase_help_home() {
  global $user;
  $seen_updates = variable_get('av_seen_updates', array());
  $vnum = AVBASE_SYSTEM_VERSION;
  if (empty($seen_updates[$vnum][$user->uid])) {
    $seen_updates[$vnum][$user->uid] = TRUE;
    variable_set('av_seen_updates', $seen_updates);
  }

  $output = '';
  $output .= <<<HTML
  <div class="uk-panel uk-panel-box">
    <div class="uk-badge uk-badge-primary uk-text-uppercase" style="position: absolute;top:-1px;left:-1px;">Notable changes from recent $vnum update:</div>
    <ul class="uk-list">
      <li>- Disabled products can now be included on Stock Status Report.</li>
      <li>- Vendor Balances Report now shows Received Item reference id.</li>
      <li><hr /></li>
      <li class="uk-text-muted">- Added batch approval of pending transactions on Transactions Home page.</li>
      <li class="uk-text-muted">- Added report: Received Payment Details</li>
      <li class="uk-text-muted">- Added report: Bills Payment Details</li>
      <li class="uk-text-muted">- Auto filling of invoice/charge due date is now based on transaction date instead of current date.</li>
      <li class="uk-text-muted">- Reports now use "mm/dd/yyyy" as date format.</li>
      <li class="uk-text-muted">- Added new filter "Payments made until" on related reports (e.g. Customer Balances, Invoice List, etc.).</li>
    </ul>
  </div>
  <hr />
HTML;


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
  $help_items[] = array(
    'data' => '<a href="#system-requirements" data-uk-smooth-scroll>System Requirements</a>',
  );

  $item_list = array('items' => $help_items);
  $output .= theme('item_list', $item_list);

  $output .= theme('avbase_help_txn_status', array('help_title' => 'Transaction statuses'));
  $output .= theme('avbase_help_ks', array('help_title' => 'Keyboard shortcuts'));
  $output .= theme('avbase_help_print', array('help_title' => 'Printing with dot matrix printers'));
  $output .= theme('avbase_help_sys_req', array('help_title' => 'System Requirements'));
  return '<div class="printable">' . $output . '</div>';
}