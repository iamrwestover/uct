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
  // variable_set('avtxns_as_of_date_lock', NULL);
  $dad = avtxns_txn_list_filter_date_auto_options(AVBASE_MONTH_ONLY_FORMAT);
  $dad_default = empty($dad['last_month_to_date']) ? array() : $dad['last_month_to_date'];
  $transaction_lock_date = variable_get('avtxns_as_of_date_lock', NULL);
  if (!empty($transaction_lock_date)) {
    $dad_default['date_from'] = format_date(strtotime('first day of this month - 2 months', $transaction_lock_date), 'custom', AVBASE_MONTH_ONLY_FORMAT);
    $dad_default['date_to'] = format_date($transaction_lock_date, 'custom', AVBASE_MONTH_ONLY_FORMAT);
  }
  $filter_form_settings = array(
    'date_auto' => array(
      '#title' => t('Dates'),
      '#default_value' => 'custom',
    ),
    'date_from' => array(
      '#title' => t('From'),
      '#default_value' => $dad_default['date_from'],
      '#attributes' => array(
        'data-uk-datepicker' => "{format:'" . AVBASE_MONTH_ONLY_FORMAT_JS . "',
          minDate: '" . format_date(AVBASE_DATE_MIN_TIMESTAMP, 'custom', AVBASE_MONTH_ONLY_FORMAT) . "',
          maxDate: '" . format_date(AVBASE_DATE_MAX_TIMESTAMP, 'custom', AVBASE_MONTH_ONLY_FORMAT) . "',
        }",
        'class' => array('trigger-ajax-search'),
      ),
      '#av_date_format' => AVBASE_MONTH_ONLY_FORMAT,
      '#av_date_inputmask_format' => AVBASE_MONTH_ONLY_FORMAT_MASK,
    ),
    'date_to' => array(
      '#title' => t('To'),
      '#default_value' => $dad_default['date_to'],
      '#attributes' => array(
        'data-uk-datepicker' => "{format:'" . AVBASE_MONTH_ONLY_FORMAT_JS . "',
          minDate: '" . format_date(AVBASE_DATE_MIN_TIMESTAMP, 'custom', AVBASE_MONTH_ONLY_FORMAT) . "',
          maxDate: '" . format_date(AVBASE_DATE_MAX_TIMESTAMP, 'custom', AVBASE_MONTH_ONLY_FORMAT) . "',
        }",
        'class' => array('trigger-ajax-search'),
      ),
      '#av_date_format' => AVBASE_MONTH_ONLY_FORMAT,
      '#av_date_inputmask_format' => AVBASE_MONTH_ONLY_FORMAT_MASK,
    ),
    'running_balance' => array(
      '#title' => 'Running balance',
      '#default_value' =>  $fs_type == AVREPORTS_FS_BS,
    ),
    'reset_button' => array('#attributes' => array('style' => 'margin-top: 25px;')),
  );
  $form['transaction_list_filter'] = avtxns_txn_list_filter_form($form, $form_state, $filter_form_settings, AVBASE_MONTH_ONLY_FORMAT);

  // Show update notif.
  // variable_set('avreports_last_scan', strtotime('last week'));
  // variable_del('avreports_last_scan');
  $last_update = variable_get('avreports_last_scan', 0);
  // $date_diff = time() - $last_update;
  // $days_since_last_update = round($date_diff / (60 * 60 * 24));
  $update_button_class = '';
  // if ($days_since_last_update > 14) {
  //   $update_button_class = ' uk-button-danger';
  // }

  // $button_tooltip = 'Last scan: ' . (empty($last_update) ? 'Never' : format_date($last_update, 'custom', 'F d, Y'));
  $button_tooltip = '';
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
  $period_balance_only = FALSE;
  // Start with 0 balance for BS, unless selected start date is same less than or equal to fiscal year.
  $start_with_zero_balance = $fs_type == AVREPORTS_FS_BS;
  $start_with_zero_balance = $start_with_zero_balance && $ts > variable_get('av_fiscal_date', 0);
  $start_with_zero_balance = FALSE;
  // $start_with_zero_balance = FALSE;
  $running_balance = empty($user_filter['settings']['running_balance']) ? FALSE : TRUE;

  // $period_balance_only = FALSE;
  $ts = ($period_balance_only || $start_with_zero_balance) ? strtotime('yesterday', $date_from_timestamp) : $ts;
  while ($ts <= $date_to_timestamp) {
    if ($period_balance_only && count($date_group) < 1) {
      $tc_label = 'Base';
      $date_group_id = 'base';
      $last_day_timestamp = $ts;
    }
    elseif ($start_with_zero_balance && count($date_group) < 1) {
      $tc_label = 'Beginning Balance';
      $date_group_id = 'beg_bal';
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
  if (!$running_balance && !empty($date_group)) {
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
    if ($dg_id == 'base' || $dg_id == 'beg_bal') {
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
  $fiscal_date = variable_get('av_fiscal_date', 0);
  $fs_rows = array();
  foreach ($date_tos as $date_to) {
    if ($running_balance && $fiscal_date > $date_to) {
      continue;
    }
    $show_fiscal_beg_bal = $running_balance && $fiscal_date <= $date_to && $fiscal_date >= strtotime('first day of this month', $date_to);
    foreach ($fs_accounts as $account_id => $account_row) {
      // if ($account_id != 1 && $account_id != 2) continue;
      $this_where = $this_args = array();
      if ($show_fiscal_beg_bal) {
        $query = "SELECT SUM(fs.debit - fs.credit) amount FROM {avtbl_fs} fs";
        $this_where['account_id'] = "fs.account_id = :account_id";
        $this_args[':account_id'] = $account_id;
        $this_where['date_to'] = "fs.before_this_day <= :date_to";
        $this_args[':date_to'] = strtotime('tomorrow midnight', $date_to);
        if (!empty($this_where)) {
          $query .= " WHERE " . implode(' AND ', $this_where);
        }
      }
      else {
        $query = "SELECT (fs.debit - fs.credit) amount FROM {avtbl_fs} fs";
        $this_where['account_id'] = "fs.account_id = :account_id";
        $this_args[':account_id'] = $account_id;
        $this_where['date_to'] = "fs.before_this_day = :date_to";
        $this_args[':date_to'] = strtotime('tomorrow midnight', $date_to);
        if (!empty($this_where)) {
          $query .= " WHERE " . implode(' AND ', $this_where);
        }
      }
      $amount = db_query($query, $this_args)->fetchField();
      $amount = empty($amount) ? 0 : $amount;


      $last_amount = $running_balance ? (empty($fs_rows[$account_id]) ? 0 : end($fs_rows[$account_id])) : 0;
      $fs_rows[$account_id][$date_to] = $amount + $last_amount;
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
      'class' => array('uk-table-condensed uk-table-hover uk-text-small'),
    ),
    // '#sticky' => FALSE,
  );
  $transaction_lock_date = variable_get('avtxns_as_of_date_lock', NULL);
  $transaction_list_table['other_info']['#markup'] = empty($transaction_lock_date)
    ? '<span class="uk-text-danger uk-text-bold"><i class="uk-icon-exclamation-triangle uk-margin-small-right"></i>Scan and Lock has never been run before.</span>'
    : t('<span class="uk-text-uppercase uk-text-bold uk-text-large">As of @date</span>', array('@date' => format_date($transaction_lock_date, 'custom', 'F d, Y')));

  $form['transaction_list_table'] = array(
    '#type' => 'container',
    '#theme' => 'avreports_txn_list',
    '#attributes' => array(
      'id' => 'transaction-list-table-wrapper',
      'class' => array('printable av-sticky-small')
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
  // variable_del('avtxns_as_of_date_lock');
  $transaction_lock_date = variable_get('avtxns_as_of_date_lock', NULL);

  $form['last_update'] = array(
    '#type' => 'item',
    '#markup' => empty($transaction_lock_date)
      ? '<span class="uk-text-danger uk-text-bold"><i class="uk-icon-exclamation-triangle uk-margin-small-right"></i>Scan and Lock has never been run before.</span>'
      : t('Transactions on or before end of %date have been locked.', array('%date' => format_date($transaction_lock_date, 'custom', 'F Y'))),
    // '#markup' => empty($transaction_lock_date) ? 'Never' : format_date($transaction_lock_date, 'custom', 'F d, Y'),
    '#title_display' => 'inline-before',
    '#description' => 'To help generate reports quicker and more efficiently, the system has to <strong>scan and lock</strong> transactions.
    <!--<br />It may take some time to finish updating the system\'s <strong>memory</strong> depending on the number of transactions that need to be scanned.-->
    <br />A re-scan is recommended whenever a locked transaction has been modified.',
  );

  $date_from = empty($transaction_lock_date) ? avreports_get_that_transaction_date() : strtotime('tomorrow', $transaction_lock_date);
  $form['date_from'] = array(
    '#title' => t('From'),
    '#type' => 'textfield',
    '#icon_key' => 'calendar',
    '#required' => TRUE,
    // '#default_value' => format_date($date_from, 'custom', AVBASE_MONTH_ONLY_FORMAT),
    '#default_value' => '2013-01',
    '#disabled' => empty($transaction_lock_date),
    '#attributes' => array(
      'data-uk-datepicker' => "{format:'" . AVBASE_MONTH_ONLY_FORMAT_JS . "',
          minDate: '" . format_date(AVBASE_DATE_MIN_TIMESTAMP, 'custom', AVBASE_MONTH_ONLY_FORMAT) . "',
          maxDate: '" . format_date(AVBASE_DATE_MAX_TIMESTAMP, 'custom', AVBASE_MONTH_ONLY_FORMAT) . "',
        }",
      'class' => array('trigger-ajax-search uk-width-1-6'),
      'placeholder' => AVBASE_MONTH_ONLY_FORMAT_JS,
      'autocomplete' => 'off',
    ),
    '#av_date_inputmask' => TRUE,
    '#av_date_format' => AVBASE_MONTH_ONLY_FORMAT,
    '#av_date_inputmask_format' => AVBASE_MONTH_ONLY_FORMAT_MASK,
    '#element_validate' => array('avbase_element_validate_date'),
  );
  $form['date_to'] = $form['date_from'];
  $form['date_to']['#title'] = 'To';
  unset($form['date_to']['#disabled']);
  if (empty($transaction_lock_date)) {
    unset($form['date_to']['#default_value']);
  }
  else {
    // $form['date_to']['#default_value'] = format_date(strtotime('first day of next month', $date_from), 'custom', AVBASE_MONTH_ONLY_FORMAT);
    $form['date_to']['#default_value'] = '2018-07';
  }
  // $form['range'] = array(
  //   '#type' => 'radios',
  //   '#title' => 'Scan range',
  //   '#options' => array(
  //     'all' => 'Re-scan everything',
  //     'new' => 'Only transactions from last scan date onwards',
  //   ),
  //   '#default_value' => 'new',
  //   '#attributes' => array(
  //     'class' => array('uk-button-group'),
  //   ),
  //   '#required' => TRUE,
  //   '#description' => 'To help generate reports quicker and more efficiently, the system has to <strong>scan and remember</strong> transaction history.
  //   <!--<br />It may take some time to finish updating the system\'s <strong>memory</strong> depending on the number of transactions that need to be scanned.-->
  //   <br />A re-scan of all transactions is recommended from time to time. It takes more time, but it ensures that your reports are up-to-date.',
  // );



  // if (empty($last_update)) {
  //   $form['range']['#options']['all'] = 'Scan everything';
  //   $form['range']['#default_value'] = 'all';
  // }
// dpm(avtxns_get_qty_change_txn_types());
// dpm(avtxns_get_qty_change_txn_types('increment'));
// dpm(avtxns_get_qty_change_txn_types('decrement'));
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
    '#value' => t('Begin scan and lock'),
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
      return;
    }
  }

  $selected_date_from = empty($form_state['values']['date_from']) ? 0 : $form_state['values']['date_from'];
  $selected_date_to = empty($form_state['values']['date_to']) ? 0 : $form_state['values']['date_to'];
  if ($selected_date_from >= $selected_date_to) {
    form_set_error('', 'End date must be later than start date.');
  }

  $transaction_lock_date = variable_get('avtxns_as_of_date_lock', NULL);
  if (!empty($transaction_lock_date)) {
    $max_date_from = strtotime('tomorrow', $transaction_lock_date);
    if ($selected_date_from > $max_date_from) {
      form_set_error('', t('Start date must must not be later than %date.', array('%date' => format_date($max_date_from, 'custom', 'F Y'))));
    }
  }
}

/**
 * Submit handler for avreports_fs_update_form().
 */
function avreports_fs_update_form_submit($form, &$form_state) {
  $selected_date_from = empty($form_state['values']['date_from']) ? NULL : $form_state['values']['date_from'];
  $selected_date_to = empty($form_state['values']['date_to']) ? NULL : $form_state['values']['date_to'];

  $transaction_lock_date = variable_get('avtxns_as_of_date_lock', NULL);
  if (empty($transaction_lock_date)) {
    // Have not scanned ever.
    // Set start date from the very first record in transactions table.
    $date_start = avreports_get_that_transaction_date();
    // Empty table.
    db_query("TRUNCATE TABLE {avtbl_fs}");
    db_query("TRUNCATE TABLE {avtbl_inventory_cache}");
  }
  else {
    // Delete all history for transactions from last update onwards.
    db_query("DELETE FROM {avtbl_fs} WHERE before_this_day > $selected_date_from");
    db_query("DELETE FROM {avtbl_inventory_cache} WHERE before_this_day > $selected_date_from");
    $date_start = $selected_date_from;
  }

  // If no date start, interrupt and send error message.
  if (empty($date_start)) {
    drupal_set_message('There are no transactions recorded yet.', 'warning');
    return;
  }
  $date_start = strtotime('today midnight', $date_start);
  // Set end date.
  $date_end = avreports_batch_period_end($selected_date_to);
  // dpm($date_start . '-' , format_date($date_start, 'long'));
  // dpm($date_end . '-' , format_date($date_end, 'long'));
  // return;
  // Set lock date.
  variable_set('avtxns_as_of_date_lock', strtotime('yesterday', $date_end));

  $operations = array();
  $operations['products'] = array('avreports_batch_scan_inventory_transactions2', array('products', $date_start, $date_end)); // Faster for products.
  $operations['accounts'] = array('avreports_batch_scan_transactions', array('accounts', $date_start, $date_end)); // Wayyyyy faster for accounts.
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
    'init_message' => 'Initializing scan...',
    'progress_message' => 'Completed @current of @total scan operations.',
    // 'progress_message' => NULL,
    'error_message' => 'Scan encountered an error',
    'file' => AVREPORTS_MODULE_PATH . '/inc/fs2.form.inc',
  );
  batch_set($batch);
}

/**
 * Batch process to scan Accounts transaction history.
 *
 * @param string $scan_type
 * @param $date_start
 * @param $date_end
 * @param $context
 */
function avreports_batch_scan_transactions($scan_type, $date_start, $date_end, &$context) {
  if (!isset($context['sandbox']['progress'])) {
    $date_diff = $date_end - $date_start;
    // $context['sandbox']['max'] = round($date_diff / (60 * 60 * 24));
    $context['sandbox']['max'] = $date_diff;

    $context['results'] = array();
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['rows_scanned'] = 0;
    $context['sandbox']['date_start'] = $date_start;
    $context['sandbox']['last_row_id'] = 0;
    $context['sandbox']['start_time'] = time();

    // $context['results']['start_time'] = empty($context['results']['start_time']) ? time() : $context['results']['start_time'];
  }

  // Pointers.
  $_results = &$context['results'];
  $_progress = &$context['sandbox']['progress'];
  $_max = &$context['sandbox']['max'];
  $_date_start = &$context['sandbox']['date_start'];
  $_last_row_id = &$context['sandbox']['last_row_id'];
  $_rows_scanned = &$context['sandbox']['rows_scanned'];

  // Init.
  $period_start = $_date_start;
  $period_end = avreports_batch_period_end($period_start);

  $rows = array();
  switch ($scan_type) {
    case 'products':
      // Set row limit per period.
      $row_limit = 1;

      // $query = "SELECT avprod.id, avprod.title, avprod.initial_qty, avprod.initial_value item_cost, avprod.initial_qty_date,
      //   (avprod.initial_value * avprod.initial_qty) AS asset_value FROM avtbl_transaction_details avtxn_detail
      //   INNER JOIN {avtbl_transactions} avtxn ON avtxn.id = avtxn_detail.transaction_id
      //   INNER JOIN {avtbl_products} avprod ON avprod.id = avtxn_detail.item_id
      //   WHERE avprod.id > $_last_row_id AND avprod.id < 2300
      //   AND avtxn.status <> :void_status AND avtxn.transaction_type IN (:this_types)
      //   AND avtxn.transaction_date >= :date_from
      //   AND avtxn.transaction_date < :date_to
      //   ORDER BY avprod.id LIMIT $row_limit";
      // $rows = db_query($query, array(
      //   ':void_status' => AVTXNS_TXN_STATUS_VOID,
      //   ':this_types' => avtxns_get_qty_change_txn_types(),
      //   ':date_from' => $period_start,
      //   ':date_to' => $period_end,
      // ))->fetchAllAssoc('id');

      $query = "SELECT id, title, initial_qty, initial_value item_cost, initial_qty_date
        FROM {avtbl_products}
        WHERE id > $_last_row_id ORDER BY id LIMIT $row_limit";
      $rows = db_query($query)->fetchAllAssoc('id');
      // $query = "SELECT id, title, initial_qty, initial_value item_cost, initial_qty_date,
      //   (initial_value * initial_qty) AS asset_value FROM {avtbl_products}
      //   WHERE id > $_last_row_id AND id = 4087 ORDER BY id LIMIT $row_limit";
      // $rows = db_query($query)->fetchAllAssoc('id');
      break;
    case 'accounts':
      // Set row limit per period.
      $row_limit = 1;
      $query = "SELECT id, title, account_type FROM {avtbl_account_chart} WHERE id > $_last_row_id AND status = 1 ORDER BY id LIMIT $row_limit";
      $rows = db_query($query)->fetchAllAssoc('id');
      break;
  }


  if (!empty($rows)) {
    // Set period limit per run.
    $period_limit = 1;
    $remaining = $_max - $_progress;
    if ($period_limit > $remaining) {
      $period_limit = $remaining;
    }
    for ($i = 0; $i < $period_limit; $i++) {
      switch ($scan_type) {
        case 'products':
          foreach ($rows as $row) {
            avtxns_read_product_transactions($row, $period_start, $period_end);
            $_last_row_id = $row->id;
            $row_title = $row->title;
            $_rows_scanned++;
          }
          break;
        case 'accounts':
          foreach ($rows as $row) {
            avtxns_read_account_transactions($row->id, $row->account_type, $_date_start, $period_end);
            $_last_row_id = $row->id;
            $row_title = $row->title;
            $_rows_scanned++;
          }
          break;
      }
    }

    // $context['message'] = t('Completed @num @type of %period transactions.<br />Last scanned: %title', array('@num' => $_rows_scanned, '%period' => format_date($period_start, 'custom', 'F Y'), '@type' => $scan_type, '%title' => $row_title));
    $context['message'] = t('<span class="uk-text-capitalize">@type</span> scanned for %period : @progress<br />... %title',
      array(
        '%period' => format_date($period_start, 'custom', 'F Y'),
        '@type' => $scan_type,
        '@progress' => $_rows_scanned,
        '%title' => $row_title,
      )
    );
    $context['message'] .= '<div class="uk-text-muted uk-text-small uk-margin-top">'
      . t('Scanning transactions from @start until end of @end.', array('@start' => format_date($date_start, 'custom', 'F Y'), '@end' => format_date(strtotime('yesterday', $date_end), 'custom', 'F Y')))
      . '</div>';
    $context['message'] .= '<div class="uk-text-muted uk-text-small">' . t('Transactions on or before end of @date are now locked.', array('@date' => format_date(strtotime('yesterday', $date_end), 'custom', 'F Y'))) . '</div>';
    // $context['message'] .= '<div class="uk-text-muted uk-text-small uk-margin-top">' . t('Earliest transaction included on scan: ' . format_date($date_start, 'custom', 'M. d, Y')) . '</div>';
    // $context['message'] .= '<div class="uk-text-muted uk-text-small">' . t('Latest transaction included on scan: ' . format_date(strtotime('yesterday', $date_end), 'custom', 'M. d, Y')) . '</div>';
    $context['message'] .= '<div class="uk-text-muted uk-text-small"><i class="uk-icon-exclamation-triangle uk-color-orange"></i> Void and pending transactions are excluded from scan.</div>';
  }
  else {
    // No more products to scan for this period. Proceed to next period.
    // $period_end = avreports_batch_period_end($period_start);
    // Reset last row id scanned.
    $_last_row_id = 0;
    $_rows_scanned = 0;
    // Update progress counter.
    $_progress += $period_end - $period_start;
    // Update start date for next period.
    $_date_start = $period_end;
  }

  // Compute for progress percentage.
  $finished = $_progress / $_max;
  // if ($finished >= 1) {
  //   $total_time = time() - $context['sandbox']['start_time'];
  //   $minutes = floor($total_time / 60);
  //   $seconds = $total_time - ($minutes * 60);
  //   dpm("{$minutes}m {$seconds}s");
  // }

  // Set progress percentage.
  $context['finished'] = $finished;
}

function avreports_batch_scan_inventory_transactions2($scan_type, $date_start, $date_end, &$context) {
  if (!isset($context['sandbox']['progress'])) {
    // $date_diff = $date_end - $date_start;
    // $context['sandbox']['max'] = round($date_diff / (60 * 60 * 24));
    // $context['sandbox']['max'] = $date_diff;

    if ($scan_type == 'products') {
      $query = "SELECT COUNT(id) FROM {avtbl_products}";
    }
    elseif ($scan_type == 'accounts') {
      $query = "SELECT COUNT(id) FROM {avtbl_account_chart} WHERE status = 1";
    }

    // $context['sandbox']['item_count'] = db_query($query)->fetchField();
    // $context['sandbox']['item_completed'] = 0;
    // $context['sandbox']['max'] = $date_diff * $context['sandbox']['item_count'];
    $context['sandbox']['max'] = db_query($query)->fetchField();
    // $context['sandbox']['max'] = 1;

    $context['results'] = array();
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['rows_scanned'] = 0;
    $context['sandbox']['date_start'] = $date_start;
    $context['sandbox']['last_row_id'] = 0;
    $context['sandbox']['period_start'] = $date_start;
    $context['sandbox']['db_rows'] = array();
    $context['sandbox']['start_time'] = time();
    // $context['results']['start_time'] = empty($context['results']['start_time']) ? time() : $context['results']['start_time'];
  }

  // Pointers.
  $_results = &$context['results'];
  $_progress = &$context['sandbox']['progress'];
  $_max = &$context['sandbox']['max'];
  $_date_start = &$context['sandbox']['date_start'];
  $_period_start = &$context['sandbox']['period_start'];
  $_last_row_id = &$context['sandbox']['last_row_id'];
  // $_rows_scanned = &$context['sandbox']['rows_scanned'];
  // $_item_count = &$context['sandbox']['item_count'];
  // $_item_completed = &$context['sandbox']['item_completed'];

  $_db_rows = &$context['sandbox']['db_rows'];


  // Set row limit.
  $row_limit = 1;


  if (empty($_db_rows)) {
    if ($scan_type == 'products') {
      // $query = "SELECT avprod.id, avprod.title, avprod.initial_qty, avprod.initial_value item_cost, avprod.initial_qty_date,
      // (avprod.initial_value * avprod.initial_qty) AS asset_value FROM {avtbl_transaction_details} avtxn_detail
      // INNER JOIN {avtbl_transactions} avtxn ON avtxn.id = avtxn_detail.transaction_id
      // INNER JOIN {avtbl_products} avprod ON avprod.id = avtxn_detail.item_id
      // WHERE avprod.id > $_last_row_id AND avprod.id < 3500
      // AND avtxn.status <> :void_status AND avtxn.transaction_type IN (:this_types)
      // AND avtxn.transaction_date >= :date_from
      // AND avtxn.transaction_date < :date_to
      // ORDER BY avprod.id LIMIT $row_limit";
      // $_db_rows = db_query($query, array(
      //   ':void_status' => AVTXNS_TXN_STATUS_VOID,
      //   ':this_types' => avtxns_get_qty_change_txn_types(),
      //   ':date_from' => $date_start,
      //   ':date_to' => $date_end,
      // ))->fetchAllAssoc('id');

      // 3500
      $query = "SELECT id, title, initial_qty, initial_value item_cost, initial_qty_date
      FROM {avtbl_products}
      WHERE id > $_last_row_id AND id < 2300
      ORDER BY id LIMIT $row_limit";
      $_db_rows = db_query($query)->fetchAllAssoc('id');
    }
    elseif ($scan_type == 'accounts') {
      $query = "SELECT id, title, account_type FROM {avtbl_account_chart} WHERE id > $_last_row_id AND status = 1 ORDER BY id LIMIT $row_limit";
      $_db_rows = db_query($query)->fetchAllAssoc('id');
    }
  }

  // $query = "SELECT id, title, initial_qty, initial_value item_cost, initial_qty_date,
  //   (initial_value * initial_qty) AS asset_value FROM {avtbl_products}
  //   WHERE id > $_last_row_id AND id = 4087 ORDER BY id LIMIT $row_limit";
  // $_db_rows = db_query($query)->fetchAllAssoc('id');

  $row_title = '-';
  $last_row_id = 0;
  $period_end = avreports_batch_period_end($_period_start);
  foreach ($_db_rows as $row) {
    if ($scan_type == 'products') {
      avtxns_read_product_transactions($row, $_period_start, $period_end);
    }
    elseif ($scan_type == 'accounts') {
      avtxns_read_account_transactions($row->id, $row->account_type, $_date_start, $period_end);
    }
    $last_row_id = $row->id;
    $row_title = $row->title;
  }
  $last_period_scanned = $_period_start;

  if (empty($last_row_id)) {
    $_progress = $_max;
  }
  elseif ($period_end >= $date_end) {
    $_period_start = $_date_start;
    $_last_row_id = $last_row_id;
    // Update progress.
    $_progress += $row_limit;
    // $_item_completed += $row_limit;
    // Reset db row.
    $_db_rows = array();
  }
  else {
    // $_progress += $period_end - $_period_start;
    $_period_start = $period_end;
  }

  $context['message'] = t('<span class="uk-text-capitalize">@type</span> scanned: @progress of @total<br />... %title - @period',
    array(
      '@period' => format_date($last_period_scanned, 'custom', 'F Y'),
      '@type' => $scan_type,
      '@progress' => $_progress,
      '@total' => $_max,
      '%title' => $row_title
    )
  );
  $context['message'] .= '<div class="uk-text-muted uk-text-small uk-margin-top">'
    . t('Scanning transactions from @start until end of @end.', array('@start' => format_date($date_start, 'custom', 'F Y'), '@end' => format_date(strtotime('yesterday', $date_end), 'custom', 'F Y')))
    . '</div>';
  $context['message'] .= '<div class="uk-text-muted uk-text-small">' . t('Transactions on or before end of @date are now locked.', array('@date' => format_date(strtotime('yesterday', $date_end), 'custom', 'F Y'))) . '</div>';
  // $context['message'] .= '<div class="uk-text-muted uk-text-small uk-margin-top">' . t('Earliest transaction included on scan: ' . format_date($date_start, 'custom', 'M. d, Y')) . '</div>';
  // $context['message'] .= '<div class="uk-text-muted uk-text-small">' . t('Latest transaction included on scan: ' . format_date(strtotime('yesterday', $date_end), 'custom', 'M. d, Y')) . '</div>';
  $context['message'] .= '<div class="uk-text-muted uk-text-small"><i class="uk-icon-exclamation-triangle uk-color-orange"></i> Void and pending transactions are excluded from scan.</div>';



  // Compute for progress percentage.
  $finished = $_progress / $_max;
  // if ($finished >= 1) {
  //   $total_time = time() - $context['sandbox']['start_time'];
  //   $minutes = floor($total_time / 60);
  //   $seconds = $total_time - ($minutes * 60);
  //   dpm("{$minutes}m {$seconds}s");
  // }

  // Set progress percentage.
  $context['finished'] = $finished;
}

function avreports_batch_scan_inventory_transactions3($scan_type, $date_start, $date_end, &$context) {
  if (!isset($context['sandbox']['progress'])) {
    if ($scan_type == 'products') {
      $query = "SELECT COUNT(id) FROM {avtbl_products}";
    }
    elseif ($scan_type == 'accounts') {
      $query = "SELECT COUNT(id) FROM {avtbl_account_chart} WHERE status = 1";
    }

    $context['sandbox']['max'] = db_query($query)->fetchField();

    // $context['results'] = array();
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['last_row_id'] = 0;
    // $context['sandbox']['start_time'] = time();
    $context['results']['start_time'] = empty($context['results']['start_time']) ? time() : $context['results']['start_time'];
  }
// dpm($context['results']['start_time']);
  // Pointers.
  $_results = &$context['results'];
  $_progress = &$context['sandbox']['progress'];
  $_max = &$context['sandbox']['max'];
  // $_date_start = &$context['sandbox']['date_start'];
  $_last_row_id = &$context['sandbox']['last_row_id'];


  // Set row limit.
  $row_limit = 1;
  if ($scan_type == 'products') {
    $query = "SELECT avprod.id, avprod.title, avprod.initial_qty, avprod.initial_value item_cost, avprod.initial_qty_date,
      (avprod.initial_value * avprod.initial_qty) AS asset_value FROM avtbl_transaction_details avtxn_detail
      INNER JOIN {avtbl_transactions} avtxn ON avtxn.id = avtxn_detail.transaction_id
      INNER JOIN {avtbl_products} avprod ON avprod.id = avtxn_detail.item_id
      WHERE avprod.id > $_last_row_id
      AND avtxn.status <> :void_status AND avtxn.transaction_type IN (:this_types)
      AND avtxn.transaction_date >= :date_from
      AND avtxn.transaction_date < :date_to
      ORDER BY avprod.id LIMIT $row_limit";
    $_db_rows = db_query($query, array(
      ':void_status' => AVTXNS_TXN_STATUS_VOID,
      ':this_types' => avtxns_get_qty_change_txn_types(),
      ':date_from' => $date_start,
      ':date_to' => $date_end,
    ))->fetchAllAssoc('id');
    // $query = "SELECT id, title, initial_qty, initial_value item_cost, initial_qty_date,
    //   (initial_value * initial_qty) AS asset_value FROM {avtbl_products}
    //   WHERE id > $_last_row_id AND id < 2215 ORDER BY id LIMIT $row_limit";
  }
  elseif ($scan_type == 'accounts') {
    $query = "SELECT id, title, account_type FROM {avtbl_account_chart} WHERE id > $_last_row_id AND status = 1 ORDER BY id LIMIT $row_limit";
    $_db_rows = db_query($query)->fetchAllAssoc('id');
  }



  // $query = "SELECT id, title, initial_qty, initial_value item_cost, initial_qty_date,
  //   (initial_value * initial_qty) AS asset_value FROM {avtbl_products}
  //   WHERE id > $_last_row_id AND id = 4087 ORDER BY id LIMIT $row_limit";
  // $_db_rows = db_query($query)->fetchAllAssoc('id');



  if (empty($_db_rows)) {
    $_progress = $_max;
  }
  else {
    $row_title = '-';
    foreach ($_db_rows as $row) {
      if ($scan_type == 'products') {
        avtxns_read_product_transactions($row, $date_start, $date_end);
      }
      elseif ($scan_type == 'accounts') {
        avtxns_read_account_transactions($row->id, $row->account_type, $date_start, $date_end);
      }
      $_last_row_id = $row->id;
      $row_title = $row->title;
    }
    $last_period_scanned = $date_start;
    $_progress += $row_limit;

    $context['message'] = t('<span class="uk-text-capitalize">@type</span> scanned for %period : @progress of @total<br />... %title',
      array(
        '%period' => format_date($last_period_scanned, 'custom', 'F Y'),
        '@type' => $scan_type,
        '@progress' => $_progress,
        '@total' => $_max,
        '%title' => $row_title
      )
    );
  }



  // $context['message'] .= '<div class="uk-text-muted uk-text-small uk-margin-top">' . t('Earliest transaction included on scan: ' . format_date($date_start, 'custom', 'M. d, Y')) . '</div>';
  // $context['message'] .= '<div class="uk-text-muted uk-text-small">' . t('Latest transaction included on scan: ' . format_date(strtotime('yesterday', $date_end), 'custom', 'M. d, Y')) . '</div>';
  $context['message'] .= '<div class="uk-text-muted uk-text-small uk-margin-top"><i class="uk-icon-exclamation-triangle uk-color-orange"></i> Void and pending transactions are excluded from scan.</div>';



  // Compute for progress percentage.
  $finished = $_progress / $_max;
  // if ($finished >= 1) {
  //   $total_time = time() - $context['sandbox']['start_time'];
  //   $minutes = floor($total_time / 60);
  //   $seconds = $total_time - ($minutes * 60);
  //   dpm("{$minutes}m {$seconds}s");
  // }

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
// dpm($results);
//     if (!empty($results['start_time'])) {
//       $total_time = time() - $results['start_time'];
//       $minutes = floor($total_time / 60);
//       $seconds = $total_time - ($minutes * 60);
//       dpm("{$minutes}m {$seconds}s");
//     }

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

function avreports_batch_period_end($start_date) {
  // Monthly.
  return strtotime('first day of next month', $start_date);
  // Daily.
  // return strtotime('tomorrow midnight', $start_date);
}
