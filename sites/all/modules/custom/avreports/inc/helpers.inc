<?php

/**
 * Return form array containing client's balance ages.
 * @param $client_id
 * @return array|NULL
 */
function avreports_client_balance_aging_form($client_id) {
  $client = avclient_load($client_id);
  if (empty($client->id)) {
    return NULL;
  }

  $invoice_where = array(
    array('avclient.id', $client->id),
    array('avtxns.transaction_type', AVTXNS_TXN_TYPE_INVOICE),
    array('avtxns.status', 1),
  );
  $invoices = avtxns_txn_query_load($invoice_where);
  $now = time() /*+ (86400*44)*/;
  $balance_ages = array();
  $term_days = empty($client->term_data['days']) ? 30 : $client->term_data['days'];
  if (!empty($term_days)) {
    for ($x = 1; $x <= 12; $x++) {
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
    $balance_age = ceil(($now - $invoice->transaction_date) / 86400);
    $days = ceil($balance_age / $term_days) * $term_days;
    if (isset($balance_ages[$days])) {
      $balance_ages[$days] += $invoice->balance;
    }
  }

  $balance_ages_form = array('#theme' => 'avreports_balance_ages');
  $ages_count = 0;
  foreach ($balance_ages as $k => $balance) {
    $balance_ages_form[$k] = array(
      '#type' => 'item',
      '#title' => t('@from to @to days', array('@from' => $k - $term_days + 1, '@to' => $k)),
      '#markup' => number_format($balance, 2),
    );

    $ages_count++;
    if ($ages_count == count($balance_ages)) {
      $balance_ages_form[$k]['#title'] = t('@from++ days', array('@from' => $k - $term_days));
    }
  }
  return $balance_ages_form;
}