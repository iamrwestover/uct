<?php

/**
 * General validation for item list form fields.
 * @param $form
 * @param $form_state
 */
function avtxns_item_list_fields_validate(&$form, &$form_state) {
  $transaction_type = empty($form_state['values']['transaction_type']) ? '' : $form_state['values']['transaction_type'];
  $ttd = avtxns_txn_types($transaction_type);

  $row_id = empty($form_state['values']['id']) ? NULL : $form_state['values']['id'];

  // If customer is suspended, prevent further deliveries.
  $client_id = empty($form_state['values']['client_id']) ? NULL : $form_state['values']['client_id'];
  if (empty($row_id) && !empty($client_id) && in_array($transaction_type, array(AVTXNS_TXN_TYPE_DELIVERY))) {
    $client_status = db_query("SELECT status FROM {avtbl_clients} WHERE id = :id", array(':id' => $client_id))->fetchField();
    if ($client_status != AVCLIENT_STATUS_ACTIVE) {
      $csd = avclients_client_status_details($client_status);
      form_set_error('client_name', t('@client_type is @client_status.', array('@client_type' => $ttd['ctd']['name'], '@client_status' => $csd['title'])));
    }
  }



  $form_state_item_rows = empty($form_state['values']['product_rows']) ? array() : $form_state['values']['product_rows'];

  // Get form storage.
  $form_state['storage'] = isset($form_state['storage']) ? $form_state['storage'] : array();
  $_storage = &$form_state['storage'];

  // Get item rows.
  $_storage['item_rows'] = isset($_storage['item_rows']) ? $_storage['item_rows'] : array();
  $_item_rows = &$_storage['item_rows'];


  // Validate total.
  $total = 0;
  foreach ($form_state_item_rows as $fsir_k => $fsir) {
    if (isset($fsir['total'])) {
      $fsir_total = (float)$fsir['total'];
      $total += $fsir_total;
      if (isset($fsir['balance'])) {
        $fsir_balance = (float)$fsir['balance'];
        if ($fsir_total > $fsir_balance) {
          form_set_error("product_rows][{$fsir_k}][total", 'Item total cannot be more than remaining balance.');
        }
      }
    }
  }

  if (!empty($ttd['total_is_amount_to_credit'])) {
    $form_state['values']['amount_to_credit'] = $total;
    //$form_state['values']['available_credit'] = $total;
  }
  if (!empty($ttd['payment']) && isset($form_state['values']['amount_received'])) {
    $amount_to_credit = (float) $form_state['values']['amount_received'] - $total;
    if ($amount_to_credit < 0) {
      form_set_error('amount_received', 'Amount received cannot be less than this transaction\'s total amount.');
    }
    else {
      $form_state['values']['amount_to_credit'] = $amount_to_credit;
      //$form_state['values']['available_credit'] = $amount_to_credit;
    }
  }

  // Make sure at least one product is added.
  if (empty($ttd['non_product']) && empty($ttd['payment'])) {
    $detected_product_ids = array();
    $product_ids = array();
    foreach ($form_state_item_rows as $k => $row) {
      if (!empty($row['product_id'])) {
        $detected_product_ids[$k] = $row['product_id'];
        $form['product_rows'][$k]['product_title']['#attributes']['data-selected-product-id'] = $row['product_id'];

        if (!empty($ttd['prevent_duplicate_product'])) {
          if (isset($product_ids[$row['product_id']])) {
            form_set_error("product_rows][{$k}][product_title", t('There are more than 1 entries of %product_name.', array('%product_name' => $row['product_title'])));
          } else {
            $product_ids[$row['product_id']] = $row['product_id'];
          }
        }
      }
    }

    // No valid product detected.
    if (empty($detected_product_ids)) {
      form_set_error('', t('Please add at least one valid item.'));
    }

    // Save product row details into drupal JS settings.
    if (!empty($detected_product_ids)) {
      $product_rows = avproduct_load_ids($detected_product_ids);
      drupal_add_js(array('avbase' => array('products' => $product_rows)), 'setting');
    }
  }

  // Check if at least 1 valid account is entered.
  if ($ttd['transaction'] == 'accounts') {
    $detected_account_ids = array();
    $detected_client_ids = array();
    foreach ($form_state_item_rows as $k => $row) {
      if (!empty($row['account_id'])) {
        $detected_account_ids[$k] = $row['account_id'];
        //$form['product_rows'][$k]['product_title']['#attributes']['data-selected-product-id'] = $row['product_id'];
      }
      if (!empty($row['client_id'])) {
        $detected_client_ids[$k] = $row['client_id'];
      }
    }


    // No valid account detected.
    if (empty($detected_account_ids)) {
      form_set_error('', t('Please add at least one valid account.'));
    }
    else {
      // Check to make sure only 1 AP/AR account is entered.
      $account_rows = avtxns_account_chart_load_ids($detected_account_ids);
      $ap_ar_count = 0;
      if (!empty($account_rows)) {
        foreach ($account_rows as $arow) {
          if (in_array($arow->account_type, array(AVTXN_AC_TYPE_AP, AVTXN_AC_TYPE_AR))) {
            $ap_ar_count++;
          }
        }
      }

      if ($ap_ar_count > 1) {
        form_set_error('', t('You cannot use more than one AR or AP account in the same transaction.'));
      }
      else {
        // Check to make sure AP/AR includes a vendor/customer ID.
        $client_rows = avclient_load_ids($detected_client_ids);
        foreach ($detected_account_ids as $ai_k => $ai) {
          $account_row = empty($account_rows[$ai]) ? NULL : $account_rows[$ai];
          $account_row_type = isset($account_row->account_type) ? $account_row->account_type : NULL;
          $account_row_client_id = empty($detected_client_ids[$ai_k]) ? NULL : $detected_client_ids[$ai_k];
          $account_row_client_row = empty($client_rows[$account_row_client_id]) ? NULL : $client_rows[$account_row_client_id];
          $account_row_client_type = isset($account_row_client_row->client_type) ? $account_row_client_row->client_type : NULL;
          $acd = avtxns_account_chart_types($account_row_type);
          if ($account_row_type == AVTXN_AC_TYPE_AP && $account_row_client_type != AVCLIENT_TYPE_VENDOR) {
            form_set_error('', t('@account_type_name must include a Vendor.', array('@account_type_name' => $acd['name'])));
          }
          if ($account_row_type == AVTXN_AC_TYPE_AR && $account_row_client_type != AVCLIENT_TYPE_CUSTOMER) {
            form_set_error('', t('@account_type_name must include a Customer.', array('@account_type_name' => $acd['name'])));
          }
        }
      }

      // Make sure journal entries amount is balanced.
      $debit_total = 0;
      $credit_total = 0;
      foreach ($form_state_item_rows as $fsr_k => $fsr) {
        $debit_value = empty($fsr['debit']) ? 0 : $fsr['debit'];
        $credit_value = empty($fsr['credit']) ? 0 : $fsr['credit'];
        $debit_total += (float) $debit_value;
        $credit_total += (float) $credit_value;
      }
      if ($debit_total != $credit_total) {
        form_set_error('', t('Transaction is not balanced. Please make sure that debit and credit totals are equal.'));
      }

      //form_set_error('', t('fffffffPlease add at least one valid account.'));
    }
  }


  // Merge inputted product rows into $_item_rows.
  $inputted_rows = isset($form_state['input']['product_rows']) ? $form_state['input']['product_rows'] : array();
  $merged = array();
  //$rearranged_input_rows = array();
  foreach ($inputted_rows as $k => $v) {
    //$_item_rows[$k] = isset($_item_rows[$k]) ? array_merge($_item_rows[$k], $v) : $v;
    $merged[$k] = isset($_item_rows[$k]) ? array_merge($_item_rows[$k], $v) : $v;
    //$rearranged_input_rows[$k] = $v;
  }
  $_item_rows = $merged;

  // Make sure all included reference transaction IDs are still open.
  if (empty($row_id)) {
    $ref_txn_ids = array();
    foreach ($_item_rows as $k => $v) {
      $ref_txn_id = empty($v['ref_txn_id']) ? NULL : $v['ref_txn_id'];
      if ($ref_txn_id) {
        $ref_txn_ids[$ref_txn_id] = $ref_txn_id;
      }
    }

    if (!empty($ref_txn_ids)) {
      $query = "SELECT id, transaction_type, status FROM {avtbl_transactions} WHERE id IN (:txn_ids)";
      $rs = db_query($query, array(':txn_ids' => $ref_txn_ids));
      $valid_statuses = array(AVTXNS_TXN_STATUS_OPEN);
      if (!empty($ttd['payment'])) {
        $valid_statuses[] = AVTXNS_TXN_STATUS_CLOSED;
      }
      foreach ($rs as $txn) {
        if (!in_array($txn->status, $valid_statuses)) {
          form_set_error('', t('Cannot proceed with items from @ttype# @txn_id. Transaction is not available anymore. Try removing these items before proceeding.', array('@ttype' => strtoupper($txn->transaction_type) , '@txn_id' => $txn->id)));
        }
      }
    }
  }

  if (!empty($ttd['payment']) && empty($_item_rows)) {
    form_set_error('', t('No payment received.'));
    return;
  }
}

/**
 * Only validate a product row element if a product is actually selected.
 * @param $element
 * @param $form_state
 * @param $form
 */
function avtxns_item_list_row_element_validate($element, &$form_state, $form) {
  //// Get form storage.
  //$form_state['storage'] = isset($form_state['storage']) ? $form_state['storage'] : array();
  //$_storage = &$form_state['storage'];
  //
  //// Get item rows.
  //$_storage['item_rows'] = isset($_storage['item_rows']) ? $_storage['item_rows'] : array();
  //$_item_rows = &$_storage['item_rows'];

  $form_state_item_rows = empty($form_state['values']['product_rows']) ? array() : $form_state['values']['product_rows'];
  $transaction_type = empty($form_state['values']['transaction_type']) ? '' : $form_state['values']['transaction_type'];
  $ttd = avtxns_txn_types($transaction_type);

  // If a valid product is detected for this element's row, do validate this element.
  $item_row_key = isset($element['#item_row_key']) ? $element['#item_row_key'] : '';
  if ($ttd['transaction'] == 'accounts') {
    $do_validate = !empty($form_state_item_rows[$item_row_key]['account_id']);
  }
  else {
    $do_validate = !empty($form_state_item_rows[$item_row_key]['product_id']);
  }
  if ($do_validate) {
    $element_value = $element['#value'];
    if (empty($element['#dont_require']) && trim($element_value) == '') {
      form_error($element, t('@field_title is required.', array('@field_title' => $element['#title'])));
    }

    if (!empty($element['#av_prod_element_validate'])) {
      foreach ($element['#av_prod_element_validate'] as $func) {
        $func($element, $form_state);
      }
    }
  }
}

/**
 * Check if reference id should be required.
 * @param $element
 * @param $form_state
 */
function avtxns_element_validate_reference_id_required($element, &$form_state) {
  $values = isset($form_state['values']) ? $form_state['values'] : array();
  $pmt_method_id = empty($values['pmt_method_id']) ? NULL : $values['pmt_method_id'];

  if ($pmt_method_id == AVBASE_PMT_METHOD_CHECK) {
    $element_value = $element['#value'];
    $element_value = trim($element_value);
    $element_title = $element['#title'];
    if ($element_value == '') {
      form_error($element, t('@element_title  is required for Check payments.', array('@element_title' => $element_title)));
    }
  }
}
