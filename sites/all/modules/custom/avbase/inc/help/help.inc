<?php

function avbase_help_home() {
  global $user;
  $seen_updates = variable_get('av_seen_updates', array());
  $vnum = AVBASE_SYSTEM_VERSION;
  if (empty($seen_updates[$vnum][$user->uid])) {
    $seen_updates[$vnum][$user->uid] = TRUE;
    variable_set('av_seen_updates', $seen_updates);
  }

  $changelog_html = <<<HTML
  <div class="uk-panel uk-panel-box">
    <div class="uk-badge uk-badge-primary uk-text-uppercase" style="position: absolute;top:-1px;left:-1px;">Notable changes from recent $vnum update:</div>
    <ul class="uk-list">
      <li>- Improved quantity reservation.</li>
      <li>- Added validation of negative quantities when editing Received Item or Stock Transfer - In transactions.</li>
      <li>- Out of stock items are now organized at the very bottom of Delivery item list.</li>
      <li>- Added Sales Return Register report.</li>
      <li>- Statement of Account now shows for reference numbers.</li>
      <li>- Updated Balance Sheet Report.</li>
      <li>- Updated Profit & Loss Report.</li>
      <li>- Received Payment and Bills Payment List Reports now shows Excess payment column data.</li>
      <li>- Invoice and Bills List Reports now shows Credited amount column data.</li>
      <li>- Usability and performance improvements.</li>
      <li><hr class="uk-margin-bottom" /></li>
      <li class="uk-text-muted">- Added Sales Order Register report.</li>
      <li class="uk-text-muted">- Easier input of dates.</li>
      <li class="uk-text-muted">- Fixed bug related to inconsistent product quantity.</li>
      <li class="uk-text-muted">- Added Payment Method column on Bills Payment report.</li>
      <li class="uk-text-muted">- Fixed bug where filter by principal name doesnt work properly on Stock Status report.</li>
      <li class="uk-text-muted">- Swapped credit/debit setting for accounting reports.</li>
      <li class="uk-text-muted">- Improved performance on stock status report export.</li>
      <li class="uk-text-muted">- Various bug fixes and performance improvements.</li>
      <li class="uk-text-muted">- Added "add rows" button when editing stock adjustment reports.</li>
      <li class="uk-text-muted">- Added principal column on stock status report.</li>
      <li class="uk-text-muted">- Account names on Profit and Loss report now links to account history.</li>
      <li class="uk-text-muted">- Added filter by account name on Accounts History report.</li>
      <li class="uk-text-muted">- Fixed Accounts History report to also include non-journal account transactions.</li>
    </ul>
  </div>
  <br />
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
    'data' => '<a href="#backup-create" data-uk-smooth-scroll>Creating backups</a>',
  );
  $help_items[] = array(
    'data' => '<a href="#backup-restore" data-uk-smooth-scroll>Restoring a database backup</a>',
  );
  $help_items[] = array(
    'data' => '<a href="#new-database" data-uk-smooth-scroll>System Reset with a New Database</a>',
  );
  $help_items[] = array(
    'data' => '<a href="#system-requirements" data-uk-smooth-scroll>System Requirements</a>',
  );

  $item_list = array('items' => $help_items, 'attributes' => array('class' => array('uk-nav uk-nav-side')));
  $help_items_nav = theme('item_list', $item_list);

  $help_items_html = theme('avbase_help_txn_status', array('help_title' => 'Transaction statuses'));
  $help_items_html .= theme('avbase_help_ks', array('help_title' => 'Keyboard shortcuts'));
  $help_items_html .= theme('avbase_help_print', array('help_title' => 'Printing with dot matrix printers'));
  $help_items_html .= theme('avbase_help_backup_create', array('help_title' => 'Creating backups'));
  $help_items_html .= theme('avbase_help_backup_restore', array('help_title' => 'Restoring a database backup'));
  $help_items_html .= theme('avbase_help_new_db', array('help_title' => 'System Reset with a New Database'));
  $help_items_html .= theme('avbase_help_sys_req', array('help_title' => 'System Requirements'));

  $output = <<<HTML
<div class="printable">
  <div class="uk-grid">
    <div class="uk-width-1-5">
      $help_items_nav
    </div>
    <div class="uk-width-4-5">
      $changelog_html
      $help_items_html    
    </div>
  </div>
  
</div>
HTML;
  return $output;
}