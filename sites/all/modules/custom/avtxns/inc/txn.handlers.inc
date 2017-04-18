<?php

/**
 * Perform actions depending on the kind of change that will be made
 * on the database for POs.
 * @param $op
 * @param $row
 * @param $form_state
 * @return object|NULL
 */
function avtxns_avbase_crud_dbchange_txn($op, $row, $form_state) {
  switch ($op) {
    case 'pre-save':
      //if (empty($row['id'])) {
      //  $row['transaction_date'] = time();
      //}
      return $row;
      break;

    case 'after-save':
      // Remove qty reservations.
      avtxns_remove_user_qty_reservation();

      $form_state_item_rows = empty($form_state['values']['product_rows']) ? array() : $form_state['values']['product_rows'];

      // Get item rows.
      $item_rows = isset($form_state['storage']['item_rows']) ? $form_state['storage']['item_rows'] : array();

      // Get transaction type.
      $transaction_type = empty($form_state['values']['transaction_type']) ? '' : $form_state['values']['transaction_type'];
      $ttd = avtxns_txn_types($transaction_type);

      // Get current transaction status.
      $transaction_status = isset($form_state['values']['transaction_status']) ? $form_state['values']['transaction_status'] : NULL;

      // Set table field names.
      $all_table_field_names = array(
        'transaction_id',
        'item_id',
        'uom_id',
        'qty_per_uom',
        'qty',
        'cost',
        'good_stock',
        'discount1',
        'discount2',
        'discount3',
        'discount4',
        //'paid_amount',
        'ref_txn_dtl_id',
        'ref_txn_id',
        'ref_txn_type',
        'account_id',
        'debit',
        'credit',
        'client_id',
        'description',
      );
      //sort($all_table_field_names);

      $close_transaction_ids = array();
      $insert_rows = array();
      $insert_args = array();
      $update_rows = array();
      $update_args = array();
      $transaction_detail_ids = array();
      //$matching_table_field_names = array();
      $item_ids = array();
      $discount_updates = array();
      $entered_discounts = array();
      foreach ($item_rows as $ir_k => $item_row) {
        // Skip if account_id is empty.
        if ($ttd['transaction'] == 'accounts' && empty($form_state_item_rows[$ir_k]['account_id'])) {
          continue;
        }

        // Skip if product id is empty.
        if (empty($ttd['non_product']) && empty($ttd['payment']) && empty($form_state_item_rows[$ir_k]['product_id'])) {
          continue;
        }

        // Skip if payment but 0 total.
        if (!empty($ttd['payment'])) {
          $this_row_total = isset($item_row['total']) ? (float)$item_row['total'] : 0;
          if ($this_row_total <= 0) {
            continue;
          }
        }

        // Make sure only item rows owned by this owner get saved.
        if (empty($ttd['non_product']) && !empty($item_row['client_id']) && $item_row['client_id'] != $row['client_id']) {
          continue;
        }

        // Override certain item row values for Stock Adjustment transactions.
        if ($transaction_type == AVTXNS_TXN_TYPE_STOCK_ADJUSTMENT) {
          $item_rows[$ir_k]['qty'] = $item_row['qty'] = $item_row['qty_diff'];
          $item_rows[$ir_k]['qty_per_uom'] = $item_row['qty_per_uom'] = 1;
        }


        // Set foreign key.
        $item_row['transaction_id'] = $row['id'];

        // Initialize variables.
        $transaction_detail_id = NULL;
        $insert_arr = $update_arr = array();
        $item_row['discount1'] = $item_row['discount2'] = $item_row['discount3'] = $item_row['discount4'] = 0;
        // Overwrite paid amount.
        if (!empty($ttd['total_is_entered_amount'])) {
          //$item_row['paid_amount'] = $item_row['total'];
          $item_rows[$ir_k]['qty'] = $item_row['qty'] = 1;
          $item_rows[$ir_k]['qty_per_uom'] = $item_row['qty_per_uom'] = 1;
          $item_rows[$ir_k]['cost'] = $item_row['cost'] = $item_row['total'];
        }

        if (isset($item_row['product_id'])) {
          $item_id = $item_row['item_id'] = $item_row['product_id'];
          $item_ids[] = $item_id;
          $discount_updates['discount1'][$item_id] = $discount_updates['discount2'][$item_id] = $discount_updates['discount3'][$item_id] = $discount_updates['discount4'][$item_id] = 0;

          // Explode discount text to discount array.
          $discount_text = isset($item_row['discount']) ? trim($item_row['discount']) : '';
          $discount_parts = explode('/', $discount_text);
          $x = 1;
          foreach ($discount_parts as $k => $v) {
            if (!is_numeric($v)) {
              continue;
            }
            $discount_field_name = 'discount' . $x++;
            $item_row[$discount_field_name] = (float) $v;
            $discount_updates[$discount_field_name][$item_id] = $item_row[$discount_field_name];

            $this_dc_val = (1 - ((float) $v / 100));
            $entered_discounts[$item_id][$ir_k] = isset($entered_discounts[$item_id][$ir_k]) ? ($entered_discounts[$item_id][$ir_k] * $this_dc_val) : $this_dc_val;
          }
        }

        // Remember transaction detail id to know which rows to insert/update.
        $transaction_detail_id = empty($item_row['id']) ? 0 : $item_row['id'];
        if (!empty($transaction_detail_id)) {
          $transaction_detail_ids[] = $transaction_detail_id;
        }

        // Build insert and update array.
        foreach ($all_table_field_names as $field_name) {
          $arg_id = ':arg_' . count($insert_args);
          $value = isset($item_row[$field_name]) ? $item_row[$field_name] : NULL;
          if (in_array($field_name, array('credit', 'debit'))) {
            $value = (float) $value;
          }

          if (empty($transaction_detail_id)) {
            $insert_arr[] = $arg_id;
            $insert_args[$arg_id] = $value;
          }
          elseif (!is_null($value)) {
            $update_rows[$field_name][$transaction_detail_id] = $value;
          }
        }
        if (!empty($insert_arr)) {
          $insert_rows[] = "(" . implode(',', $insert_arr) . ")";
        }

        // Remember which transaction IDs to close.
        if (!empty($item_row['ref_txn_id'])) {
          $close_transaction_ids[$item_row['ref_txn_id']] = $item_row['ref_txn_id'];
        }
      }

      // Append all ref_txn_ids from this transaction to list of transactions to reconsider status.
      if (empty($form_state['#new_record']) && !empty($ttd['pending_txns_reference_type'])) {
        $ref_query = "SELECT avtxn_details.ref_txn_id
        FROM {avtbl_transaction_details} AS avtxn_details
        WHERE avtxn_details.transaction_id = :id AND avtxn_details.ref_txn_id";
        $ref_args = array(':id' => $row['id']);
        if (!empty($ttd['credit_txn_types'])) {
          $ref_query .= " AND ref_txn_type NOT IN (:txn_types)";
          $ref_args[':txn_types'] = $ttd['credit_txn_types'];
        }
        $ref_rs = db_query($ref_query, $ref_args);
        foreach ($ref_rs as $ref_row) {
          $close_transaction_ids[$ref_row->ref_txn_id] = $ref_row->ref_txn_id;
        }
      }



      // Delete transaction detail ids that were removed.
      $to_be_deleted_rows = array();
      if (empty($form_state['#new_record'])) {
        $delete_query = "DELETE FROM {avtbl_transaction_details} WHERE transaction_id = :txn_id";
        $sd_query = "SELECT id, transaction_id, item_id, ref_txn_type, (qty * qty_per_uom) AS incoming_base_qty, good_stock
          FROM {avtbl_transaction_details} WHERE transaction_id = :txn_id";
        $delete_args = array(':txn_id' => $row['id']);
        if (!empty($transaction_detail_ids)) {
          $delete_query .= " AND id NOT IN (:txn_dtl_ids)";
          $sd_query .= " AND id NOT IN (:txn_dtl_ids)";
          $delete_args[':txn_dtl_ids'] = $transaction_detail_ids;
        }
        if (!empty($ttd['credit_txn_types'])) {
          $delete_query .= " AND (ref_txn_type NOT IN (:txn_types) OR ref_txn_type IS NULl)";
          $sd_query .= " AND (ref_txn_type NOT IN (:txn_types) OR ref_txn_type IS NULl)";
          $delete_args[':txn_types'] = $ttd['credit_txn_types'];
        }
        if (!empty($ttd['qty_change'])) {
          $sd_rs = db_query($sd_query, $delete_args);
          foreach ($sd_rs as $sd_row) {
            $to_be_deleted_rows['delete_' . avtxns_get_item_row_key($sd_row)] = $sd_row;
          }
        }
        db_query($delete_query, $delete_args);
      }

      // Insert new rows.
      if (!empty($insert_rows)) {
        $query = "INSERT INTO {avtbl_transaction_details} (" . implode(', ', $all_table_field_names) . ") VALUES " . implode(', ', $insert_rows);
        db_query($query, $insert_args);
      }
      // Update existing rows.
      if (!empty($update_rows)) {
        $set = array();
        foreach ($update_rows as $field_name => $id_and_val) {
          $when = array();
          foreach ($id_and_val as $k => $v) {
            $arg_id = ':arg_' . count($update_args);
            $when[] = "WHEN $k THEN $arg_id";
            $update_args[$arg_id] = $v;
          }
          if (!empty($when)) {
            $set[] = "$field_name = CASE id " . implode(' ', $when) . ' END';
          }
        }

        if (!empty($set)) {
          $query = "UPDATE {avtbl_transaction_details} SET " . implode(', ', $set) . " WHERE id IN (:transaction_detail_ids)";
          $update_args[':transaction_detail_ids'] = $transaction_detail_ids;
          db_query($query, $update_args);
        }
      }

      // Update product on-hand qty.
      avbase_product_qty_update($item_rows, $row, FALSE, $to_be_deleted_rows);

      // Update cost and price.
      if (!empty($ttd['cost_change'])) {
        avtxns_cost_changes_db_update($form_state, $row, $item_ids, $transaction_type);
      }

      // Discount changes.
      if (!empty($discount_updates)) {
        $discount_update_sets = array();
        $du_item_ids = array();
        foreach ($discount_updates as $discount_field_name => $du) {
          $discount_update_sets[$discount_field_name] = "$discount_field_name = CASE id";
          foreach($du as $du_item_id => $discount_value) {
            $discount_update_sets[$discount_field_name] .= " WHEN $du_item_id THEN $discount_value";
            $du_item_ids[$du_item_id] = $du_item_id;
          }
          $discount_update_sets[$discount_field_name] .= ' END';
        }

        // Update product discounts.
        if (!empty($ttd['product_discount_overwrite']) && !empty($discount_update_sets)) {
          $du_query = "UPDATE {avtbl_products} SET " . implode(', ', $discount_update_sets);
          $du_query .= ' WHERE ID IN (' . implode(', ', $du_item_ids) . ')';
          if (db_query($du_query)) {
            avbase_custom_watchdog("{$transaction_type}__discount_update", "Transaction ID: {$row['id']}", array('#row' => $row, '#item_ids' => $du_item_ids, '#entity_name' => $transaction_type));
          }
        }

        // Send transaction status to PENDING if one of the products' discount is over the limit.
        //$transaction_row = avtxns_txn_load($row['id']);
        if (!empty($ttd['product_discount_check']) && (is_null($transaction_status) || $transaction_status == AVTXNS_TXN_STATUS_OPEN)) {
          $set_to_pending = FALSE;
          $query = "SELECT id, title,
            " . avproducts_discount_value_query() . " AS discount
            FROM {avtbl_products}
            WHERE id IN (:ids)";
          $du_rs = db_query($query, array(':ids' => array_keys($entered_discounts)));
          //dpm($entered_discounts);
          //dpm($item_rows);
          foreach ($du_rs as $du_row) {
            $compare_with = empty($entered_discounts[$du_row->id]) ? array() : $entered_discounts[$du_row->id];
            foreach ($compare_with as $cw_discount) {
              //dpm($du_row->title . '='. round(((float) (1 - $cw_discount)), 4) . ' >? ' .(((float) $du_row->discount)));

              if (round(((float) (1 - $cw_discount)), 4) > round(((float) $du_row->discount), 4)) {
                $set_to_pending = TRUE;
                break;
              }
            }

            //for ($x = 1; $x <= 4; $x++) {
            //  $discount_field_name = 'discount' . $x;
            //  $existing_discount_value = empty($du_row->$discount_field_name) ? 0 : $du_row->$discount_field_name;
            //  $incoming_discount_value = empty($discount_updates[$discount_field_name][$du_row->id]) ? 0 : $discount_updates[$discount_field_name][$du_row->id];
            //  if ($incoming_discount_value > ($existing_discount_value * AVTXN_DISCOUNT_MULTIPLIER)) {
            //    $set_to_pending = TRUE;
            //    break;
            //  }
            //  if ($set_to_pending) {
            //    break;
            //  }
            //}
          }

          if ($set_to_pending) {
            if (avtxns_txn_change_status($row['id'], AVTXNS_TXN_STATUS_PENDING)) {
              $transaction_status = AVTXNS_TXN_STATUS_PENDING;
              drupal_set_message('Some item discounts exceeded the maximum allowed. This transaction has been sent for approval.', 'warning');
            }
          }
        }
      }

      // If payment is check, send to pending.
      if (!empty($ttd['payment']) && $form_state['values']['pmt_method_id'] == AVBASE_PMT_METHOD_CHECK && $transaction_status != AVTXNS_TXN_STATUS_PENDING) {
        if (avtxns_txn_change_status($row['id'], AVTXNS_TXN_STATUS_PENDING)) {
          $transaction_status = AVTXNS_TXN_STATUS_PENDING;
          drupal_set_message('Check payments are temporarily sent for approval.', 'warning');
        }
      }

      $now = time();
      // Close transaction ids.
      if (!empty($close_transaction_ids)) {
        avtxns_txn_reconsider_status($close_transaction_ids);
        //if (db_query("UPDATE {avtbl_transactions} SET changed = :changed, status = 0 WHERE id IN (:ids)", array(':ids' => $close_transaction_ids, ':changed' => $now))) {
        //  avbase_custom_watchdog("{$transaction_type}__close", "ID: multiple", array('#ids' => $close_transaction_ids, '#manner' => 'transaction-auto'));
        //}
      }

      // Suspend/re-activate customer if necessary.
      if ($transaction_type == AVTXNS_TXN_TYPE_DELIVERY && (is_null($transaction_status) || $transaction_status == AVTXNS_TXN_STATUS_OPEN)) {
        // Make this transaction pending if limit is reached.
        $limit_reached = avbase_client_limit_reached($row['client_id'], $row['id']);
        if ($limit_reached && avtxns_txn_change_status($row['id'], AVTXNS_TXN_STATUS_PENDING)) {
          $transaction_status = AVTXNS_TXN_STATUS_PENDING;
          $balance_ages_form = avreports_client_balance_aging_form($row['client_id']);
          drupal_set_message(t('This transaction has been sent for approval. Client\'s @credit_limit credit limit will be reached if this transaction is approved.', array('@credit_limit' => number_format($balance_ages_form['#credit_limit'], 2))), 'warning');
        }
      }
      //elseif ($transaction_type == AVTXNS_TXN_TYPE_PAYMENT) {
      //  $balance_ages_form = avreports_client_balance_aging_form($row['client_id']);
      //  if ($balance_ages_form['#client_status'] != AVCLIENT_STATUS_ACTIVE && empty($balance_ages_form['#limit_reached']) && $balance_ages_form['#client_type'] == AVCLIENT_TYPE_CUSTOMER) {
      //    if (avclients_client_change_status($row['client_id'], AVCLIENT_STATUS_ACTIVE)) {
      //      drupal_set_message(t('Account has been reactivated.'));
      //    }
      //  }
      //}
      break;
  }
  return NULL;
}

/**
 * Extra submit handler for purchase and sales order form.
 * @param $form
 * @param $form_state
 */
function avtxns_txn_form_submit($form, &$form_state) {
  $triggering_element = isset($form_state['triggering_element']) ? $form_state['triggering_element'] : array();
  $transaction_id = empty($form_state['values']['id']) ? NULL : $form_state['values']['id'];
  switch ($triggering_element['#id']) {
    case 'submit_and_send':
      //$mail_to = empty($form_state['values']['email']) ? '' : $form_state['values']['email'];
      avtxns_send_txn_to_email($transaction_id);
      break;
  }
}

/**
 * Extra submit handler for other transaction form.
 * @param $form
 * @param $form_state
 */
function avtxns_txn_form_submit2($form, &$form_state) {
  $form_state['rebuild'] = TRUE;
  $trigger = $form_state['triggering_element'];

  // Get form storage.
  $form_state['storage'] = isset($form_state['storage']) ? $form_state['storage'] : array();
  $_storage = &$form_state['storage'];

  // Get item rows.
  $_storage['item_rows'] = isset($_storage['item_rows']) ? $_storage['item_rows'] : array();
  $_item_rows = &$_storage['item_rows'];

  // Get transaction type.
  $transaction_type = empty($form_state['values']['transaction_type']) ? '' : $form_state['values']['transaction_type'];
  $ttd = avtxns_txn_types($transaction_type);

  switch ($trigger['#id']) {
    case 'detect-cost-change-btn':
    case 'submit_and_print':
      if (empty($ttd['cost_change'])) {
        break;
      }
      // Get non-empty product rows from item rows.
      $entered_products = array();
      $product_ids = array();
      foreach ($_item_rows as $k => $row) {
        if (empty($row['product_id'])) {
          continue;
        }
        $entered_products[$k] = $row;
        $product_ids[$row['product_id']] = $row['product_id'];
      }

      // Load matching product details from db.
      $product_rows = array();
      if (!empty($product_ids)) {
        $product_rows = avproduct_load_ids($product_ids);
      }

      $cost_changes = array();
      foreach ($entered_products as $k => $entered_product) {
        $product_row = empty($product_rows[$entered_product['product_id']]) ? array() : $product_rows[$entered_product['product_id']];
        if (empty($product_row->id)) {
          continue;
        }
        //$incoming_base_qty = $entered_product['qty'] * $entered_product['qty_per_uom'];
        $current_cost = (float) $product_row->cost;
        if ($ttd['transaction'] == 'sales') {
          $current_cost = (float) $product_row->price;
        }
        $current_sales_price = (float) $product_row->price;
        $new_cost = $entered_product['cost'] / $entered_product['qty_per_uom'];
        $new_cost = round((float) $new_cost, AVBASE_DECIMAL_MAX);

        //$proposed_cost = avtxns_compute_product_cost($current_cost, $product_row->qty, $new_cost, $incoming_base_qty);
        $proposed_cost = $new_cost;
        if (round($current_cost, 2) == round($proposed_cost, 2)) {
          continue;
        }

        $price = avtxns_compute_sales_price($current_sales_price, $current_cost, $proposed_cost);
        $price_markup = $price['markup'] * 100;
        $proposed_sales_price = $price['price'];
        $cost_changes[$k] = array(
          'product_id' => $product_row->id,
          'title' => $product_row->title,
          'current_cost' => $current_cost,
          'proposed_cost' => $proposed_cost,
          'current_sales_price' => $current_sales_price,
          'proposed_sales_price' => $proposed_sales_price,
          'price_markup' => $price_markup,
        );
      }

      if (!empty($cost_changes)) {
        $form_state['values']['detected_cost_changes'] = $cost_changes;
        return;
      }
      break;
  }

  //$form_state['rebuild'] = FALSE;
  avbase_crud_form_submit($form, $form_state);

  $transaction_id = empty($form_state['values']['id']) ? NULL : $form_state['values']['id'];
  $redirect_query = array();
  if ($trigger['#id'] == 'submit_and_print') {
    $redirect_query['print'] = 1;
  }
  $form_state['redirect'] = url("{$ttd['base_path']}/$transaction_id/view", array('absolute' => TRUE, 'query' => $redirect_query));

  // Reset form.
  unset($form_state['input']);
  unset($form_state['storage']);
  $form_state['values'] = array();
}

/**
 * Redirect transaction form.
 * @param $form
 * @param $form_state
 */
function avtxns_redirect_transaction_form($form, &$form_state) {
  $triggering_element = isset($form_state['triggering_element']) ? $form_state['triggering_element'] : array();
  $transaction_id = empty($form_state['values']['id']) ? NULL : $form_state['values']['id'];
  $transaction_type = empty($form_state['values']['transaction_type']) ? NULL : $form_state['values']['transaction_type'];
  $ttd = avtxns_txn_types($transaction_type);

  $redirect_query = array();
  if ($triggering_element['#id'] == 'submit_and_print') {
    $redirect_query['print'] = 1;
  }
  $form_state['redirect'] = array($ttd['base_path'] . '/' . $transaction_id . '/view', array('query' => $redirect_query));
}
