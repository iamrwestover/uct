<?php

/**
 * Build a trial balance form.
 * @param $form
 * @param $form_state
 *
 * @return mixed
 */
function avreports_trial_balance_form($form, &$form_state) {
  $transaction_lock_date = variable_get('avtxns_as_of_date_lock', NULL);
  if (empty($transaction_lock_date)) {
    $default_date = strtotime('today', time());
    $default_date = strtotime('last day of last month', $default_date);
  }
  else {
    $default_date = $transaction_lock_date;
  }
  $last_scan_date = $transaction_lock_date;
  $filter_form_settings = array(
    'date_to' => array(
      '#title' => t('Period'),
      '#default_value' => format_date($default_date, 'custom', AVBASE_MONTH_ONLY_FORMAT),
      '#av_date_format' => AVBASE_MONTH_ONLY_FORMAT,
    ),
    'reset_button' => array('#button_label' => 'This month', '#attributes' => array('style' => 'margin-top: 25px;')),
  );
  $form['transaction_list_filter'] = avtxns_txn_list_filter_form($form, $form_state, $filter_form_settings, AVBASE_MONTH_ONLY_FORMAT);

  $update_button_class = '';
  $button_tooltip = '';
  $update_button_html = l('<i class="uk-icon-search"></i><br />Scan', 'av/reports/fs-update-options', array(
    'html' => TRUE,
    'attributes' => array('class' => array('uk-button uk-button-mini uk-margin-small-right' . $update_button_class), 'data-uk-tooltip' => "{cls: 'tt-blue'}", 'title' => $button_tooltip),
    'query' => drupal_get_destination(),
  ));
  $update_notif_html = $update_button_html;
  $form['transaction_list_filter']['#prefix'] = '<div class="uk-float-right uk-text-small uk-text-right">' . $update_notif_html. '</div>';

  // Prepare query variables.
  $select = $joins = $where = $args = $group_by = array();
  $user_filter = empty($form_state['values']['avtxns_tl_filter']) ? array() : $form_state['values']['avtxns_tl_filter'];
  $filter_query = empty($user_filter['filter_query']) ? array() : $user_filter['filter_query'];
  $where = empty($filter_query['where']) ? array() : $filter_query['where'];
  $args = empty($filter_query['args']) ? array() : $filter_query['args'];
  $date_start = $args[':date_to_stamp'];
  $date_end = strtotime('first day of next month', $args[':date_to_stamp']);


  // Header.
  $header = array();
  $header['account_code'] = array('data' => 'ACCOUNT CODE');
  $header['account_title'] = array('data' => 'ACCOUNT TITLE');
  $header['debit_ftm'] = array('data' => 'DEBIT (FTM)', 'class' => array('uk-text-right'));
  $header['debit_ftm']['data-uk-tooltip'] = '{cls: "tt-blue"}';
  $header['debit_ftm']['title'] = t('For the month of @date', array('@date' => format_date($date_start, 'custom', 'F Y')));

  $header['credit_ftm'] = array('data' => 'CREDIT (FTM)', 'class' => array('uk-text-right'));
  $header['credit_ftm']['data-uk-tooltip'] = $header['debit_ftm']['data-uk-tooltip'];
  $header['credit_ftm']['title'] = $header['debit_ftm']['title'];

  $header['debit_ytd'] = array('data' => 'DEBIT (YTD)', 'class' => array('uk-text-right'));
  $header['debit_ytd']['data-uk-tooltip'] = '{cls: "tt-blue"}';
  $header['debit_ytd']['title'] = t('Year to Date of @date', array('@date' => format_date($date_start, 'custom', 'Y')));

  $header['credit_ytd'] = array('data' => 'CREDIT (YTD)', 'class' => array('uk-text-right'));
  $header['credit_ytd']['data-uk-tooltip'] = $header['debit_ytd']['data-uk-tooltip'];
  $header['credit_ytd']['title'] = $header['debit_ytd']['title'];

  // Set table pagination and sort settings.
  if (!empty($user_filter['settings']['table_href'])) {
    $table_ajax_href = $user_filter['settings']['table_href'];
    $href_parts = parse_url($table_ajax_href);
    if (!empty($href_parts['query'])) {
      parse_str($href_parts['query'], $href_query);
      foreach ($href_query as $k => $v) {
        $_GET[$k] = $v;
      }
    }
  }
  $order_by = avbase_build_header_order_array($header);



  // Build table rows.
  $table_rows = array();

  $fiscal_date = variable_get('av_fiscal_date', 0);
  $fiscal_date_end = strtotime('first day of next month', $fiscal_date);
  $show_fiscal_beg_bal = $fiscal_date_end <= $date_end && $fiscal_date_end >= strtotime('first day of this month', $date_end);

  // Build query for FTM.
  // Account chart.
  $select[] = 'avachart.*';
  $where[] = 'avachart.status = 1';
  // FS cache table.
  $select['debit'] = 'avfs.debit debit_ftm';
  $select['credit'] = 'avfs.credit credit_ftm';
  $joins['avfs'] = "LEFT JOIN avtbl_fs avfs ON avfs.account_id = avachart.id AND avfs.before_this_day = :end_timestamp AND avfs.before_this_day > $fiscal_date_end";
  if ($show_fiscal_beg_bal) {
    $select['debit'] = 'SUM(avfs.debit) debit_ftm';
    $select['credit'] = 'SUM(avfs.credit) credit_ftm';
    $joins['avfs'] = "LEFT JOIN avtbl_fs avfs ON avfs.account_id = avachart.id AND avfs.before_this_day <= :end_timestamp";
    $group_by[] = 'avachart.id';
  }
  $args[':end_timestamp'] = $date_end;
  // Remove unused where.
  unset($where['date_to']);
  unset($args[':date_to_stamp']);
  $query = "SELECT " . implode(', ', $select) . " FROM {avtbl_account_chart} AS avachart " . implode(' ', $joins);
  avbase_query_supplements($query, $where, $order_by, NULL, NULL, $group_by);
  $fs_accounts = db_query($query, $args)->fetchAllAssoc('id');

  // Build query for YTD.
  $select = $joins = $where = $group_by = array();
  $ytd_end_timestamp = strtotime('first day of January next year', $date_start);
  $select[] = 'avfs.account_id';
  $select[] = 'SUM(avfs.debit) debit_ytd';
  $select[] = 'SUM(avfs.credit) credit_ytd';
  $group_by[] = 'avfs.account_id';
  $where[] = 'avfs.before_this_day <= :end_timestamp AND avfs.before_this_day > :year_start_timestamp';
  $args[':year_start_timestamp'] = strtotime('first day of January this year', $date_start);
  $args[':end_timestamp'] = $ytd_end_timestamp;
  $query = "SELECT " . implode(', ', $select) . " FROM {avtbl_fs}  avfs " . implode(' ', $joins);
  avbase_query_supplements($query, $where, array(), NULL, NULL, $group_by);
  $accounts_ytd = db_query($query, $args)->fetchAllAssoc('account_id');

  // Append ytd data into fs_accounts.
  foreach ($fs_accounts as $fsa_id => $fsa) {
    $fs_accounts[$fsa_id]->debit_ytd = isset($accounts_ytd[$fsa_id]->debit_ytd) ? $accounts_ytd[$fsa_id]->debit_ytd : 0;
    $fs_accounts[$fsa_id]->credit_ytd = isset($accounts_ytd[$fsa_id]->credit_ytd) ? $accounts_ytd[$fsa_id]->credit_ytd : 0;
  }

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

  $totals = array();
  foreach ($fs_accounts_by_type as $account_type => $accounts) {
    foreach ($accounts as $account) {
      avreports_trial_balance_row($table_rows, $totals, $account, $date_end);
    }
  }

  // Add "totals" row.
  if (!empty($table_rows)) {
    $totals_table_row = array();
    $colspan = array_search('debit_ftm', array_keys($header));
    if (!empty($colspan)) {
      $totals_table_row[] = array('data' => '<div class="uk-text-right">TOTAL</div>', 'colspan' => $colspan);
      $totals_table_row[] = '<div class="uk-text-right">' . avbase_accounting_number_format($totals['debit_ftm']) . '</div>';
      $totals_table_row[] = '<div class="uk-text-right">' . avbase_accounting_number_format($totals['credit_ftm']) . '</div>';
      $totals_table_row[] = '<div class="uk-text-right">' . avbase_accounting_number_format($totals['debit_ytd']) . '</div>';
      $totals_table_row[] = '<div class="uk-text-right">' . avbase_accounting_number_format($totals['credit_ytd']) . '</div>';
      $remaining_cols = count($header) - ($colspan + (count($totals_table_row) - 1));
      if ($remaining_cols) {
        $totals_table_row[] = array('data' => NULL, 'colspan' => $remaining_cols);
      }
      if (!empty($totals_table_row)) {
        $table_rows[] = array(array('data' => NULL, 'colspan' => count($header)));
        $table_rows[] = $totals_table_row;
      }
    }
  }





  $transaction_list_table['table'] = array(
    '#header' => $header,
    '#empty' => '<div class="uk-text-muted">' . t('No record found.') . '</div>',
    '#attributes' => array(
      'id' => 'transaction-list-table',
      'class' => array('uk-table-condensed uk-table-hover uk-text-small'),
    ),
  );
  $transaction_list_table['table']['#theme'] = 'table';
  $transaction_list_table['table']['#rows'] = $table_rows;
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
  $form['transaction_list_table'] += $transaction_list_table;

  $form['buttons']['#theme'] = 'avbase_crud_button_group';
  $form['buttons']['print'] = array(
    '#id' => 'av-print-btn',
    '#type' => 'button',
    '#value' => t('Print...'),
    '#visible_in_view_mode' => TRUE,
    '#icon_key' => 'print',
  );

  // Hack to minimize caching size.
  if (empty($transaction_list_table['table']['#type']) || $transaction_list_table['table']['#type'] != 'tableselect') {
    $form['#form_keys_no_cache'] = array('transaction_list_table');
  }
  return $form;
}

function avreports_trial_balance_row(&$table_rows, &$totals, $account, $date_end) {
  $actd = avtxns_account_chart_types($account['account_type']);
  $account_balance_type = $actd['balance_type'];
  if ($account_balance_type == 'debit') {
    $account['credit_ftm'] *= -1;
    $account['credit_ytd'] *= -1;
  }
  else {
    // Swap.
    $_credit = $account['debit_ftm'];
    $account['debit_ftm'] = $account['credit_ftm'];
    $account['credit_ftm'] = $_credit;
    $account['debit_ftm'] *= -1;
    $account['debit_ytd'] *= -1;
  }


  $history_link_date = array('date_from' => format_date(strtotime('first day of last month', $date_end), 'custom', AVREPORTS_LIST_DATE_FORMAT), 'date_to' => format_date(strtotime('last day of last month', $date_end), 'custom', AVREPORTS_LIST_DATE_FORMAT));


  $data = array();
  $data['account_code'] = check_plain($account['account_code']);
  $data['account_title'] = check_plain($account['title']);
  $amount_text = avreports_account_history_link($account['title'], $history_link_date, avbase_accounting_number_format($account['debit_ftm']), TRUE, $account['id']);
  $data['debit_ftm'] = '<div class="uk-text-right">' . $amount_text . '</div>';
  $amount_text = avreports_account_history_link($account['title'], $history_link_date, avbase_accounting_number_format($account['credit_ftm']), TRUE, $account['id']);
  $data['credit_ftm'] = '<div class="uk-text-right">' . $amount_text . '</div>';
  $data['debit_ytd'] = '<div class="uk-text-right">' . avbase_accounting_number_format($account['debit_ytd']) . '</div>';
  $data['credit_ytd'] = '<div class="uk-text-right">' . avbase_accounting_number_format($account['credit_ytd']) . '</div>';
  // Careful when adding classes on rows, check if tableselect mode is ON.
  $table_rows[] = $data;

  $totals['debit_ftm'] = (isset($totals['debit_ftm']) ? $totals['debit_ftm'] : 0) + $account['debit_ftm'];
  $totals['credit_ftm'] = (isset($totals['credit_ftm']) ? $totals['credit_ftm'] : 0) + $account['credit_ftm'];
  $totals['debit_ytd'] = (isset($totals['debit_ytd']) ? $totals['debit_ytd'] : 0) + $account['debit_ytd'];
  $totals['credit_ytd'] = (isset($totals['credit_ytd']) ? $totals['credit_ytd'] : 0) + $account['credit_ytd'];
  if (!empty($account['children'])) {
    foreach ($account['children'] as $account_child) {
      avreports_trial_balance_row($table_rows, $totals, $account_child, $date_end);
    }
  }
}