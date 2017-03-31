<?php

/**
 * Profit and Loss Report Form.
 * @param $form
 * @param $form_state
 */
function avreports_profit_loss_form($form, &$form_state) {
  $dad = avtxns_txn_list_filter_date_auto_options();
  $dad_default = empty($dad['last_month_to_date']) ? array() : $dad['last_month_to_date'];
  $filter_form_settings = array(
    'date_auto' => array(
      '#title' => t('Dates'),
      '#default_value' => 'last_month_to_date',
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
  $table_columns = array();
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
    $tc_label = '';
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
    $table_columns[$ts] = array(
      'date_from' => $ts,
      'date_to' => $last_day_timestamp,
      'label' => $tc_label,
    );
    $ts = strtotime('tomorrow', $last_day_timestamp);
  }

  // Headers.
  $header = array();
  $header['account'] = '';
  foreach ($table_columns as $tk => $tc) {
    $header[$tk] = $tc['label'];
  }
  $header['total'] = 'Total';

  // Get account charts.
  $profit_loss_account_types = array(AVTXN_AC_TYPE_INCOME, AVTXN_AC_TYPE_OTHER_INCOME, AVTXN_AC_TYPE_COGS, AVTXN_AC_TYPE_EXPENSE);
  $accounts_where = $accounts_args = array();
  $accounts_where[] = 'avachart.account_type IN (:account_types)';
  $accounts_args[':account_types'] = $profit_loss_account_types;
  $accounts = avtxns_account_chart_query_load($accounts_where, $accounts_args);
  //dpm($accounts);

  // Build query.
  $pl_transaction_types = array(AVTXNS_TXN_TYPE_INVOICE);
  $pl_transaction_statuses = array(AVTXNS_TXN_STATUS_CLOSED, AVTXNS_TXN_STATUS_OPEN);
  $select[] = 'avtxn_detail.*';
  $select[] = 'avtxn.transaction_type';
  $select['grand_total'] = avtxns_txn_detail_sum_query() . ' AS grand_total';
  $joins[] = 'INNER JOIN {avtbl_transactions} AS avtxn ON avtxn.id = avtxn_detail.transaction_id';
  $where[] = 'avtxn.transaction_type IN (:txn_types)';
  $where[] = 'avtxn.status IN (:txn_statuses)';
  $args[':txn_types'] = $pl_transaction_types;
  $args[':txn_statuses'] = $pl_transaction_statuses;
  $group_by = array('avtxn.id', 'avtxn_detail.ref_txn_type');
  $query = avreports_profit_loss_query($where, $select, $joins, $group_by);
  $rs = db_query($query, $args);
  $account_rows = array();
  foreach ($rs as $row) {
    $ttd = avtxns_txn_types($row->transaction_type);
    $ref_txn_type = empty($row->ref_txn_type) ? '-' : $row->ref_txn_type;
    $account_ids = empty($ttd['account_ids'][$ref_txn_type]) ? array() : $ttd['account_ids'][$ref_txn_type];
    foreach ($account_ids as $a_id => $a_type) {
      $this_account_type = isset($accounts[$a_id]->account_type) ? $accounts[$a_id]->account_type : '';
      if (!in_array($this_account_type, $profit_loss_account_types)) {
        continue;
      }
      //$profit_loss_account_types
      $account_rows[$a_id]['txn_details'][] = array(
        'transaction_id' => $row->transaction_id,
        'transaction_type' => $row->transaction_type,
        'account_id' => $a_id,
        'account_type' => $this_account_type,
        $a_type => $row->grand_total,
      );
    }
  }


  // Do the same but for Journals.
  unset($select['grand_total']);
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
  $args[':account_ids'] = array_keys($accounts);
  $query = avreports_profit_loss_query($where, $select, $joins);
  $rs = db_query($query, $args);
  foreach ($rs as $row) {
    $account_row = array(
      'transaction_id' => $row->transaction_id,
      'transaction_type' => $row->transaction_type,
      'account_id' => $row->account_id,
      'account_type' => isset($accounts[$row->account_id]->account_type) ? $accounts[$row->account_id]->account_type : '',
    );
    if ($row->transaction_type == AVTXNS_TXN_TYPE_JOURNAL) {
      $account_row['debit'] = $row->debit;
      $account_row['credit'] = $row->credit;
      $account_rows[$row->account_id]['txn_details'][] = $account_row;
    }
    else {
      // For other trans, fill up account_id, debit, credit.
      $ttd = avtxns_txn_types($row->transaction_type);
      if (!empty($ttd['default_account_id'])) {
        foreach ($ttd['default_account_id'] as $a_id => $a_type) {
          // One for the default account.
          $this_account_row = $account_row;
          $this_account_row['account_id'] = $a_id;
          $this_account_row[$a_type] = $row->cost;
          $account_rows[$a_id]['txn_details'][] = $this_account_row;
          // Another one for the entered account.
          $entered_account_balance_type = $a_type == 'credit' ? 'debit' : 'credit';
          $this_account_row = $account_row;
          $this_account_row[$entered_account_balance_type] = $row->cost;
          $account_rows[$row->account_id]['txn_details'][] = $this_account_row;
        }
      }
    }
  }

  // Compute total balance per account.
  foreach ($account_rows as $r_k => $row) {
    $balance_total = 0;
    foreach ($row['txn_details'] as $txn_detail) {
      $actd = avtxns_account_chart_types($txn_detail['account_type']);
      $td_credit = empty($txn_detail['credit']) ? 0 : $txn_detail['credit'];
      $td_debit = empty($txn_detail['debit']) ? 0 : $txn_detail['debit'];
      $balance_total += $actd['balance_type'] == 'debit' ? $td_debit : $td_credit;
      $balance_total -= $actd['balance_type'] == 'credit' ? $td_debit : $td_credit;
    }
    $account_rows[$r_k]['account_balance'] = $balance_total;

  }

  // Additional account information.
  foreach ($account_rows as $r_k => $row) {
    $account_rows[$r_k]['id'] = $r_k;
    $account_rows[$r_k]['title'] = isset($accounts[$r_k]->title) ? $accounts[$r_k]->title : '';
    $account_rows[$r_k]['parent'] = isset($accounts[$r_k]->parent) ? $accounts[$r_k]->parent : NULL;
  }
dpm($accounts);
dpm($account_rows);
  $account_rows_tree = avbase_rows_build_tree($account_rows, 0, FALSE, FALSE);
dpm($account_rows_tree);
  avbase_account_chart_sum_parent_balance($account_rows_tree);
dpm($account_rows_tree);
return $form;
  $accounts_tree = avbase_rows_build_tree($accounts, 0, FALSE);
  dpm($accounts_tree);


  $table_rows = array();
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

function avreports_profit_loss_query($where, $select, $joins, $group_by = NULL) {
  $query = "SELECT " . implode(', ', $select) . " FROM {avtbl_transaction_details} AS avtxn_detail " . implode(' ', $joins);
  if (!empty($where)) {
    $query .= " WHERE " . implode(' AND ', $where);
  }
  if (!empty($group_by)) {
    $query .= " GROUP BY " . implode(', ', $group_by);
  }
  return $query;
}