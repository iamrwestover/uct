<?php

/**
 * Return form array containing client's balance ages.
 * @param $client_id
 * @return array|NULL
 */
function avreports_client_balance_aging_form($client_id) {
  $data = &drupal_static(__FUNCTION__, array());
  if (isset($data[$client_id])) {
    return $data[$client_id];
  }

  $client = avclient_load($client_id);
  if (empty($client->id)) {
    $data[$client_id] = NULL;
    return $data[$client_id];
  }

  $invoice_where = array(
    array('avclients.id', $client->id),
    array('avtxns.transaction_type', array(AVTXNS_TXN_TYPE_INVOICE, AVTXNS_TXN_TYPE_CUSTOMER_CHARGES), 'IN'),
    array('avtxns.status', array(AVTXNS_TXN_STATUS_LOCKED, AVTXNS_TXN_STATUS_OPEN), 'IN'),
  );
  $invoices = avtxns_txn_query_load($invoice_where);
  $now = time() /*+ (86400*44)*/;
  $balance_ages = array(0 => 0);
  $term_days = empty($client->term_data['days']) ? 30 : $client->term_data['days'];
  if (!empty($term_days)) {
    for ($x = 1; $x <= 11; $x++) {
      $balance_ages[$term_days * $x] = 0;
    }
  }

  //if (isset($client->opening_balance)) {
  //  $opening_balance = isset($client->opening_balance) ? (float) $client->opening_balance : 0;
  //  if (!empty($opening_balance)) {
  //    $invoices['opening_balance'] = (object) array('balance' => $opening_balance, 'transaction_date' => $client->created);
  //  }
  //}

  end($balance_ages);
  $last_balance_days = key($balance_ages);
  foreach ($invoices as $invoice) {
    $balance_age = ($now - $invoice->transaction_date) / 86400;
    if ($balance_age < 1 && !empty($invoice->term_id)) {
      $days = 0;
    }
    else {
      $balance_age = ceil($balance_age);
      $days = ceil($balance_age / $term_days) * $term_days;
    }


    if ($days == 0) {
      // COD.
      $balance_ages[0] += $invoice->balance;
    }
    elseif (isset($balance_ages[$days])) {
      $balance_ages[$days] += $invoice->balance;
    }
    elseif ($days >= $last_balance_days) {
      $balance_ages[$last_balance_days] += $invoice->balance;
    }
  }

  $balance_ages_form = array('#theme' => 'avreports_balance_ages');
  $ages_count = 0;
  $total_balance = 0;
  foreach ($balance_ages as $k => $balance) {
    $red = $k > 60;
    $total_balance += $balance;
    $age_title = $k == 0 ? 'C.O.D' : t('@from-@to days old', array('@from' => $k - $term_days + 1, '@to' => $k));
    $balance_ages_form[$k] = array(
      '#type' => 'item',
      '#title' => $red ? "<em>$age_title</em>" : $age_title,
      '#markup' => number_format(round($balance, 2), 2),
    );
    if ($red) {
      $balance_ages_form[$k]['#markup'] = '<em>' . $balance_ages_form[$k]['#markup'] . '</em>';
    }

    $ages_count++;
    if ($ages_count == count($balance_ages)) {
      $balance_ages_form[$k]['#title'] = t('@from++ days old', array('@from' => $k - $term_days));
      if ($red) {
        $balance_ages_form[$k]['#title'] = '<em>' . $balance_ages_form[$k]['#title'] . '</em>';
      }
    }
  }

  $credit_limit = empty($client->credit_limit) ? 0 : (float) $client->credit_limit;
  $balance_ages_form['#total_balance'] = round($total_balance, 2);
  $balance_ages_form['#credit_limit'] = $credit_limit;
  $balance_ages_form['#limit_reached'] = !empty($credit_limit) && round($total_balance, 2) >= round($credit_limit, 2);
  $balance_ages_form['#client_type'] = $client->client_type;
  $balance_ages_form['#client_status'] = $client->status;

  $data[$client_id] = $balance_ages_form;
  return $data[$client_id];
}

function avreports_account_type_balance_rows(&$table_rows, $pl_accounts_by_type, $account_type, $arnp, $date_group, &$at_totals, $spacer_at_bottom = TRUE, $user_filter = array()) {
  // Account Type info.
  $actd = avtxns_account_chart_types($account_type);

  // Header.
  $data = array();
  $data['account'] = array('data' => '<div class="uk-text-uppercase uk-text-bold">' . $actd['name'] . '</div>', 'colspan' => 1000);
  $table_rows[] = array('data' => $data);

  // Details.
  $pl_account_rows = empty($pl_accounts_by_type[$account_type]) ? array() : $pl_accounts_by_type[$account_type];
  $at_totals[$account_type] = array();
  $account_type_table_rows = avreports_account_type_balance_row_children($pl_account_rows, $arnp, $date_group, $at_totals[$account_type], 0, $user_filter);
  //avbase_account_chart_sum_parent_balance($pl_account_rows);
  $table_rows = array_merge($table_rows, $account_type_table_rows);

  // Total.
  $data = array();
  $data['account'] = '<div class="uk-text-bold">Total ' . $actd['name'] . '</div>';
  foreach ($date_group as $tk => $tc) {
    $period_total = isset($at_totals[$account_type][$tk]) ? $at_totals[$account_type][$tk] : 0;
    $this_total_text = avbase_accounting_number_format($period_total);
    $data[$tk] = '<div class="uk-text-bold uk-text-right">' . $this_total_text . '</div>';
  }
  $at_total = isset($at_totals[$account_type]['at_total']) ? $at_totals[$account_type]['at_total'] : 0;
  $at_total_text = avbase_accounting_number_format($at_total);
  $data['total'] = '<div class="uk-text-bold uk-text-right">' . $at_total_text . '</div>';
  $table_rows[] = $data;

  // Spacer.
  if ($spacer_at_bottom) {
    $data = array();
    $data['account'] = array('data' => '&nbsp;', 'colspan' => 1000);
    $table_rows[] = $data;
  }
}

function avreports_account_type_balance_row_children(&$account_rows, $arnp, $date_group, &$account_totals, $depth = 0, $user_filter = array()) {
  $table_rows = array();

  foreach ($account_rows as $r_k => $account_row) {
    $depth_html = '';
    for ($i = 0; $i < $depth; $i++) {
      $depth_html .= '<span class="uk-margin-left"></span>';
    }

    // Do not link to history if account type is a match.
    $link_to_history = TRUE;
    if (in_array($account_row['account_type'], array(AVTXN_AC_TYPE_AR, AVTXN_AC_TYPE_AP)) || in_array($account_row['id'], array(AVTXN_AC_SPECIFIC_CASH_ID, AVTXN_AC_SPECIFIC_INVENTORY_ID))) {
      $link_to_history = FALSE;
    }

    $data = array();
    $data['account'] = $depth_html . ($link_to_history ? avreports_account_history_link($account_row['title'], $user_filter) : $account_row['title']);
    // $data['account'] .= $account_row['id'];
    $this_account_balance = 0;
    foreach ($date_group as $tk => $tc) {
      $this_period_balance = isset($arnp[$tk][$r_k]['account_balance']) ? $arnp[$tk][$r_k]['account_balance'] : 0;
      $this_account_balance += $this_period_balance;
      $account_totals[$tk] = isset($account_totals[$tk]) ? $account_totals[$tk] + $this_period_balance : $this_period_balance;
      $data[$tk] = '<div class="uk-text-right">' . avbase_accounting_number_format($this_period_balance) . '</div>';
    }
    $this_account_balance_text = avbase_accounting_number_format($this_account_balance);
    $account_rows[$r_k]['account_balance'] = $this_account_balance;
    $account_totals['at_total'] = isset($account_totals['at_total']) ? $account_totals['at_total'] + $this_account_balance : $this_account_balance;

    $data['total'] = '<div class="uk-text-right">' . $this_account_balance_text . '</div>';
    $table_rows[] = $data;
    //$account_type_total += $account_row['account_balance'];

    if (!empty($account_row['children'])) {
      $table_rows = array_merge($table_rows, avreports_account_type_balance_row_children($account_rows[$r_k]['children'], $arnp, $date_group, $account_totals, $depth + 1, $user_filter));
    }
  }

  return $table_rows;
}

function avreports_account_type_difference_row(&$table_rows, $account_label, &$at_totals, $type1, $type2, $date_group, $is_diff = TRUE) {
  $this_total = 0;
  $type1_total = isset($at_totals[$type1]['at_total']) ? $at_totals[$type1]['at_total'] : 0;
  $type2_total = isset($at_totals[$type2]['at_total']) ? $at_totals[$type2]['at_total'] : 0;
  if ($is_diff) {
    $this_total = $type1_total - $type2_total;
  }
  else {
    $this_total = $type1_total + $type2_total;
  }
  $totals_id = $type1 . '.' . $type2;


  $data = array();
  $data['account'] = '<div class="uk-text-bold">' . $account_label . '</div>';
  foreach ($date_group as $tk => $tc) {
    $period_total1 = isset($at_totals[$type1][$tk]) ? $at_totals[$type1][$tk] : 0;
    $period_total2 = isset($at_totals[$type2][$tk]) ? $at_totals[$type2][$tk] : 0;
    if ($is_diff) {
      $period_total = $period_total1 - $period_total2;
    }
    else {
      $period_total = $period_total1 + $period_total2;
    }
    $period_total_text = avbase_accounting_number_format($period_total);
    $data[$tk] = '<div class="uk-text-bold uk-text-right">' . $period_total_text . '</div>';

    $at_totals[$totals_id][$tk] = $period_total;
  }
  $this_total_text = avbase_accounting_number_format($this_total);
  $data['total'] = '<div class="uk-text-bold uk-text-right">' . $this_total_text . '</div>';
  $table_rows[] = $data;
  // Spacer.
  $data = array();
  $data['account'] = array('data' => '&nbsp;', 'colspan' => 1000);
  $table_rows[] = $data;

  $at_totals[$totals_id]['at_total'] = $this_total;
}

function avreports_product_transaction_history() {
  $product_history_rows = &drupal_static(__FUNCTION__, array());
  if (!empty($product_history_rows)) {
    return $product_history_rows;
  }

  //(IF(avtxn.transaction_type = :stk_type, 0, SUM(avtxn_detail.qty * avtxn_detail.qty_per_uom))
  //SUM(avtxn_detail.cost * avtxn_detail.qty) / SUM(avtxn_detail.qty * avtxn_detail.qty_per_uom) as txn_ave_cost,
  //SUM(avtxn_detail.cost * avtxn_detail.qty) / SUM(avtxn_detail.qty * avtxn_detail.qty_per_uom) as txn_ave_cost,
  $query = "SELECT
    avtxn.id, avtxn.transaction_type, avtxn.transaction_date,
    avtxn_detail.item_id,
    SUM(avtxn_detail.qty_per_uom * avtxn_detail.qty) AS txn_qty,
    SUM(avtxn_detail.cost * avtxn_detail.qty) as txn_value,

    avprod.initial_qty, avprod.initial_value
    FROM {avtbl_transaction_details} AS avtxn_detail
    INNER JOIN {avtbl_transactions} AS avtxn ON avtxn.id = avtxn_detail.transaction_id
    INNER JOIN {avtbl_products} AS avprod ON avprod.id = avtxn_detail.item_id
    WHERE avtxn.transaction_type IN (:ttypes) AND avtxn.status IN (:statuses)
    AND IF(avtxn.transaction_type = :sr, avtxn_detail.good_stock = 1, 1)
    GROUP BY avtxn.id, avtxn_detail.item_id
    ORDER BY avtxn.transaction_date
  ";
  $args = array();
  $args[':ttypes'] = avtxns_get_qty_change_txn_types();
  //$args[':ttypes'] = array(AVTXNS_TXN_TYPE_RECEIVE);
  //$args[':ttypes'] = array(AVTXNS_TXN_TYPE_STOCK_ADJUSTMENT);
  //$args[':stk_type'] = array(AVTXNS_TXN_TYPE_STOCK_ADJUSTMENT);
  $args[':sr'] = AVTXNS_TXN_TYPE_SALES_RETURN;
  $args[':statuses'] = array(AVTXNS_TXN_STATUS_LOCKED, AVTXNS_TXN_STATUS_OPEN);
  $rows = array();
  $rs = db_query($query, $args);
  foreach ($rs as $row) {
    $rows[$row->item_id]['initial_qty'] = $row->initial_qty;
    $rows[$row->item_id]['initial_value'] = $row->initial_value;
    unset($row->initial_qty);
    unset($row->initial_value);
    $rows[$row->item_id]['txn_details'][$row->id] = $row;
    //$rows[$row->item_id]['#initial_ave_cost'] = $row->initial_ave_cost;
  }

  foreach ($rows as $product) {
    $asset_value = empty($product['initial_value']) ? 0 : $product['initial_value'];
    $on_hand_qty = empty($product['initial_qty']) ? 0 : $product['initial_qty'];
    $ave_cost = !empty($on_hand_qty) ? $asset_value / $on_hand_qty : 0;
    foreach ($product['txn_details'] as $txn_id => $txn_detail) {
      $ttd = avtxns_txn_types($txn_detail->transaction_type);
      $qty_change = empty($ttd['qty_change']) ? '' : $ttd['qty_change'];
      $txn_qty = empty($txn_detail->txn_qty) ? 0 : $txn_detail->txn_qty;



      //$product['txn_details'][$txn_id]->asset_value = $asset_value;
      if ($qty_change == 'increment') {
        $on_hand_qty += $txn_qty;
      }
      elseif ($qty_change == 'decrement') {
        $on_hand_qty -= $txn_qty;
      }
      else {
        continue;
      }



      //if ($txn_detail->id == 64) {
      //  //dpm($txn_value);
      //  dpm($on_hand_qty);
      //  dpm($asset_value);
      //  dpm($txn);
      //}
      if ($txn_detail->transaction_type == AVTXNS_TXN_TYPE_RECEIVE) {

        // Compute new average cost.
        $txn_value = empty($txn_detail->txn_value) ? 0 : $txn_detail->txn_value;

        $asset_value += $txn_value;

        $ave_cost = !empty($on_hand_qty) ? $asset_value / $on_hand_qty : $ave_cost;
        $cog = $txn_value;
      }
      else {
        $asset_value = $on_hand_qty * $ave_cost;
        $cog = $ave_cost * $txn_qty;
        //if ($txn_detail->item_id == 2088 && $txn_detail->id == 49) {
        //  dpm($ttd['qty_change']);
        //  dpm($txn_qty);
        //  dpm($txn);
        //  dpm($on_hand_qty);
        //  dpm($on_hand_qty * $ave_cost);
        //  dpm($asset_value);
        //}
      }

      $product['txn_details'][$txn_id]->ave_cost = $ave_cost;
      $product['txn_details'][$txn_id]->on_hand_qty = $on_hand_qty;
      $product['txn_details'][$txn_id]->asset_value = $asset_value;
      $product['txn_details'][$txn_id]->cog = $cog;
    }
  }

  $product_history_rows = $rows;
  return $product_history_rows;
}

function avreports_get_transaction_ave_cost(&$transaction_ids) {
  $history_rows = avreports_product_transaction_history();
  foreach ($transaction_ids as $txn_id => $txn) {
    $cog_total = 0;
    foreach ($txn['product_ids'] as $product_id) {
      $cog_value = empty($history_rows[$product_id]['txn_details'][$txn_id]->cog) ? 0 : $history_rows[$product_id]['txn_details'][$txn_id]->cog;
      $cog_total += $cog_value;
    }
    $transaction_ids[$txn_id]['cog'] = $cog_total;
  }
}

function avreports_report_live_preview(&$form) {
  $live_preview = isset($_GET['preview']) ? $_GET['preview'] : 1;
  if ($live_preview != 1) {
    unset($form['transaction_list_table']);
    unset($form['buttons']['print']);
  }
}

function avreports_count_max_rows(&$form, &$query, $select, $where, $group_by, $args, $offset) {
  $rows_max = 1;
  $count_query = str_replace('_expression_', '1 AS expression', $query);
  $query = str_replace('_expression_', implode(', ', $select), $query);
  if ($offset === 0) {
    // Compute total record count.
    avbase_query_supplements($count_query, $where, NULL, NULL, NULL, $group_by);
    $count_query = "SELECT COUNT(*) AS expression FROM ($count_query) subquery";
    $rows_max = db_query($count_query, $args)->fetchField();
    // dpm($rows_max);
  }
  $form['#report_rows_max'] = $rows_max;
}

function avreports_count_max_rows_stock_status(&$form, &$query, $select, $where, $group_by, $args, $offset, $joins, $all_products = FALSE) {
  $rows_max = 1;
  // $count_query = str_replace('_expression_', '1 AS expression', $query);
  $query = str_replace('_expression_', implode(', ', $select), $query);
  if ($offset === 0) {
    // Compute total record count.
    if ($all_products) {

      $group_by = array();
      unset($joins['avtxn']);
      unset($joins['avtxn_detail']);
      unset($args[':void_type']);
      unset($args[':ttypes']);
      unset($args[':date_to_stamp']);
      unset($args[':date_from_stamp']);
    }

    $count_query = "SELECT 1 AS expression FROM {avtbl_products} AS avprod " . implode(' ', $joins);
    avbase_query_supplements($count_query, $where, array(), NULL, NULL, $group_by);
    $count_query = "SELECT COUNT(*) AS expression FROM ($count_query) subquery";
    $rows_max = db_query($count_query, $args)->fetchField();
  }
  $form['#report_rows_max'] = $rows_max;
}

function avreports_account_history_link($title, $date = array(), $link_title = NULL, $mute_link = TRUE, $account_id = NULL) {
  if (empty($date)) {
    return $title;
  }
  $link_title = is_null($link_title) ? $title : $link_title;
  $link_options = array('attributes' => array('target' => '_blank'));
  if ($mute_link) {
    // $link_options['attributes']['class'] = array('uk-link-muted');
  }
  $link_options['query']['custom_reset'] = 1; // Important.
  $link_options['query']['account_name'] = $title;
  $link_options['query']['date_auto'] = 'custom';
  $link_options['query']['date_from'] = $date['date_from'];
  $link_options['query']['date_to'] = $date['date_to'];
  $link_options['attributes']['title'] = 'View this account\'s history';

  switch ($account_id) {
    case AVTXN_AC_SPECIFIC_INVENTORY_ID:
    case AVTXN_AC_SPECIFIC_COGS_ID:
    $path = 'av/reports/stock-history-summary';
      $link_options['query']['date_from'] = format_date(strtotime($link_options['query']['date_from']), 'custom', AVBASE_MONTH_ONLY_FORMAT);
      $link_options['query']['date_to'] = format_date(strtotime($link_options['query']['date_to']), 'custom', AVBASE_MONTH_ONLY_FORMAT);
      break;
    default:
      $path = 'av/reports/account-history';
  }
  return l($link_title, $path, $link_options);
}

function avreports_product_history_link($title, $date = array(), $link_title = NULL, $mute_link = TRUE) {
  // if (empty($date)) {
  //   return $title;
  // }
  $link_title = is_null($link_title) ? $title : $link_title;
  $link_options = array('attributes' => array('target' => '_blank'));
  if ($mute_link) {
    // $link_options['attributes']['class'] = array('uk-link-muted');
  }
  $link_options['query']['custom_reset'] = 1; // Important.
  $link_options['query']['product_title'] = $title;
  $link_options['query']['date_auto'] = 'custom';
  if (!empty($date['date_from'])) {
    $link_options['query']['date_from'] = $date['date_from'];
  }
  if (!empty($date['date_to'])) {
    $link_options['query']['date_to'] = $date['date_to'];
  }
  $link_options['attributes']['title'] = 'View this product\'s history';

  $path = 'av/reports/stock-history-detailed';
  return l($link_title, $path, $link_options);
}

function avreports_account_type_balance_rows2(&$table_rows, $pl_accounts_by_type, $account_type, $fs_rows, $date_group, &$column_totals, $spacer_at_bottom = TRUE, $user_filter = array()) {
  $data = array();

  // Account Type info.
  $actd = avtxns_account_chart_types($account_type);

  // Account Type header.
  $data['account'] = array('data' => '<div class="uk-text-uppercase uk-text-bold">' . $actd['name'] . '</div>', 'colspan' => 1000);
  $table_rows[] = array('data' => $data);

  // Get account rows for this account type.
  $pl_account_rows = empty($pl_accounts_by_type[$account_type]) ? array() : $pl_accounts_by_type[$account_type];

  if ($account_type == AVTXN_AC_TYPE_CAPITAL) {
    // Insert Owners Equity in Capital.
    $pl_account_rows[AVTXN_AC_SPECIFIC_EQUITY_ID] = array(
      'id' => AVTXN_AC_SPECIFIC_EQUITY_ID,
      'title' => "Owner's Equity",
      'account_type' => AVTXN_AC_TYPE_CAPITAL,
      'weight' => 999,
      'status' => 1,
    );
    $total_assets_id = AVTXN_AC_TYPE_CURRENT . '.' . AVTXN_AC_TYPE_FIXED;
    foreach ($date_group as $date_from => $dg) {
      $date_to = $dg['date_to'];
      $assets_total = isset($column_totals[$total_assets_id][$date_to]) ? $column_totals[$total_assets_id][$date_to] : 0;
      $liab_total = isset($column_totals[AVTXN_AC_TYPE_LIABILITY][$date_to]) ? $column_totals[AVTXN_AC_TYPE_LIABILITY][$date_to] : 0;
      $this_cell_total = $assets_total - $liab_total;
      $fs_rows[AVTXN_AC_SPECIFIC_EQUITY_ID][$date_to] = $this_cell_total;
    }
  }

  $column_totals[$account_type] = array();
  $account_type_table_rows = avreports_account_type_balance_row_children2($pl_account_rows, $fs_rows, $date_group, $column_totals[$account_type], 0, $user_filter);

  //avbase_account_chart_sum_parent_balance($pl_account_rows);
  $table_rows = array_merge($table_rows, $account_type_table_rows);

  // Total.
  $data = array();
  $data['account'] = '<div class="uk-text-bold">Total ' . $actd['name'] . '</div>';
  foreach ($date_group as $dg_id => $dg) {
    $date_to = $dg['date_to'];
    $column_total = empty($column_totals[$account_type][$date_to]) ? 0 : $column_totals[$account_type][$date_to];
    $this_total_text = avbase_accounting_number_format($column_total);
    if ($dg_id != 'base') {
      $data[$date_to] = '<div class="uk-text-bold uk-text-right">' . $this_total_text . '</div>';
    }
  }
  // $at_total = isset($column_totals[$account_type]['at_total']) ? $column_totals[$account_type]['at_total'] : 0;
  // $at_total_text = avbase_accounting_number_format($at_total);
  // $data['total'] = '<div class="uk-text-bold uk-text-right">' . $at_total_text . '</div>';
  $table_rows[] = $data;

  // Spacer.
  if ($spacer_at_bottom) {
    $data = array();
    $data['account'] = array('data' => '&nbsp;', 'colspan' => 1000);
    $table_rows[] = $data;
  }
}

function avreports_account_type_balance_row_children2(&$account_rows, $fs_rows, $date_group, &$column_totals, $depth = 0, $user_filter = array()) {
  $table_rows = array();
  foreach ($account_rows as $account_id => $account_row) {
    $depth_html = '';
    for ($i = 0; $i < $depth; $i++) {
      $depth_html .= '<span class="uk-margin-left"></span>';
    }

    // Do not link to history if account type is a match.
    // $link_to_history = !in_array($account_row['account_type'], array(AVTXN_AC_TYPE_AR, AVTXN_AC_TYPE_AP));
    // $link_to_history = $link_to_history && !in_array($account_row['id'], array(AVTXN_AC_SPECIFIC_CASH_ID, AVTXN_AC_SPECIFIC_INVENTORY_ID, AVTXN_AC_SPECIFIC_EQUITY_ID));

    $data = array();
    // $data['account'] = $depth_html . ($link_to_history ? avreports_account_history_link($account_row['title'], $user_filter) : $account_row['title']);
    // $data['account'] = $depth_html . avreports_account_history_link($account_row['title'], $user_filter, NULL, FALSE);
    $data['account'] = $depth_html . check_plain($account_row['title']);
    // $data['account'] .= $account_row['id'];
    $this_row_amount = 0;
    $prev_amount = 0;
    foreach ($date_group as $dg_id => $dg) {
      $date_to = $dg['date_to'];
      $ending_amount = empty($fs_rows[$account_id][$date_to]) ? 0 : $fs_rows[$account_id][$date_to];
      // If base is set, presume that period amount should be used instead of ending amount.
      $this_cell_amount = isset($date_group['base']) ? ($ending_amount - $prev_amount) : $ending_amount;
      // If in total column, show row total.
      $amount_to_show = $dg_id == 'total' ? $this_row_amount : $this_cell_amount;
      // Only show if not in base column, we only use base for correct difference computation if needed.
      // Also, base amount shouldn't be added in total row amount.
      if ($dg_id != 'base') {
        $history_link_date = array('date_from' => format_date($dg['date_from'], 'custom', AVREPORTS_LIST_DATE_FORMAT), 'date_to' => format_date($dg['date_to'], 'custom', AVREPORTS_LIST_DATE_FORMAT));
        $data[$date_to] = '<div class="uk-text-right">' . avreports_account_history_link($account_row['title'], $history_link_date, avbase_accounting_number_format($amount_to_show), TRUE, $account_row['id']) . '</div>';
        $this_row_amount += $this_cell_amount;
      }
      $column_totals[$date_to] = empty($column_totals[$date_to]) ? $amount_to_show : ($column_totals[$date_to] + $amount_to_show);
      $prev_amount = $ending_amount;
    }
    // $total_column_amount_text = avbase_accounting_number_format($total_column_amount);
    // $data['total'] = '<div class="uk-text-right">' . $total_column_amount_text . '</div>';
    // $account_rows[$account_id]['account_balance'] = $total_column_amount;
    // $column_totals['at_total'] = isset($column_totals['at_total']) ? $column_totals['at_total'] + $total_column_amount : $total_column_amount;


    $table_rows[] = $data;
    //$account_type_total += $account_row['account_balance'];

    if (!empty($account_row['children'])) {
      $table_rows = array_merge($table_rows, avreports_account_type_balance_row_children2($account_rows[$account_id]['children'], $fs_rows, $date_group, $column_totals, $depth + 1, $user_filter));
    }
  }

  return $table_rows;
}

function avreports_account_type_difference_row2(&$table_rows, $account_label, &$column_totals, $type1, $type2, $date_group, $is_diff = TRUE) {
  $this_total = 0;
  $type1_total = isset($column_totals[$type1]['at_total']) ? $column_totals[$type1]['at_total'] : 0;
  $type2_total = isset($column_totals[$type2]['at_total']) ? $column_totals[$type2]['at_total'] : 0;
  if ($is_diff) {
    $this_total = $type1_total - $type2_total;
  }
  else {
    $this_total = $type1_total + $type2_total;
  }
  $totals_id = $type1 . '.' . $type2;


  $data = array();
  $data['account'] = '<div class="uk-text-bold">' . $account_label . '</div>';
  foreach ($date_group as $dg_id => $dg) {
    $date_to = $dg['date_to'];
    $period_total1 = isset($column_totals[$type1][$date_to]) ? $column_totals[$type1][$date_to] : 0;
    $period_total2 = isset($column_totals[$type2][$date_to]) ? $column_totals[$type2][$date_to] : 0;
    if ($is_diff) {
      $period_total = $period_total1 - $period_total2;
    }
    else {
      $period_total = $period_total1 + $period_total2;
    }
    $period_total_text = avbase_accounting_number_format($period_total);
    $column_totals[$totals_id][$date_to] = $period_total;
    if ($dg_id != 'base') {
      $data[$date_to] = '<div class="uk-text-bold uk-text-right">' . $period_total_text . '</div>';
    }
  }
  // $this_total_text = avbase_accounting_number_format($this_total);
  // $data['total'] = '<div class="uk-text-bold uk-text-right">' . $this_total_text . '</div>';
  $table_rows[] = array('data' => $data, 'class' => array('uk-bg-gray'));
  // Spacer.
  $data = array();
  $data['account'] = array('data' => '&nbsp;', 'colspan' => 1000);
  $table_rows[] = $data;

  $column_totals[$totals_id]['at_total'] = $this_total;
}
