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
      <li>- Fixed bug where account balance type is not properly reflected on debit/credit values of Trial Balance and Account history report.</li>
      <li>- Sales return register report now has option to see all Sales Return credits applied on specific transactions made on selected dates.</li>
    </ul>
    <!--<hr class="uk-margin-bottom" />-->
    <button class="uk-button uk-button-small" data-uk-toggle="{target:'#more-changelog'}">more...</button>
    <ul id="more-changelog" class="uk-list uk-text-muted uk-hidden">
      <li class="uk-text-bold">v1.180731:</li>
      <li>- Added Trial Balance report.</li>
      <li>- Updated Balance Sheet Report.</li>
      <li>- Updated Profit & Loss Report.</li>
      <li>- Reference ID will now validate if entered value already exists for that transaction type.</li>
      <li>- Added Stock Transfer IN and OUT Register reports.</li>
      <li>- Added Account Code field for chart of accounts.</li>
      <li>- Added Transaction Date column on list of credits.</li>
      <li>&nbsp;</li>
      <li class="uk-text-bold">earlier versions:</li>
      <li>- Added customer street and barangay fields.</li>
      <li>- Start and End date can now be chosen before scanning transactions for BS, PL and TB reports. Transactions on selected dates will now be considered Locked.</li>
      <li>- Heavily reworked Account History report.</li>
      <li>- Date picker popup now has dropdown menu on Month and Year.</li>
      <li>- Improved quantity reservation.</li>
      <li>- Added validation of negative quantities when editing Received Item or Stock Transfer - In transactions.</li>
      <li>- Out of stock items are now organized at the very bottom of Delivery item list.</li>
      <li>- Added Sales Return Register report.</li>
      <li>- Statement of Account now shows for reference numbers.</li>
      <li>- Received Payment and Bills Payment List Reports now shows Excess payment column data.</li>
      <li>- Invoice and Bills List Reports now shows Credited amount column data.</li>
      <li>- Added Sales Order Register report.</li>
      <li>- Easier input of dates.</li>
      <li>- Fixed bug related to inconsistent product quantity.</li>
      <li>- Added Payment Method column on Bills Payment report.</li>
      <li>- Fixed bug where filter by principal name doesn't work properly on Stock Status report.</li>
      <li>- Swapped credit/debit setting for accounting reports.</li>
      <li>- Improved performance on stock status report export.</li>
      <li>- Various bug fixes and performance improvements.</li>
      <li>- Added "add rows" button when editing stock adjustment reports.</li>
      <li>- Added principal column on stock status report.</li>
      <li>- Account names on Profit and Loss report now links to account history.</li>
      <li>- Added filter by account name on Accounts History report.</li>
      <li>- Fixed Accounts History report to also include non-journal account transactions.</li>
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