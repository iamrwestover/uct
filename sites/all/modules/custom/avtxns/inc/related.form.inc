<?php

function avtxns_related_form($form, &$form_state, $transaction_type, $row) {
  if (empty($row->id) || $row->transaction_type != $transaction_type) {
    drupal_not_found();
    drupal_exit();
  }

  drupal_set_title('Transactions related to ' . strtoupper($row->transaction_type) . '# ' . $row->id);

  // Get transaction type details.
  $ttd = avtxns_txn_types($transaction_type);

  if (!empty($ttd['ctd'])) {
    $form['client_name'] = array(
      '#markup' => empty($row->client_name) ? '' : '<div class="uk-text-bold">' . $row->client_name . '</div>',
      '#title' => $ttd['ctd']['name'],
    );
    $form['client_address_string'] = array(
      '#markup' => empty($row->address_string) ? '' : nl2br($row->address_string),
    );
    if (!empty($row->contact_number)) {
      $form['client_address_string']['#markup'] .= '<br />' . check_plain($row->contact_number);
    }
  }

  $references = avtxns_get_txn_references(array($row->id));
  $table_rows = array();
  $refs = empty($references[$row->id]['refs']) ? array() : $references[$row->id]['refs'];
  $credit_txn_types = avtxns_get_creditable_txn_types();
  foreach ($refs as $k => $v) {
    $ttd = avtxns_txn_types($v->transaction_type);
    $end_path = '/view';
    if (!empty($ttd['payable'])) {
      $end_path = '/apply-credits';
    }
    $link_path = $ttd['base_path'] . '/' . $v->transaction_id . $end_path;
    $table_rows[$v->transaction_id] = array(
      'amount' => array('data' => '', 'class' => array('uk-text-right')),
      'txn' => array(
        'data' => l(strtoupper($v->transaction_type) . '# ' . $v->transaction_id, $link_path),
      ),
      'type' => l($ttd['name'], $link_path),
      'date' => format_date($v->transaction_date, 'custom', 'F d, Y'),
    );
    if (in_array($v->transaction_type, $credit_txn_types) || in_array($row->transaction_type, $credit_txn_types)) {
      $table_rows[$v->transaction_id]['amount']['data'] = l(number_format($v->cost, 2), $link_path);
    }
  }

  $header = array(
    'amount' => array('data' => 'Amount', 'class' => array('uk-text-right')),
    'txn' => array('data' => 'Transaction', 'class' => ''),
    'type' => array('data' => 'Type', 'class' => ''),
    'date' => array('data' => 'Date', 'class' => ''),
  );
  $form['related_txns'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $table_rows,
    '#empty' => '<div class="uk-text-muted">' . t('No related transactions found.') . '</div>',
    '#attributes' => array('class' => array('uk-table-condensed')),
  );
  return $form;
}
