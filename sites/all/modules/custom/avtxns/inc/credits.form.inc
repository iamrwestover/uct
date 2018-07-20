<?php

function avtxns_txn_credits_form($form, &$form_state, $transaction_type, $row) {
  if (empty($row->id) || $row->transaction_type != $transaction_type) {
    drupal_not_found();
    drupal_exit();
  }

  if (in_array($row->status, array(AVTXNS_TXN_STATUS_VOID, AVTXNS_TXN_STATUS_PENDING))) {
    drupal_set_message('Credits cannot be applied on this transaction.', 'warning');
    return array();
  }

  // Get transaction type details.
  $ttd = avtxns_txn_types($transaction_type);
  drupal_set_title($ttd['name'] . ' Credits');

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

  // Attachments.
  $form['#attached']['js'][] = AVTXNS_MODULE_PATH . '/js/credits.form.js';
  $form['#validate'][] = 'avtxns_txn_credits_form_validate';

  // Last changed validation.
  $form['id'] = array('#type' => 'value', '#value' => $row->id);
  $form['table_name'] = array('#type' => 'value', '#value' => 'avtbl_transactions');
  $form['last_changed'] = array('#type' => 'hidden', '#default_value' => (isset($row->changed) ? $row->changed : NULL));
  $form['#validate'][] = 'avbase_crud_form_validate';

  // Set autocomplete off.
  $form['#attributes']['autocomplete'] = 'off';

  // Remember values.
  $form['client_id'] = array('#type' => 'value', '#value' => $row->client_id);
  $form['transaction_type'] = array('#type' => 'value', '#value' => $transaction_type);

  $saved_credit_rows = array();
  $row_credit_items = empty($row->credit_items) ? array() : $row->credit_items;
  $existing_credit_items = array();
  foreach ($row_credit_items as $rci) {
    $saved_credit_rows[$rci->ref_txn_id] = $rci;
    $existing_credit_items[$rci->ref_txn_id] = $rci->ref_txn_id;
  }

  $form['existing_credit_items'] = array('#type' => 'value', '#value' => $existing_credit_items);
  $credit_rows = avtxns_get_client_credits($row->client_id, $transaction_type, $existing_credit_items);

  $table_rows = array();
  $form['credit_rows'] = array('#tree' => TRUE);
  foreach ($credit_rows as $cr_k => $credit_row) {
    $available_credit = round((float)$credit_row->available_credit, 2);
    if (empty($available_credit) && !isset($saved_credit_rows[$credit_row->id])) {
      continue;
    }

    $amount_used = empty($saved_credit_rows[$credit_row->id]->cost) ? 0 : $saved_credit_rows[$credit_row->id]->cost;

    $crtd = avtxns_txn_types($credit_row->transaction_type);
    $table_cols = array();
    $table_cols['transaction_id'] = array(
      '#type' => 'item',
      '#markup' => l(strtoupper($credit_row->transaction_type) . '# ' . $credit_row->id, $crtd['base_path'] . '/' . $credit_row->id . '/view'),
    );
    $table_cols['transaction_date'] = array(
      '#type' => 'item',
      '#markup' => format_date($credit_row->transaction_date, 'custom', AVBASE_DATE_FORMAT),
    );
    $table_cols['transaction_type_text'] = array(
      '#type' => 'item',
      '#markup' => !empty($crtd['payment']) ? 'Excess payment' : $crtd['name'],
    );
    $table_cols['original_amount'] = array(
      '#type' => 'item',
      '#markup' => number_format($credit_row->amount_to_credit, 2),
      '#column_attributes' => array('class' => array('uk-text-right')),
    );
    $table_cols['open_balance'] = array(
      '#type' => 'item',
      '#markup' => number_format($available_credit, 2),
      '#column_attributes' => array('class' => array('uk-text-right')),
    );
    $table_cols['tick_box'] = array(
      '#type' => 'checkbox',
      '#button_checkbox' => TRUE,
      '#default_value' => !empty($amount_used),
      '#attributes' => array(
        'class' => array('tick-box'),
        'data-open-balance' => $available_credit + $amount_used,
      ),
      '#column_attributes' => array('class' => array('uk-text-right')),
    );
    $table_cols['amount_to_use'] = array(
      '#type' => 'textfield',
      '#title' => 'Amount used',
      '#title_display' => 'none',
      '#maxlength' => 19,
      '#element_validate' => array('element_validate_number', 'avbase_element_validate_money_length', 'avbase_element_validate_non_negative_number'),
      '#special_element_type' => 'number',
      '#attributes' => array(
        'data-column-name' => 'amt-to-use',
        'class' => array('amt-to-use uk-text-right'),
        'step' => 0.01,
      ),
      '#default_value' => empty($amount_used) ? '' : avbase_number_format($amount_used, 2),
    );
    $table_cols['credit_type'] = array(
      '#type' => 'hidden',
      '#default_value' => $credit_row->transaction_type,
      '#column_attributes' => array('class' => array('uk-hidden')),
    );
    $table_cols['amount_used'] = array(
      '#type' => 'hidden',
      '#default_value' => empty($amount_used) ? '' : $amount_used,
      '#column_attributes' => array('class' => array('uk-hidden')),
    );

    $table_rows[$cr_k] = $table_cols;
    $table_rows[$cr_k]['#row_attributes'] = array('class' => array('uk-table-middle av-credit-row'));
  }

  if (!empty($table_rows)) {
    $table = array(
      '#theme' => 'avbase_form_table',
      '#header' => array(
        'transaction_id' => array('data' => '#', 'class' => 'suk-text-right'),
        'transaction_date' => array('data' => t('Date')),
        'transaction_type_text' => array('data' => t('Type')),
        'original_amount' => array('data' => t('Original Amount'), 'class' => 'uk-text-right'),
        'open_balance' => array('data' => t('Remaining Balance'), 'class' => 'uk-text-right'),
        'tick_box' => '',
        'amount_to_use' => array('data' => t('Amount used'), 'class' => 'uk-text-right'),
      ),
    );
    $table['rows'] = $table_rows;
    $table['#attributes'] = array('class' => array('uk-table-striped', 'uk-table-condensed'));
    $form['credit_rows'] += $table;

    $form['total_credits_used'] = array(
      '#markup' => '<div class="uk-text-right uk-text-large"><span class="uk-margin-large-right">Total credits used</span>
        <span id="total-credits-used">' . number_format(0, 2) .'</span></div>',
    );
    $form['bill_total'] = array(
      '#type' => 'hidden',
      '#default_value' => $row->debit_total - $row->total_paid,
    );
    $form['bill_total_text'] = array(
      '#markup' => '<div class="uk-text-right uk-text-large"><span class="uk-margin-large-right">' . $ttd['name'] . ' balance</span>
        <span id="bill-total-text">' . number_format($row->debit_total - $row->total_paid, 2) .'</span></div>',
    );

    // Buttons
    $form['buttons'] = array('#theme' => 'avbase_crud_button_group');
    $form['buttons']['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Save'),
      '#attributes' => array(
        'class' => array('uk-button-primary'),
        'data-ks' => 'CTRL+S',
      ),
    );
    $form['buttons']['cancel'] = array(
      '#markup' => l('Cancel', $ttd['base_path'] . '/' . $row->id . '/view', array('attributes' => array('class' => array('uk-button')))),
    );
  }
  else {
    $form['no_credits'] = array('#markup' => '<div class="uk-text-muted uk-text-center">No credits available.</div>');
  }

  return $form;
}

function avtxns_txn_credits_form_submit($form, &$form_state) {
  $transaction = db_transaction();
  try {
    $entered_rows = empty($form_state['values']['credit_rows']['rows']) ? array() : $form_state['values']['credit_rows']['rows'];
    $transaction_id = empty($form_state['values']['id']) ? NULL : $form_state['values']['id'];
    $transaction_type = empty($form_state['values']['transaction_type']) ? NULL : $form_state['values']['transaction_type'];
    $ttd = avtxns_txn_types($transaction_type);
    // Delete existing credits.
    $query = "DELETE FROM {avtbl_transaction_details} WHERE transaction_id = :txn_id AND ref_txn_type IN (:ref_types)";
    db_query($query, array(':txn_id' => $transaction_id, ':ref_types' => $ttd['credit_txn_types']));

    // Insert new credits.
    foreach ($entered_rows as $ek => $er) {
      $amount_to_use = (float)$er['amount_to_use'];
      if (empty($amount_to_use)) {
        continue;
      }
      $row = array(
        'transaction_id' => $transaction_id,
        'qty' => 1,
        'qty_per_uom' => 1,
        'cost' => $amount_to_use,
        'ref_txn_id' => $ek,
        'ref_txn_type' => $er['credit_type'],
      );
      db_insert('avtbl_transaction_details')->fields($row)->execute();
    }

    // Close transaction ids.
    $reconsider_ids = array_keys($entered_rows);
    $reconsider_ids[] = $transaction_id;
    avtxns_txn_reconsider_status($reconsider_ids);
    //if (!empty(array_keys($entered_rows))) {
    //
    //  if (db_query("UPDATE {avtbl_transactions} SET changed = :changed, status = 0 WHERE id IN (:ids)", array(':ids' => array_keys($entered_rows), ':changed' => time()))) {
    //    avbase_custom_watchdog("{$transaction_type}__close", "ID: multiple", array('#ids' => array_keys($entered_rows), '#manner' => 'credits-auto'));
    //  }
    //}

    drupal_set_message('Credit changes saved.');
    avbase_custom_watchdog("credits_saved", "ID: {$transaction_id}", array('#row' => $entered_rows));
  }
  catch (Exception $e) {
    $transaction->rollback();
    watchdog_exception('credits_save', $e);
    throw $e;
  }
}

function avtxns_txn_credits_form_validate($form, &$form_state) {
  $client_id = empty($form_state['values']['client_id']) ? NULL : $form_state['values']['client_id'];
  $transaction_id = empty($form_state['values']['id']) ? NULL : $form_state['values']['id'];
  if (empty($client_id)) {
    return;
  }
  $entered_rows = empty($form_state['values']['credit_rows']['rows']) ? array() : $form_state['values']['credit_rows']['rows'];
  if (empty($entered_rows)) {
    form_set_error('', 'No credits selected.');
    return;
  }
  $transaction_type = empty($form_state['values']['transaction_type']) ? NULL : $form_state['values']['transaction_type'];
  $ttd = avtxns_txn_types($transaction_type);
  $credit_rows = avtxns_get_client_credits($client_id, $transaction_type, $form_state['values']['existing_credit_items']);
  $total_credits_used = 0;
  foreach ($entered_rows as $ek => $er) {
    if (!isset($credit_rows[$ek])) {
      form_set_error("credit_rows][rows][{$ek}][amount_to_use", 'Credit is not valid anymore.');
    }
    $amount_to_use = (float)$er['amount_to_use'];
    $amount_used = (float)$er['amount_used'];
    $_amount_used = $amount_to_use - $amount_used;
    if (round($_amount_used, 2) > round($credit_rows[$ek]->available_credit, 2)) {
      form_set_error("credit_rows][rows][{$ek}][amount_to_use", 'Amount used is greater than available balance.');
    }
    $total_credits_used += $amount_to_use;
  }


  $transaction_row = avtxns_txn_load($transaction_id);
  $transaction_row_total = empty($transaction_row->debit_total) ? 0 : $transaction_row->debit_total;
  $transaction_row_total -= $transaction_row->total_paid;
  $remaining_total = $transaction_row_total - $total_credits_used;
  if (round((float)$remaining_total, 2) < 0) {
    form_set_error("", t('@tname balance cannot be less than 0.', array('@tname' => $ttd['name'])));
  }
}

function avtxns_get_client_credits($client_id, $transaction_type, $existing_credit_items = array()) {
  $data = &drupal_static(__FUNCTION__, array());
  $data_id = "$client_id:$transaction_type";
  if (isset($data[$data_id])) {
    return $data[$data_id];
  }

  $ttd = avtxns_txn_types($transaction_type);
  $credit_rows = array();
  $query = "SELECT avtxns.* FROM {avtbl_transactions} avtxns
    WHERE avtxns.client_id = :client_id AND avtxns.status IN (:statuses) AND avtxns.transaction_type IN (:ttypes)
    AND avtxns.amount_to_credit > 0";
  $args = array(
    ':client_id' => $client_id,
    ':statuses' => array(AVTXNS_TXN_STATUS_OPEN, AVTXNS_TXN_STATUS_LOCKED),
    ':ttypes' => $ttd['credit_txn_types']
  );
  if (!empty($existing_credit_items)) {
    $query .= " OR (avtxns.status = :closed AND avtxns.id IN (:existing_credit_ids))";
    $args[':closed'] = AVTXNS_TXN_STATUS_CLOSED;
    $args[':existing_credit_ids'] = array_keys($existing_credit_items);
  }
  $rs = db_query($query, $args);
  $txn_ids = array();
  foreach ($rs as $row) {
    $txn_ids[$row->id] = $row->id;
    $row->available_credit = (float) $row->amount_to_credit;
    $credit_rows[$row->id] = $row;
  }

  if (!empty($txn_ids)) {
    $credits_used = avtxns_get_txn_references($txn_ids);
    foreach ($credits_used as $cu_k => $cu) {
      $credit_rows[$cu_k]->available_credit -= (float)$cu['simple_costs_total'];
    }
    //$query = "SELECT avtxn_dtls.transaction_id, avtxn_dtls.ref_txn_id, avtxn_dtls.cost FROM {avtbl_transaction_details} avtxn_dtls
    //    INNER JOIN {avtbl_transactions} avtxns ON avtxns.id = avtxn_dtls.transaction_id
    //    WHERE avtxn_dtls.ref_txn_id IN (:ids) AND avtxns.status IN (:statuses)";
    //$rs = db_query($query, array(':ids' => $txn_ids, ':statuses' => array(AVTXNS_TXN_STATUS_OPEN, AVTXNS_TXN_STATUS_LOCKED)));
    //foreach ($rs as $row) {
    //  $credit_rows[$row->ref_txn_id]->available_credit -= (float)$row->cost;
    //  //if ($row->transaction_id == $transaction_id) {
    //  //  $credit_rows[$row->ref_txn_id]->amount_used = (float) $row->paid_amount;
    //  //}
    //}
  }

  $data[$data_id] = $credit_rows;
  return $data[$data_id];
}