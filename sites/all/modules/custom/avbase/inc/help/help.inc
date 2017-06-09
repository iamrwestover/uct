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
      <li>- Added report: Delivery Receipt Register</li>
      <li>- Added customer channel column and filter on STT Report.</li>
      <li>- Critical fix on export data involving rows that are more than the set batch limit.</li>
      <li>- Experimental change on backup module.</li>
      <li>- Bug fixes and performance improvements.</li>
      <li><hr /></li>
      <li class="uk-text-muted">- Stock Adjustment transactions can now be edited.</li>
      <li class="uk-text-muted">- Stock Adjustment transaction date can now be changed.</li>
      <li class="uk-text-muted">- Fixed report preview still showing preview on first page load.</li>
      <li class="uk-text-muted">- Database optimizations.</li>
      <li class="uk-text-muted">- Added "Notes" on client edit.</li>
      <li class="uk-text-muted">- Bug fixes and performance improvements..</li>
      <li class="uk-text-muted">- Reports: Export Data</li>
      <li class="uk-text-muted">- RPDs now excluded from STT report.</li>
      <li class="uk-text-muted">- Added Customer Balances report (combination of Invoice and Customer Charges list).</li>
      <li class="uk-text-muted">- Added Vendor Balances report (combination of Bills and Other Bills list).</li>
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

  $item_list = array('items' => $help_items);
  $output .= theme('item_list', $item_list);

  $output .= theme('avbase_help_txn_status', array('help_title' => 'Transaction statuses'));
  $output .= theme('avbase_help_ks', array('help_title' => 'Keyboard shortcuts'));
  $output .= theme('avbase_help_print', array('help_title' => 'Printing with dot matrix printers'));
  return '<div class="printable">' . $output . '</div>';
}