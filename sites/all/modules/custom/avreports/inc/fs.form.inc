<?php

/**
 * Build an fs form.
 * @param $form
 * @param $form_state
 * @param $type
 *
 * @return mixed
 */
function avreports_fs_form($form, &$form_state, $type) {
  ini_set('memory_limit', AVBASE_EXPANDED_MEMORY_LIMIT);
  $dad = avtxns_txn_list_filter_date_auto_options();
  $dad_default = empty($dad['last_month']) ? array() : $dad['last_month'];
  $filter_form_settings = array(
    'date_auto' => array(
      '#title' => t('Dates'),
      '#default_value' => 'last_month',
    ),
    'date_from' => array(
      '#title' => t('Date from'),
      '#default_value' => $dad_default['date_from'],
    ),
    'date_to' => array(
      '#title' => t('Date to'),
      '#default_value' => $dad_default['date_to'],
    ),
    'reset_button' => array('#attributes' => array('style' => 'margin-top: 25px;')),
  );
  $form['transaction_list_filter'] = avtxns_txn_list_filter_form($form, $form_state, $filter_form_settings);

  // Prepare query variables.
  $select = $joins = $where = $args = array();
  $user_filter = empty($form_state['values']['avtxns_tl_filter']) ? array() : $form_state['values']['avtxns_tl_filter'];
  $filter_query = empty($user_filter['filter_query']) ? array() : $user_filter['filter_query'];
  $where = empty($filter_query['where']) ? array() : $filter_query['where'];
  $args = empty($filter_query['args']) ? array() : $filter_query['args'];

  // Add table columns depending on date difference.
  $date_from_timestamp = strtotime($user_filter['date_from']);
  $date_to_timestamp = strtotime($user_filter['date_to']);
  $date_group = array();
  $ts = $date_from_timestamp;
  while ($ts <= $date_to_timestamp) {
    $last_day_timestamp = strtotime('last day of this month', $ts);
    $last_day_timestamp = $last_day_timestamp > $date_to_timestamp ? $date_to_timestamp : $last_day_timestamp;
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
    $date_group[$ts] = array(
      'date_from' => $ts,
      'date_to' => $last_day_timestamp,
      'label' => $tc_label,
    );
    $ts = strtotime('tomorrow', $last_day_timestamp);
  }

  // Headers.
  $header = array();
  $header['account'] = '';
  foreach ($date_group as $tk => $tc) {
    $header[$tk] = array('data' => $tc['label'], 'class' => array('uk-text-right'));
  }
  $header['total'] = array('data' => 'Total', 'class' => array('uk-text-right'));

  // Get account charts.
  $account_types = array();
  switch ($type) {
    case 'profit-loss':
      $account_types = array(AVTXN_AC_TYPE_INCOME, AVTXN_AC_TYPE_OTHER_INCOME, AVTXN_AC_TYPE_COGS, AVTXN_AC_TYPE_EXPENSE);
      $fs_transaction_types = array(AVTXNS_TXN_TYPE_INVOICE);
      $fs_invalid_statuses = array(AVTXNS_TXN_STATUS_PENDING, AVTXNS_TXN_STATUS_VOID);
      break;
    case 'balance-sheet':
      $account_types = array(AVTXN_AC_TYPE_CURRENT, AVTXN_AC_TYPE_FIXED, AVTXN_AC_TYPE_LIABILITY, AVTXN_AC_TYPE_CAPITAL, AVTXN_AC_TYPE_AR, AVTXN_AC_TYPE_AP);
      // For AR:
      $ar_transaction_types = array(AVTXNS_TXN_TYPE_INVOICE, AVTXNS_TXN_TYPE_PAYMENT, AVTXNS_TXN_TYPE_SALES_RETURN);
      // For AP:
      $ap_transaction_types = array(AVTXNS_TXN_TYPE_BILL, AVTXNS_TXN_TYPE_PAY_BILL, AVTXNS_TXN_TYPE_PURCHASE_RETURN);
      $fs_transaction_types = array_merge($ar_transaction_types, $ap_transaction_types);
      // Other AP and AR related transactions like creditmemo and vendor credit will be included in accounts related query.
      $fs_invalid_statuses = array(AVTXNS_TXN_STATUS_PENDING, AVTXNS_TXN_STATUS_VOID);
      break;
  }
  $accounts_where = $accounts_args = array();
  // We cannot limit account ids for BS because all entries in creditmemo,ccharge,etc., affects AP/AR regardless of entered account.
  if ($type != 'balance-sheet') {
    $accounts_where[] = 'avachart.account_type IN (:account_types)';
    $accounts_args[':account_types'] = $account_types;
  }
  $rs = avtxns_account_chart_rs_load($accounts_where, $accounts_args);
  $fs_accounts = $rs->fetchAllAssoc('id');

  // Build query.
  $select[] = 'avtxn_detail.*';
  $select[] = 'avtxn.transaction_type';
  $select[] = 'avtxn.transaction_date';
  $select['grand_total'] = avtxns_txn_detail_sum_query() . ' AS grand_total';
  $select['grand_total_discounted'] = avtxns_txn_detail_sum_query_discount_applied() . ' AS grand_total_discounted';
  $joins[] = 'INNER JOIN {avtbl_transactions} AS avtxn ON avtxn.id = avtxn_detail.transaction_id';
  $where[] = 'avtxn.transaction_type IN (:txn_types)';
  $where[] = 'avtxn.status NOT IN (:txn_statuses)';
  $args[':txn_types'] = $fs_transaction_types;
  $args[':txn_statuses'] = $fs_invalid_statuses;
  $group_by = array('avtxn.id', 'avtxn_detail.ref_txn_type');
  $query = avreports_fs_query($where, $select, $joins, $group_by);
  $rs = db_query($query, $args);
  $account_rows = array();

  foreach ($rs as $row) {
    $ttd = avtxns_txn_types($row->transaction_type);
    $ref_txn_type = empty($row->ref_txn_type) ? 'default' : $row->ref_txn_type;
    $account_ids = empty($ttd['account_ids'][$ref_txn_type]) ? array() : $ttd['account_ids'][$ref_txn_type];

    foreach ($account_ids as $a_id => $a_type) {
      $this_account_type = isset($fs_accounts[$a_id]->account_type) ? $fs_accounts[$a_id]->account_type : '';
      if (!in_array($this_account_type, $account_types)) {
        continue;
      }

      // MAke sure discounted amount is used for AR and AP.
      $total_amount = $row->grand_total;
      if (in_array($a_id, array(AVTXN_AC_SPECIFIC_AR_ID, AVTXN_AC_SPECIFIC_AP_ID))) {
        $total_amount = $row->grand_total_discounted;
      }

      $account_rows[$a_id][] = array(
        'transaction_id' => $row->transaction_id,
        'transaction_date' => $row->transaction_date,
        'transaction_type' => $row->transaction_type,
        'account_id' => $a_id,
        'account_type' => $this_account_type,
        $a_type => $total_amount,
      );
    }

    // Add sales discount if available and if necessary.
    if (in_array(AVTXN_AC_TYPE_INCOME, $account_types) && !empty($row->grand_total_discounted)) {
      $discounted_amount = $row->grand_total - $row->grand_total_discounted;
      $discounted_amount = round((float)$discounted_amount, 2);
      if (!empty($discounted_amount)) {
        $account_rows[AVTXN_AC_SPECIFIC_SALES_DISCOUNT_ID][] = array(
          'transaction_id' => $row->transaction_id,
          'transaction_date' => $row->transaction_date,
          'transaction_type' => $row->transaction_type,
          'account_id' => AVTXN_AC_SPECIFIC_SALES_DISCOUNT_ID,
          'account_type' => AVTXN_AC_TYPE_INCOME,
          'debit' => $discounted_amount,
        );
      }
    }
  }

  // Do the same but for accounts related transactions.
  unset($select['grand_total']);
  unset($select['grand_total_discounted']);
  $ttypes = array();
  $transaction_types = avtxns_txn_types();
  foreach ($transaction_types as $t_k => $ttd) {
    if ($ttd['transaction'] == 'accounts') {
      $ttypes[] = $t_k;
    }
  }
  $select[] = 'avtxn_detail.cost';
  $where[] = 'avtxn_detail.account_id IN (:account_ids)';
  $args[':txn_types'] = $ttypes;
  $args[':account_ids'] = array_keys($fs_accounts);
  $query = avreports_fs_query($where, $select, $joins);
  $rs = db_query($query, $args);

  foreach ($rs as $row) {
    $account_row = array(
      'transaction_id' => $row->transaction_id,
      'transaction_date' => $row->transaction_date,
      'transaction_type' => $row->transaction_type,
      'account_id' => $row->account_id,
      'account_type' => isset($fs_accounts[$row->account_id]->account_type) ? $fs_accounts[$row->account_id]->account_type : '',
    );
    if ($row->transaction_type == AVTXNS_TXN_TYPE_JOURNAL) {
      $account_row['debit'] = $row->debit;
      $account_row['credit'] = $row->credit;
      $account_rows[$row->account_id][] = $account_row;
    }
    else {
      // For other trans, fill up account_id, debit, credit.
      $ttd = avtxns_txn_types($row->transaction_type);
      if (!empty($ttd['default_account_id'])) {
        foreach ($ttd['default_account_id'] as $a_id => $a_type) {
          // One for the default account. Only if this account id is included in the allowed ones.
          if (in_array($a_id, array_keys($fs_accounts))) {
            $this_account_row = $account_row;
            $this_account_row['account_id'] = $a_id;
            $this_account_row['account_type'] = isset($fs_accounts[$a_id]->account_type) ? $fs_accounts[$a_id]->account_type : '';
            $this_account_row[$a_type] = $row->cost;
            $account_rows[$a_id][] = $this_account_row;
          }
          // Another one for the entered account.
          $entered_account_balance_type = $a_type == 'credit' ? 'debit' : 'credit';
          $this_account_row = $account_row;
          $this_account_row[$entered_account_balance_type] = $row->cost;
          $account_rows[$row->account_id][] = $this_account_row;
        }
      }
    }
  }

  // Group by period.
  $account_rows_by_period = array();
  foreach ($date_group as $dg) {
    $period_accounts = array();
    foreach ($account_rows as $a_k => $txn_details) {
      $period_txn_details = array();
      foreach ($txn_details as $r_k => $row) {
        if ($dg['date_from'] <= $row['transaction_date'] && $dg['date_to'] >= $row['transaction_date']) {
          $period_txn_details[] = $row;
        }
      }
      $period_accounts[$a_k]['txn_details'] = $period_txn_details;
    }
    $account_rows_by_period[$dg['date_from']] = $period_accounts;
  }

  // Compute total balance per account.
  foreach ($account_rows_by_period as $p_k => $period) {
    foreach ($period as $r_k => $row) {
      $account_total = 0;
      foreach ($row['txn_details'] as $txn_detail) {
        $actd = avtxns_account_chart_types($txn_detail['account_type']);
        $td_credit = empty($txn_detail['credit']) ? 0 : $txn_detail['credit'];
        $td_debit = empty($txn_detail['debit']) ? 0 : $txn_detail['debit'];
        $account_total += $actd['balance_type'] == 'debit' ? $td_debit : $td_credit;
        $account_total -= $actd['balance_type'] == 'credit' ? $td_debit : $td_credit;
      }
      $account_rows_by_period[$p_k][$r_k]['account_balance'] = $account_total;
    }
  }

  // Additional account information.
  foreach ($account_rows_by_period as $p_k => $period) {
    foreach ($period as $r_k => $row) {
      $row['id'] = $r_k;
      $row['account_type'] = isset($fs_accounts[$r_k]->account_type) ? $fs_accounts[$r_k]->account_type : '-';
      $row['title'] = isset($fs_accounts[$r_k]->title) ? $fs_accounts[$r_k]->title : '';
      $row['parent'] = isset($fs_accounts[$r_k]->parent) ? $fs_accounts[$r_k]->parent : NULL;
      $account_rows_by_period[$p_k][$r_k] = $row;
    }
  }

  $account_rows_no_parenting = $account_rows_by_period;

  // chart.
  $fs_accounts = avbase_rows_build_tree($fs_accounts, 0, FALSE, FALSE);
  $pl_accounts_by_type = array();
  foreach ($fs_accounts as $pl) {
    $pl_accounts_by_type[$pl['account_type']][$pl['id']] = $pl;
  }

  // Put Accounts Receivable under Current Assets.
  if (isset($pl_accounts_by_type[AVTXN_AC_TYPE_AR]) && isset($pl_accounts_by_type[AVTXN_AC_TYPE_CURRENT])) {
    $pl_accounts_by_type[AVTXN_AC_TYPE_CURRENT] = $pl_accounts_by_type[AVTXN_AC_TYPE_AR] + $pl_accounts_by_type[AVTXN_AC_TYPE_CURRENT];
    unset($pl_accounts_by_type[AVTXN_AC_TYPE_AR]);
  }

  // Create table.
  $table_rows = array();
  $at_totals = array();
  if ($type == 'profit-loss') {
    // INCOME.
    avreports_account_type_balance_rows($table_rows, $pl_accounts_by_type, AVTXN_AC_TYPE_INCOME, $account_rows_no_parenting, $date_group, $at_totals);
    // COGS.
    avreports_account_type_balance_rows($table_rows, $pl_accounts_by_type, AVTXN_AC_TYPE_COGS, $account_rows_no_parenting, $date_group, $at_totals);
    // Gross Profit Row.
    avreports_account_type_difference_row($table_rows, 'GROSS PROFIT / (LOSS)', $at_totals, AVTXN_AC_TYPE_INCOME, AVTXN_AC_TYPE_COGS, $date_group);
    // EXPENSES.
    avreports_account_type_balance_rows($table_rows, $pl_accounts_by_type, AVTXN_AC_TYPE_EXPENSE, $account_rows_no_parenting, $date_group, $at_totals, TRUE, $user_filter);
    // Operating Income Row.
    $totals_id = AVTXN_AC_TYPE_INCOME . '.' . AVTXN_AC_TYPE_COGS;
    avreports_account_type_difference_row($table_rows, 'OPERATING INCOME', $at_totals, $totals_id, AVTXN_AC_TYPE_EXPENSE, $date_group);
    // Other Income.
    avreports_account_type_balance_rows($table_rows, $pl_accounts_by_type, AVTXN_AC_TYPE_OTHER_INCOME, $account_rows_no_parenting, $date_group, $at_totals, TRUE, $user_filter);
    // Spacer.
    $data = array();
    $data['account'] = array('data' => '', 'colspan' => 1000);
    $table_rows[] = array('data' => $data);
    // Net Income Row.
    $totals_id = AVTXN_AC_TYPE_INCOME . '.' . AVTXN_AC_TYPE_COGS . '.' . AVTXN_AC_TYPE_EXPENSE;
    avreports_account_type_difference_row($table_rows, 'NET INCOME', $at_totals, $totals_id, AVTXN_AC_TYPE_OTHER_INCOME, $date_group, FALSE);
  }
  elseif ($type == 'balance-sheet') {
    // Accounts Receivables.
    // avreports_account_type_balance_rows($table_rows, $pl_accounts_by_type, AVTXN_AC_TYPE_AR, $account_rows_no_parenting, $date_group, $at_totals);
    // Current Assets.
    avreports_account_type_balance_rows($table_rows, $pl_accounts_by_type, AVTXN_AC_TYPE_CURRENT, $account_rows_no_parenting, $date_group, $at_totals, TRUE, $user_filter);
    // Fixed Assets.
    avreports_account_type_balance_rows($table_rows, $pl_accounts_by_type, AVTXN_AC_TYPE_FIXED, $account_rows_no_parenting, $date_group, $at_totals, TRUE, $user_filter);
    // Total Assets.
    avreports_account_type_difference_row($table_rows, 'TOTAL ASSETS', $at_totals, AVTXN_AC_TYPE_CURRENT, AVTXN_AC_TYPE_FIXED, $date_group, FALSE);
    // Liabilities.
    avreports_account_type_balance_rows($table_rows, $pl_accounts_by_type, AVTXN_AC_TYPE_LIABILITY, $account_rows_no_parenting, $date_group, $at_totals, TRUE, $user_filter);
    // Capital.
    avreports_account_type_balance_rows($table_rows, $pl_accounts_by_type, AVTXN_AC_TYPE_CAPITAL, $account_rows_no_parenting, $date_group, $at_totals, TRUE, $user_filter);
    // Total Assets.
    avreports_account_type_difference_row($table_rows, 'TOTAL LIABILITIES AND CAPITAL', $at_totals, AVTXN_AC_TYPE_LIABILITY, AVTXN_AC_TYPE_CAPITAL, $date_group, FALSE);
  }


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

  avtxns_txn_list_table_add_date_from($transaction_list_table, $user_filter);

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

function avreports_fs_query($where, $select, $joins, $group_by = NULL) {
  $query = "SELECT " . implode(', ', $select) . " FROM {avtbl_transaction_details} AS avtxn_detail " . implode(' ', $joins);
  if (!empty($where)) {
    $query .= " WHERE " . implode(' AND ', $where);
  }
  if (!empty($group_by)) {
    $query .= " GROUP BY " . implode(', ', $group_by);
  }
  return $query;
}