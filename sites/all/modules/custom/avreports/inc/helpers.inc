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
    array('avtxns.status', array(AVTXNS_TXN_STATUS_CLOSED, AVTXNS_TXN_STATUS_OPEN), 'IN'),
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

  if (isset($client->opening_balance)) {
    $opening_balance = isset($client->opening_balance) ? (float) $client->opening_balance : 0;
    if (!empty($opening_balance)) {
      $invoices['opening_balance'] = (object) array('balance' => $opening_balance, 'transaction_date' => $client->created);
    }
  }
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

function avreports_account_type_balance_rows(&$table_rows, $pl_accounts_by_type, $account_type, $arnp, $date_group, &$at_totals, $spacer_at_bottom = TRUE) {
  // Account Type info.
  $actd = avtxns_account_chart_types($account_type);

  // Header.
  $data = array();
  $data['account'] = array('data' => $actd['name'], 'colspan' => 1000, 'class' => array('uk-text-uppercase uk-text-bold'));
  $table_rows[] = array('data' => $data);

  // Details.
  $pl_account_rows = empty($pl_accounts_by_type[$account_type]) ? array() : $pl_accounts_by_type[$account_type];
  $at_totals[$account_type] = array();
  $account_type_table_rows = avreports_account_type_balance_row_children($pl_account_rows, $arnp, $date_group, $at_totals[$account_type]);
  //avbase_account_chart_sum_parent_balance($pl_account_rows);
  $table_rows = array_merge($table_rows, $account_type_table_rows);

  // Total.
  $data = array();
  $data['account'] = array('data' => 'Net ' . $actd['name'], 'class' => array('uk-text-bold'));
  foreach ($date_group as $tk => $tc) {
    $period_total = isset($at_totals[$account_type][$tk]) ? $at_totals[$account_type][$tk] : 0;
    $this_total_text = avbase_accounting_number_format($period_total);
    $data[$tk] = array('data' => $this_total_text, 'class' => array('uk-text-bold uk-text-right'));
  }
  $at_total = isset($at_totals[$account_type]['at_total']) ? $at_totals[$account_type]['at_total'] : 0;
  $at_total_text = avbase_accounting_number_format($at_total);
  $data['total'] = array('data' => $at_total_text, 'class' => array('uk-text-right uk-text-bold'));
  $table_rows[] = array('data' => $data);

  // Spacer.
  if ($spacer_at_bottom) {
    $data = array();
    $data['account'] = array('data' => '&nbsp;', 'colspan' => 1000);
    $table_rows[] = array('data' => $data);
  }
}

function avreports_account_type_balance_row_children(&$account_rows, $arnp, $date_group, &$account_totals, $depth = 0) {
  $table_rows = array();

  foreach ($account_rows as $r_k => $account_row) {
    $depth_html = '';
    for ($i = 0; $i < $depth; $i++) {
      $depth_html .= '<span class="uk-margin-left"></span>';
    }

    $data = array();
    $data['account'] = array('data' => $depth_html . check_plain($account_row['title']), 'class' => array(''));
    $this_account_balance = 0;
    foreach ($date_group as $tk => $tc) {
      $this_period_balance = isset($arnp[$tk][$r_k]['account_balance']) ? $arnp[$tk][$r_k]['account_balance'] : 0;
      $this_account_balance += $this_period_balance;
      $account_totals[$tk] = isset($account_totals[$tk]) ? $account_totals[$tk] + $this_period_balance : $this_period_balance;
      $data[$tk] = array('data' => avbase_accounting_number_format($this_period_balance), 'class' => array('uk-text-right'));
    }
    $this_account_balance_text = avbase_accounting_number_format($this_account_balance);
    $account_rows[$r_k]['account_balance'] = $this_account_balance;
    $account_totals['at_total'] = isset($account_totals['at_total']) ? $account_totals['at_total'] + $this_account_balance : $this_account_balance;

    $data['total'] = array('data' => $this_account_balance_text, 'class' => array('uk-text-right'));
    $table_rows[] = array('data' => $data);
    //$account_type_total += $account_row['account_balance'];

    if (!empty($account_row['children'])) {
      $table_rows = array_merge($table_rows, avreports_account_type_balance_row_children($account_rows[$r_k]['children'], $arnp, $date_group, $account_totals, $depth + 1));
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
  $data['account'] = array('data' => $account_label, 'class' => array('uk-text-bold'));
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
    $data[$tk] = array('data' => $period_total_text, 'class' => array('uk-text-bold uk-text-right'));

    $at_totals[$totals_id][$tk] = $period_total;
  }
  $this_total_text = avbase_accounting_number_format($this_total);
  $data['total'] = array('data' => $this_total_text, 'class' => array('uk-text-right uk-text-bold'));
  $table_rows[] = array('data' => $data);
  // Spacer.
  $data = array();
  $data['account'] = array('data' => '&nbsp;', 'colspan' => 1000);
  $table_rows[] = array('data' => $data);

  $at_totals[$totals_id]['at_total'] = $this_total;
}