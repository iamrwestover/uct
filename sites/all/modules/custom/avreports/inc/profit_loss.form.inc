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
  $profit_loss_account_types = array(AVTXN_AC_TYPE_INCOME, AVTXN_AC_TYPE_OTHER_INCOME, AVTXN_AC_TYPE_COGS, AVTXN_AC_TYPE_EXPENSE);
  $accounts_where = $accounts_args = array();
  $accounts_where[] = 'avachart.account_type IN (:account_types)';
  $accounts_args[':account_types'] = $profit_loss_account_types;
  $profit_loss_accounts = avtxns_account_chart_query_load($accounts_where, $accounts_args);

  // Build query.
  $pl_transaction_types = array(AVTXNS_TXN_TYPE_INVOICE);
  $pl_transaction_statuses = array(AVTXNS_TXN_STATUS_CLOSED, AVTXNS_TXN_STATUS_OPEN);
  $select[] = 'avtxn_detail.*';
  $select[] = 'avtxn.transaction_type';
  $select[] = 'avtxn.transaction_date';
  //$select[] = "(DATE_FORMAT(FROM_UNIXTIME(avtxn.transaction_date) ,'%M 01 %Y')) AS period";
  $select['grand_total'] = avtxns_txn_detail_sum_query() . ' AS grand_total';
  $select['grand_total_discounted'] = avtxns_txn_detail_sum_query_discount_applied() . ' AS grand_total_discounted';
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
    $ref_txn_type = empty($row->ref_txn_type) ? 'default' : $row->ref_txn_type;
    $account_ids = empty($ttd['account_ids'][$ref_txn_type]) ? array() : $ttd['account_ids'][$ref_txn_type];
    foreach ($account_ids as $a_id => $a_type) {
      $this_account_type = isset($profit_loss_accounts[$a_id]->account_type) ? $profit_loss_accounts[$a_id]->account_type : '';
      if (!in_array($this_account_type, $profit_loss_account_types)) {
        continue;
      }
      //$profit_loss_account_types
      $account_rows[$a_id][] = array(
        'transaction_id' => $row->transaction_id,
        'transaction_date' => $row->transaction_date,
        'transaction_type' => $row->transaction_type,
        'account_id' => $a_id,
        'account_type' => $this_account_type,
        $a_type => $row->grand_total,
      );
    }

    // Add sales discount if available and if necessary.
    if (in_array(AVTXN_AC_TYPE_INCOME, $profit_loss_account_types) && !empty($row->grand_total_discounted)) {
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


  // Do the same but for Journals.
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
  $args[':account_ids'] = array_keys($profit_loss_accounts);
  $query = avreports_profit_loss_query($where, $select, $joins);
  $rs = db_query($query, $args);
  foreach ($rs as $row) {
    $account_row = array(
      'transaction_id' => $row->transaction_id,
      'transaction_date' => $row->transaction_date,
      'transaction_type' => $row->transaction_type,
      'account_id' => $row->account_id,
      'account_type' => isset($profit_loss_accounts[$row->account_id]->account_type) ? $profit_loss_accounts[$row->account_id]->account_type : '',
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
          if (in_array($a_id, array_keys($profit_loss_accounts))) {
            $this_account_row = $account_row;
            $this_account_row['account_id'] = $a_id;
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


  // Get product ids from cogs transactions.
  $cogs_query = "SELECT avtxn.id, avtxn.transaction_type, avtxn.transaction_date, avtxn_detail.item_id
    FROM {avtbl_transaction_details} AS avtxn_detail
    INNER JOIN {avtbl_transactions} AS avtxn ON avtxn.id = avtxn_detail.transaction_id
    WHERE avtxn_detail.item_id AND avtxn.transaction_type IN (:ttypes) AND avtxn.status IN (:txn_statuses)";
  $cogs_args = array();
  $cogs_args[':ttypes'] = array(AVTXNS_TXN_TYPE_SALES_RETURN, AVTXNS_TXN_TYPE_DELIVERY);
  $cogs_args[':txn_statuses'] = $pl_transaction_statuses;
  $cogs_rs = db_query($cogs_query, $cogs_args);
  $cogs_transaction_ids = array();
  foreach ($cogs_rs as $r_k => $row) {
    $cogs_transaction_ids[$row->id]['product_ids'][$row->item_id] = $row->item_id;
    $cogs_transaction_ids[$row->id]['row'] = $row;
  }
  avreports_get_transaction_ave_cost($cogs_transaction_ids);
  foreach ($cogs_transaction_ids as $cti) {
    $row = $cti['row'];
    $ttd = avtxns_txn_types($row->transaction_type);
    $ref_txn_type = empty($row->ref_txn_type) ? 'default' : $row->ref_txn_type;
    $account_ids = empty($ttd['account_ids'][$ref_txn_type]) ? array() : $ttd['account_ids'][$ref_txn_type];
    foreach ($account_ids as $a_id => $a_type) {
      $this_account_type = isset($profit_loss_accounts[$a_id]->account_type) ? $profit_loss_accounts[$a_id]->account_type : '';
      if (!in_array($this_account_type, $profit_loss_account_types)) {
        continue;
      }
      //$profit_loss_account_types
      $account_rows[$a_id][] = array(
        'transaction_id' => $row->id,
        'transaction_date' => $row->transaction_date,
        'transaction_type' => $row->transaction_type,
        'account_id' => $a_id,
        'account_type' => $this_account_type,
        $a_type => $cti['cog'],
      );
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

  //dpm($account_rows_by_period);
  //return $form;
  //foreach ($account_rows as $r_k => $row) {
  //  foreach($row['txn_details'] as $txn_detail) {
  //    foreach ($date_group as $dg) {
  //      if (!isset($account_rows_by_period[$dg['date_from']]['accounts'])) {
  //        $account_rows_by_period[$dg['date_from']]['accounts'] = array();
  //      }
  //      if ($txn_detail['transaction_date'] <= $dg['date_to']) {
  //        $account_rows_by_period[$dg['date_from']]['accounts'][$r_k]['txn_details'][] = $txn_detail;
  //        break;
  //      }
  //    }
  //  }
  //}


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
      $row['account_type'] = isset($profit_loss_accounts[$r_k]->account_type) ? $profit_loss_accounts[$r_k]->account_type : '-';
      $row['title'] = isset($profit_loss_accounts[$r_k]->title) ? $profit_loss_accounts[$r_k]->title : '';
      $row['parent'] = isset($profit_loss_accounts[$r_k]->parent) ? $profit_loss_accounts[$r_k]->parent : NULL;
      $account_rows_by_period[$p_k][$r_k] = $row;
    }
  }

  $account_rows_no_parenting = $account_rows_by_period;

  //// Build parent-child relationship.
  //foreach ($account_rows_by_period as $p_k => $period) {
  //  $account_rows_by_period[$p_k] = avbase_rows_build_tree($period, 0, FALSE, FALSE);
  //  // Get total children balance.
  //  avbase_account_chart_sum_parent_balance($account_rows_by_period[$p_k]);
  //}
  //
  //// Group by account type.
  ////$account_rows_by_type = array();
  ////foreach ($account_rows_by_period as $p_k => $period) {
  ////  foreach ($period as $r_k => $row) {
  ////    $account_rows_by_type[$row['account_type']][$p_k][$r_k] = $row;
  ////  }
  ////}
  //
  //$account_rows_by_type = array();
  //foreach ($account_rows_by_period as $p_k => $period) {
  //  foreach ($period as $r_k => $row) {
  //    $account_rows_by_type[$row['account_type']][$r_k][$p_k] = $row;
  //    $account_rows_by_type[$row['account_type']]['#account_type'] = $row['account_type'];
  //    $account_rows_by_type[$row['account_type']]['#title'] = $row['title'];
  //  }
  //}
  //
  //
  //// Compute total balance per account type.
  ////foreach ($account_rows_by_type as $at_k => $periods) {
  ////  $account_type_total = 0;
  ////  foreach ($periods as $p_k => $rows) {
  ////    $period_total = 0;
  ////    foreach ($rows as $row) {
  ////      $period_total += $row['account_balance'] + $row['children_account_balance'];
  ////      $account_type_total += $period_total;
  ////    }
  ////    $account_rows_by_type[$at_k][$p_k]['#period_total'] = $period_total;
  ////    $account_rows_by_type[$at_k][$p_k]['#period_total_text'] = avbase_accounting_number_format($period_total);
  ////  }
  ////  $account_rows_by_type[$at_k]['#at_total'] = $account_type_total;
  ////  $account_rows_by_type[$at_k]['#at_total_text'] = avbase_accounting_number_format($account_type_total);
  ////}
  //foreach ($account_rows_by_type as $at_k => $rows) {
  //  $account_type_total = 0;
  //  foreach (element_children($rows) as $r_k) {
  //    $period = $rows[$r_k];
  //    $account_total = 0;
  //    foreach ($period as $p_k => $p_info) {
  //      $account_type_total += $p_info['account_balance'] + $p_info['children_account_balance'];
  //      $account_total += $p_info['account_balance'];
  //    }
  //    $account_rows_by_type[$at_k][$r_k]['#account_total'] = $account_total;
  //    $account_rows_by_type[$at_k][$r_k]['#account_total_text'] = avbase_accounting_number_format($account_total);
  //  }
  //  $account_rows_by_type[$at_k]['#at_total'] = $account_type_total;
  //  $account_rows_by_type[$at_k]['#at_total_text'] = avbase_accounting_number_format($account_type_total);
  //}

  // chart.
  $profit_loss_accounts = avbase_rows_build_tree($profit_loss_accounts, 0, FALSE, FALSE);
  $pl_accounts_by_type = array();
  foreach ($profit_loss_accounts as $pl) {
    $pl_accounts_by_type[$pl['account_type']][$pl['id']] = $pl;
  }


  // Create table.
  $table_rows = array();
  $at_totals = array();
  // INCOME.
  avreports_account_type_balance_rows($table_rows, $pl_accounts_by_type, AVTXN_AC_TYPE_INCOME, $account_rows_no_parenting, $date_group, $at_totals);

  // COGS.
  avreports_account_type_balance_rows($table_rows, $pl_accounts_by_type, AVTXN_AC_TYPE_COGS, $account_rows_no_parenting, $date_group, $at_totals);
  // Gross Profit Row.
  avreports_account_type_difference_row($table_rows, 'GROSS PROFIT / (LOSS)', $at_totals, AVTXN_AC_TYPE_INCOME, AVTXN_AC_TYPE_COGS, $date_group);

  // EXPENSES.
  avreports_account_type_balance_rows($table_rows, $pl_accounts_by_type, AVTXN_AC_TYPE_EXPENSE, $account_rows_no_parenting, $date_group, $at_totals);

  // Operating Income Row.
  $totals_id = AVTXN_AC_TYPE_INCOME . '.' . AVTXN_AC_TYPE_COGS;
  avreports_account_type_difference_row($table_rows, 'OPERATING INCOME', $at_totals, $totals_id, AVTXN_AC_TYPE_EXPENSE, $date_group);

  // Other Income.
  avreports_account_type_balance_rows($table_rows, $pl_accounts_by_type, AVTXN_AC_TYPE_OTHER_INCOME, $account_rows_no_parenting, $date_group, $at_totals);
  // Spacer.
  $data = array();
  $data['account'] = array('data' => '', 'colspan' => 1000);
  $table_rows[] = array('data' => $data);
  // Net Income Row.
  $totals_id = AVTXN_AC_TYPE_INCOME . '.' . AVTXN_AC_TYPE_COGS . '.' . AVTXN_AC_TYPE_EXPENSE;
  avreports_account_type_difference_row($table_rows, 'NET INCOME', $at_totals, $totals_id, AVTXN_AC_TYPE_OTHER_INCOME, $date_group, FALSE);


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

  if (!empty($user_filter['date_from'])) {
    $date_text = empty($user_filter['date_from']) ? '' : check_plain($user_filter['date_from']);
    if (!empty($user_filter['date_to']) && $user_filter['date_from'] != $user_filter['date_to']) {
      $date_text .= ' - ' . $user_filter['date_to'];
    }
    $transaction_list_table['date_from'] = array(
      '#type' => 'item',
      '#markup' => $date_text,
    );
  }

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