<?php

/**
 * Build an fs form.
 * @param $form
 * @param $form_state
 * @param $fs_type
 *
 * @return mixed
 */
function avreports_fs_form($form, &$form_state, $fs_type) {
  $dad = avtxns_txn_list_filter_date_auto_options('Y-m');
  $dad_default = empty($dad['last_month_to_date']) ? array() : $dad['last_month_to_date'];
  $filter_form_settings = array(
    'date_auto' => array(
      '#title' => t('Dates'),
      '#default_value' => 'last_month_to_date',
    ),
    'date_from' => array(
      '#title' => t('From'),
      '#default_value' => $dad_default['date_from'],
      '#attributes' => array(
        'data-uk-datepicker' => "{format:'YYYY-MM',
          minDate: '" . format_date(AVBASE_DATE_MIN_TIMESTAMP, 'custom', 'Y-m') . "',
          maxDate: '" . format_date(AVBASE_DATE_MAX_TIMESTAMP, 'custom', 'Y-m') . "',
        }",
        'class' => array('trigger-ajax-search'),
      ),
      '#av_date_format' => 'Y-m',
      '#av_date_inputmask_format' => 'yyyy-mm',
    ),
    'date_to' => array(
      '#title' => t('To'),
      '#default_value' => $dad_default['date_to'],
      '#attributes' => array(
        'data-uk-datepicker' => "{format:'YYYY-MM',
          minDate: '" . format_date(AVBASE_DATE_MIN_TIMESTAMP, 'custom', 'Y-m') . "',
          maxDate: '" . format_date(AVBASE_DATE_MAX_TIMESTAMP, 'custom', 'Y-m') . "',
        }",
        'class' => array('trigger-ajax-search'),
      ),
      '#av_date_format' => 'Y-m',
      '#av_date_inputmask_format' => 'yyyy-mm',
    ),
    'reset_button' => array('#attributes' => array('style' => 'margin-top: 25px;')),
  );
  $form['transaction_list_filter'] = avtxns_txn_list_filter_form($form, $form_state, $filter_form_settings, 'Y-m');

  // Show update notif.
  // variable_set('avreports_last_scan', strtotime('last week'));
  // variable_del('avreports_last_scan');
  $last_update = variable_get('avreports_last_scan', 0);
  $date_diff = time() - $last_update;
  $days_since_last_update = round($date_diff / (60 * 60 * 24));
  $update_button_class = '';
  if ($days_since_last_update > 14) {
    $update_button_class = ' uk-button-danger';
  }

  $button_tooltip = 'Last scan: ' . (empty($last_update) ? 'Never' : format_date($last_update, 'custom', 'F d, Y'));
  $update_button_html = l('<i class="uk-icon-search"></i><br />Scan', 'av/reports/fs-update-options', array(
    'html' => TRUE,
    'attributes' => array('class' => array('uk-button uk-button-mini uk-margin-small-right' . $update_button_class), 'data-uk-tooltip' => "{cls: 'tt-blue'}", 'title' => $button_tooltip),
    'query' => drupal_get_destination(),
  ));
  $update_notif_html = $update_button_html;


  // $update_notif_html .= $update_button_html;
  $form['transaction_list_filter']['#prefix'] = '<div class="uk-float-right uk-text-small uk-text-right">' . $update_notif_html. '</div>';
  // Show warning message if update has never run.
  // if (empty($last_update)) {
  //   drupal_set_message('Transaction history has never been scanned before. ' . $update_button_html, 'error');
  // }

  // Prepare query variables.
  $select = $joins = $where = $args = array();
  $user_filter = empty($form_state['values']['avtxns_tl_filter']) ? array() : $form_state['values']['avtxns_tl_filter'];
  $filter_query = empty($user_filter['filter_query']) ? array() : $user_filter['filter_query'];
  $where = empty($filter_query['where']) ? array() : $filter_query['where'];
  $args = empty($filter_query['args']) ? array() : $filter_query['args'];
  $args[':date_to_stamp'] = strtotime('last day of this month', $args[':date_to_stamp']);

  // Add table columns depending on date difference.
  $date_group = array();
  $date_from_timestamp = strtotime($user_filter['date_from']);
  $date_to_timestamp = $args[':date_to_stamp'];
  $ts = $date_from_timestamp;
  // Allow "base" date group for computation of first of the group's correct difference.
  $period_balance_only = $fs_type == AVREPORTS_FS_PL;
  // $period_balance_only = FALSE;
  $ts = $period_balance_only ? strtotime('yesterday', $date_from_timestamp) : $ts;
  while ($ts <= $date_to_timestamp) {
    if ($period_balance_only && count($date_group) < 1) {
      $tc_label = 'Base';
      $date_group_id = 'base';
      $last_day_timestamp = $ts;
    }
    else {
      $last_day_timestamp = strtotime('last day of this month', $ts);
      $last_day_timestamp = $last_day_timestamp > $date_to_timestamp ? $date_to_timestamp : $last_day_timestamp;
      $date_group_id = $ts;
      $f_month = format_date($ts, 'custom', 'M');
      $l_month = format_date($last_day_timestamp, 'custom', 'M');
      $f_day = format_date($ts, 'custom', 'd');
      $l_day = format_date($last_day_timestamp, 'custom', 'd');
      $f_year = format_date($ts, 'custom', 'y');
      $l_year = format_date($last_day_timestamp, 'custom', 'y');
      if ($f_year == $l_year) {
        if ($f_month == $l_month) {
          if ($f_day == $l_day) {
            $tc_label = "$f_month.$f_day, $f_year";
          }
          else {
            $tc_label = "$f_month.$f_day-$l_day, $f_year";
          }
        } else {
          $tc_label = "$f_month.$f_day - $l_month.$l_day, $l_year";
        }
      }
      else {
        $tc_label = "$f_month.$f_day, $f_year - $l_month.$l_day, $l_year";
      }
    }

    $date_group[$date_group_id] = array(
      'date_from' => $ts,
      'date_to' => $last_day_timestamp,
      'label' => $tc_label,
    );
    $ts = strtotime('tomorrow', $last_day_timestamp);
  }
  // Add last group for Total column if necessary.
  if ($period_balance_only && !empty($date_group)) {
    $date_group['total'] = array(
      'date_from' => strtotime('tomorrow', $last_day_timestamp),
      'date_to' => strtotime('tomorrow', $last_day_timestamp),
      'label' => 'Total',
    );
  }

  // Get all date_to's.
  $date_tos = array();
  foreach ($date_group as $dg_id => $dg) {
    if ($dg_id == 'total') {
      // No need to query for total.
      continue;
    }
    $date_tos[] = $dg['date_to'];
  }

  // Headers.
  $header = array();
  $header['account'] = '';
  foreach ($date_group as $dg_id => $tc) {
    if ($dg_id == 'base') {
      continue;
    }
    $header[$dg_id] = array('data' => $tc['label'], 'class' => array('uk-text-right'));
  }
  // $header['total'] = array('data' => 'Total', 'class' => array('uk-text-right'));

  $account_types = array();
  switch ($fs_type) {
    case AVREPORTS_FS_PL:
      $account_types = array(AVTXN_AC_TYPE_INCOME, AVTXN_AC_TYPE_OTHER_INCOME, AVTXN_AC_TYPE_COGS, AVTXN_AC_TYPE_EXPENSE);
      break;
    case AVREPORTS_FS_BS:
      $account_types = array(AVTXN_AC_TYPE_CURRENT, AVTXN_AC_TYPE_FIXED, AVTXN_AC_TYPE_LIABILITY, AVTXN_AC_TYPE_CAPITAL, AVTXN_AC_TYPE_AR, AVTXN_AC_TYPE_AP);
      break;
  }


  // Retrieve accounts.
  $accounts_where = $accounts_args = array();
  $accounts_where[] = 'avachart.account_type IN (:account_types)';
  $accounts_where[] = 'avachart.status=1';
  $accounts_args[':account_types'] = $account_types;
  $rs = avtxns_account_chart_rs_load($accounts_where, $accounts_args);
  $fs_accounts = $rs->fetchAllAssoc('id');

  $fs_accounts_hierarchy = avbase_rows_build_tree($fs_accounts, 0, FALSE, FALSE);
  $fs_accounts_by_type = array();
  foreach ($fs_accounts_hierarchy as $row) {
    $fs_accounts_by_type[$row['account_type']][$row['id']] = $row;
  }
  // Move Accounts Receivable into Current Assets right after Cash.
  if (isset($fs_accounts_by_type[AVTXN_AC_TYPE_AR]) && isset($fs_accounts_by_type[AVTXN_AC_TYPE_CURRENT])) {
    $old_currents = $fs_accounts_by_type[AVTXN_AC_TYPE_CURRENT];
    $new_currents = &$fs_accounts_by_type[AVTXN_AC_TYPE_CURRENT];
    $new_currents = array();
    $new_currents[AVTXN_AC_SPECIFIC_CASH_ID] = $old_currents[AVTXN_AC_SPECIFIC_CASH_ID];
    $new_currents[AVTXN_AC_SPECIFIC_AR_ID] = $fs_accounts_by_type[AVTXN_AC_TYPE_AR][AVTXN_AC_SPECIFIC_AR_ID];
    unset($old_currents[AVTXN_AC_SPECIFIC_CASH_ID]);
    unset($fs_accounts_by_type[AVTXN_AC_TYPE_AR]);
    $new_currents += $old_currents;
  }

  // Retrieve fs values.
  $fs_rows = array();
  // if (!empty($date_tos)) {
  //   $query = "SELECT * FROM {avtbl_fs} fs";
  //   unset($where['date_from']);
  //   unset($where['date_to']);
  //   unset($args[':date_from_stamp']);
  //   unset($args[':date_to_stamp']);
  //   $where['date_to'] = "fs.before_this_day IN (:date_tos)";
  //   $args[':date_tos'] = $date_tos;
  //   if (!empty($where)) {
  //     $query .= " WHERE " . implode(' AND ', $where);
  //   }
  //   $query .= " ORDER BY fs.before_this_day ASC";
  //   $rs = db_query($query, $args);
  //
  //   foreach ($rs as $row) {
  //     $timestamp = strtotime('yesterday', $row->before_this_day);
  //     $fs_rows[$row->account_id][$timestamp] = $row;
  //   }
  // }

  // unset($where['date_from']);
  // unset($where['date_to']);
  // unset($args[':date_from_stamp']);
  // unset($args[':date_to_stamp']);


  foreach ($date_tos as $date_to) {
    foreach ($fs_accounts as $account_id => $account_row) {
      $query = "SELECT fs.amount FROM {avtbl_fs} fs";
      $this_where['account_id'] = "fs.account_id = :account_id";
      $this_args[':account_id'] = $account_id;
      $this_where['date_to'] = "fs.before_this_day <= :date_to";
      $this_args[':date_to'] = strtotime('tomorrow midnight', $date_to);
      if (!empty($this_where)) {
        $query .= " WHERE " . implode(' AND ', $this_where);
      }
      $query .= " ORDER BY fs.before_this_day DESC LIMIT 1";
      $amount = db_query($query, $this_args)->fetchField();
      $amount = empty($amount) ? 0 : $amount;
      $fs_rows[$account_id][$date_to] = $amount;
    }
  }


  // Create table.
  $table_rows = array();
  $column_totals = array();
  if ($fs_type == AVREPORTS_FS_PL) {
    // Income.
    avreports_account_type_balance_rows2($table_rows, $fs_accounts_by_type, AVTXN_AC_TYPE_INCOME, $fs_rows, $date_group, $column_totals, TRUE, $user_filter);
    // COGS.
    avreports_account_type_balance_rows2($table_rows, $fs_accounts_by_type, AVTXN_AC_TYPE_COGS, $fs_rows, $date_group, $column_totals, TRUE, $user_filter);
    // Gross Profit Row.
    avreports_account_type_difference_row2($table_rows, 'GROSS PROFIT / (LOSS)', $column_totals, AVTXN_AC_TYPE_INCOME, AVTXN_AC_TYPE_COGS, $date_group);
    // Expenses.
    avreports_account_type_balance_rows2($table_rows, $fs_accounts_by_type, AVTXN_AC_TYPE_EXPENSE, $fs_rows, $date_group, $column_totals, TRUE, $user_filter);
    // Operating Income Row.
    $totals_id = AVTXN_AC_TYPE_INCOME . '.' . AVTXN_AC_TYPE_COGS;
    avreports_account_type_difference_row2($table_rows, 'OPERATING INCOME', $column_totals, $totals_id, AVTXN_AC_TYPE_EXPENSE, $date_group);
    // Other Income.
    avreports_account_type_balance_rows2($table_rows, $fs_accounts_by_type, AVTXN_AC_TYPE_OTHER_INCOME, $fs_rows, $date_group, $column_totals, TRUE, $user_filter);
    // Spacer.
    $data = array();
    $data['account'] = array('data' => '', 'colspan' => 1000);
    $table_rows[] = array('data' => $data);
    // Net Income Row.
    $totals_id = AVTXN_AC_TYPE_INCOME . '.' . AVTXN_AC_TYPE_COGS . '.' . AVTXN_AC_TYPE_EXPENSE;
    avreports_account_type_difference_row2($table_rows, 'NET INCOME', $column_totals, $totals_id, AVTXN_AC_TYPE_OTHER_INCOME, $date_group, FALSE);
  }
  elseif ($fs_type == AVREPORTS_FS_BS) {
    // Current Assets.
    avreports_account_type_balance_rows2($table_rows, $fs_accounts_by_type, AVTXN_AC_TYPE_CURRENT, $fs_rows, $date_group, $column_totals, TRUE, $user_filter);
    // Fixed Assets.
    avreports_account_type_balance_rows2($table_rows, $fs_accounts_by_type, AVTXN_AC_TYPE_FIXED, $fs_rows, $date_group, $column_totals, TRUE, $user_filter);
    // Total Assets.
    avreports_account_type_difference_row2($table_rows, 'TOTAL ASSETS', $column_totals, AVTXN_AC_TYPE_CURRENT, AVTXN_AC_TYPE_FIXED, $date_group, FALSE);
    // Liabilities.
    avreports_account_type_balance_rows2($table_rows, $fs_accounts_by_type, AVTXN_AC_TYPE_LIABILITY, $fs_rows, $date_group, $column_totals, TRUE, $user_filter);
    // Capital.]
    avreports_account_type_balance_rows2($table_rows, $fs_accounts_by_type, AVTXN_AC_TYPE_CAPITAL, $fs_rows, $date_group, $column_totals, TRUE, $user_filter);
    // Total Liabilities and Capital.
    avreports_account_type_difference_row2($table_rows, 'TOTAL LIABILITIES AND CAPITAL', $column_totals, AVTXN_AC_TYPE_LIABILITY, AVTXN_AC_TYPE_CAPITAL, $date_group, FALSE);
    // Equity.
    // $total_assets_id = AVTXN_AC_TYPE_CURRENT . '.' . AVTXN_AC_TYPE_FIXED;
    // avreports_account_type_difference_row2($table_rows, 'EQUITY', $column_totals, $total_assets_id, AVTXN_AC_TYPE_LIABILITY, $date_group, TRUE);
  }

  // Format the table.
  $transaction_list_table['table'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $table_rows,
    '#empty' => '<div class="uk-text-muted">' . t('No match found.') . '</div>',
    '#attributes' => array(
      'id' => 'transaction-list-table',
      'class' => array('uk-table-condensed uk-table-hover'),
    ),
  );

  $form['transaction_list_table'] = array(
    '#type' => 'container',
    '#theme' => 'avreports_inventory_ranking',
    '#attributes' => array(
      'id' => 'transaction-list-table-wrapper',
      'class' => array('printable')
    ),
  );

  // avtxns_txn_list_table_add_date_from($transaction_list_table, $user_filter);

  $form['transaction_list_table'] += $transaction_list_table;

  $form['buttons']['#theme'] = 'avbase_crud_button_group';
  $form['buttons']['print'] = array(
    '#id' => 'av-print-btn',
    '#type' => 'button',
    '#value' => t('Print...'),
    '#visible_in_view_mode' => TRUE,
    '#icon_key' => 'print',
  );
  return $form;
}

/**
 * FS Update Options form.
 * @param $form
 * @param $form_state
 *
 * @return array
 */
function avreports_fs_update_form($form, &$form_state) {
  $scan_locked = variable_get('avreports_scan_lock', 0);
  // variable_del('avreports_last_scan');
  // variable_set('avreports_last_scan', strtotime('last week'));
  $form['range'] = array(
    '#type' => 'radios',
    '#title' => 'Scan range',
    '#options' => array(
      'all' => 'Re-scan everything',
      'new' => 'Only transactions from last scan date onwards',
    ),
    '#default_value' => 'new',
    '#attributes' => array(
      'class' => array('uk-button-group'),
    ),
    '#required' => TRUE,
    '#description' => 'To help generate reports quicker and more efficiently, the system has to <strong>scan and remember</strong> transaction history.
    <!--<br />It may take some time to finish updating the system\'s <strong>memory</strong> depending on the number of transactions that need to be scanned.-->
    <br />A re-scan of all transactions is recommended from time to time. It takes more time, but it ensures that your reports are up-to-date.',
  );

  $last_update = variable_get('avreports_last_scan', 0);
  $form['last_update'] = array(
    '#type' => 'item',
    '#title' => 'Last scan',
    '#markup' => empty($last_update) ? 'Never' : format_date($last_update, 'custom', 'F d, Y'),
    '#title_display' => 'inline-before',
  );
  // if (empty($last_update)) {
  //   $form['range']['#options']['all'] = 'Scan everything';
  //   $form['range']['#default_value'] = 'all';
  // }

  if (!empty($scan_locked)) {
    $form['force'] = array(
      '#type' => 'fieldset',
      '#title' => '<i class="uk-icon-lock uk-margin-right uk-text-danger"></i>Scan is locked',
      '#collapsed' => TRUE,
      '#collapsible' => TRUE,
    );
    $form['force']['force_unlock'] = array(
      '#type' => 'checkbox',
      '#title' => 'Force unlock',
      '#description' => 'Only check this if you are sure that no one else is running a scan.',
    );
  }

  $form['buttons']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Begin scan'),
    '#icon_key' => 'search',
    '#attributes' => array('class' => array('uk-button-primary uk-margin-top')),
  );
  return $form;
}

function avreports_fs_update_form_validate($form, &$form_state) {
  $force_unlock = !empty($form_state['values']['force_unlock']);
  if (!$force_unlock) {
    $scan_locked = variable_get('avreports_scan_lock', 0);
    // Check if scan should be locked.
    if (!empty($scan_locked)) {
      form_set_error('', 'Someone has started a scan and has not yet finished. Please try again later.');
    }
  }
}

/**
 * Submit handler for avreports_fs_update_form().
 */
function avreports_fs_update_form_submit($form, &$form_state) {
  $range = empty($form_state['values']['range']) ? NULL : $form_state['values']['range'];
  if (empty($range)) {
    return;
  }

  // Do some cleaning first.
  if ($range == 'all') {
    // Empty table.
    db_query("TRUNCATE TABLE {avtbl_fs}");
    db_query("TRUNCATE TABLE {avtbl_inventory_cache}");
    // Set start date from the very first record in transactions table.
    $date_start = avreports_get_that_transaction_date();
  }
  else {
    $last_update = variable_get('avreports_last_scan', 0);
    if (empty($last_update)) {
      // Have not scanned ever.
      // Set start date from the very first record in transactions table.
      $date_start = avreports_get_that_transaction_date();
    }
    else {
      // Delete all history for transactions from last update onwards.
      db_query("DELETE FROM {avtbl_fs} WHERE before_this_day > $last_update");
      // Set start date same as last update date.
      $date_start = $last_update;
    }
  }

  // If no date start, interrupt and send error message.
  if (empty($date_start)) {
    drupal_set_message('There are no transactions recorded yet.', 'warning');
    return;
  }
  $date_start = strtotime('today midnight', $date_start);
  // Set end date.
  $date_end = avreports_get_that_transaction_date(FALSE);
  $date_end = strtotime('tomorrow midnight', $date_end);
  // $date_end = strtotime('tomorrow midnight');

  // Get all products available.
  $query = "SELECT id, title, initial_qty, initial_value item_cost, initial_qty_date,
    (initial_value * initial_qty) AS asset_value
    FROM {avtbl_products} WHERE id=4087
    ORDER BY id DESC";
  $rs = db_query($query);
  $operations = array();
  foreach ($rs as $account_row) {
    $operations[] = array('avreports_batch_scan_transactions', array('products', $account_row, $date_start, $date_end));
  }

  // Get all accounts available.
  // $query = "SELECT id, title, account_type FROM {avtbl_account_chart} WHERE status = 1 ORDER BY id DESC";
  // $rs = db_query($query);
  // $operations = array();
  // foreach ($rs as $account_row) {
  //   $operations[] = array('avreports_batch_scan_transactions', array('accounts', $account_row, $date_start, $date_end));
  // }
  if (empty($operations)) {
    drupal_set_message('Scan is not required yet.', 'warning');
    return;
  }

  // Prepare and begin batch.
  variable_set('avreports_scan_lock', TRUE);
  $batch = array(
    'operations' => $operations,
    'finished' => 'avreports_batch_scan_finished',
    'title' => 'Scanning transactions',
    // 'init_message' => t('Example Batch is starting.'),
    // 'progress_message' => 'Scanning transaction history',
    'error_message' => t('Scan encountered an error'),
    'file' => AVREPORTS_MODULE_PATH . '/inc/fs2.form.inc',
  );
  batch_set($batch);
}

/**
 * Batch process to scan Accounts transaction history.
 *
 * @param string $scan_type
 * @param $row
 * @param $date_start
 * @param $date_end
 * @param $context
 */
function avreports_batch_scan_transactions($scan_type = '', $row, $date_start, $date_end, &$context) {
  if (!isset($context['sandbox']['progress'])) {
    $date_diff = $date_end - $date_start;
    // $context['sandbox']['max'] = round($date_diff / (60 * 60 * 24));
    $context['sandbox']['max'] = $date_diff;
    $context['results'] = array();
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['date_start'] = $date_start;
  }

  // Pointers.
  $_results = &$context['results'];
  $_progress = &$context['sandbox']['progress'];
  $_max = &$context['sandbox']['max'];
  $_start = &$context['sandbox']['date_start'];

  // Set limit per iteration.
  $limit = 1;
  $remaining = $_max - $_progress;
  if ($limit > $remaining) {
    $limit = $remaining;
  }
  for ($i = 0; $i < $limit; $i++) {
    $_end = strtotime('first day of next month', $_start);
    // $_end = strtotime('tomorrow midnight', $_start);
    if ($scan_type == 'accounts') {
      avtxns_read_account_transactions($row->id, $row->account_type, $_start, $_end);
    }
    elseif ($scan_type == 'products') {
      avtxns_read_product_transactions($row, $_start, $_end);
    }
    // Update progress counter.
    $_progress += $_end - $_start;
    // Update start date for next loop.
    $_start = $_end;
    break;
  }

  // Update progress counter.
  // $_progress += $limit;

  $context['message'] = t('Scanned @progress of @max transaction days for %title.', array('@progress' => round($_progress / (60 * 60 * 24)), '@max' => round($_max / (60 * 60 * 24)), '%title' => $row->title));
  $context['message'] .= '<div class="uk-text-muted uk-text-small uk-margin-top">' . t('Earliest transaction included on scan: ' . format_date($date_start, 'custom', 'M. d, Y')) . '</div>';
  $context['message'] .= '<div class="uk-text-muted uk-text-small">' . t('Latest transaction included on scan: ' . format_date(strtotime('yesterday', $date_end), 'custom', 'M. d, Y')) . '</div>';
  $context['message'] .= '<div class="uk-text-muted uk-text-small"><i class="uk-icon-exclamation-triangle uk-color-orange"></i> Void and pending transactions are excluded from scan.</div>';

  // Compute for progress percentage.
  $finished = $_progress / $_max;
  if ($finished >= 1 && empty($_results['success'])) {
    $finished = $_progress / $_max;
  }
  // Set progress percentage.
  $context['finished'] = $finished;
}

/**
 * Callback when Transaction History scanning has finished.
 * @param $success
 * @param $results
 * @param $operations
 *
 * @throws Exception
 */
function avreports_batch_scan_finished($success, $results, $operations) {
  if ($success) {
    drupal_set_message('Scan has finished.');
    // Remember last scan.
    variable_set('avreports_last_scan', time());
    // Remove lock.
    variable_del('avreports_scan_lock');
  }
  else {
    drupal_set_message(t('Scan was interrupted.'), 'error');
  }
}

/**
 * Get the very first or last transaction date.
 * @param bool $get_first
 *
 * @return false|int
 */
function avreports_get_that_transaction_date($get_first = TRUE) {
  $sort_dir = $get_first ? 'ASC' : 'DESC';
  $ret = &drupal_static(__FUNCTION__, array());
  if (isset($ret[$sort_dir])) {
    return $ret[$sort_dir];
  }

  $query = "SELECT avtxn.transaction_date FROM {avtbl_transactions} avtxn
    WHERE avtxn.status NOT IN (" . AVTXNS_TXN_STATUS_PENDING . ',' . AVTXNS_TXN_STATUS_VOID . ")
    ORDER BY avtxn.transaction_date $sort_dir LIMIT 1";
  $timestamp = db_query($query)->fetchField();
  if (empty($timestamp)) {
    $timestamp = 0;
  }

  $ret[$sort_dir] = $timestamp;
  return $timestamp;
}
