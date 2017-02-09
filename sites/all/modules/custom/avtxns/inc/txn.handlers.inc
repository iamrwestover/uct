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
      // Get item rows.
      $item_rows = isset($form_state['storage']['item_rows']) ? $form_state['storage']['item_rows'] : array();

      // Get transaction type.
      $transaction_type = empty($form_state['values']['transaction_type']) ? '' : $form_state['values']['transaction_type'];
      $ttd = avtxns_txn_types($transaction_type);

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
        'paid_amount',
        'ref_txn_dtl_id',
      );
      //sort($all_table_field_names);

      $incoming_base_qty = array();
      $close_transaction_ids = array();
      $insert_rows = array();
      $insert_args = array();
      $update_rows = array();
      $update_args = array();
      $transaction_detail_ids = array();
      //$matching_table_field_names = array();
      $item_ids = array();
      foreach ($item_rows as $item_row) {
        if (empty($item_row['product_id'])) {
          continue;
        }
        $item_row['transaction_id'] = $row['id'];
        $item_ids[] = $item_row['item_id'] = $item_row['product_id'];
        $item_row['discount1'] = $item_row['discount2'] = $item_row['discount3'] = $item_row['discount4'] = NULL;
        $discount_text = isset($item_row['discount']) ? trim($item_row['discount']) : '';
        $discount_parts = explode('/', $discount_text);
        $x = 1;
        foreach ($discount_parts as $k => $v) {
          if (!is_numeric($v)) {
            continue;
          }
          $item_row['discount' . $x++] = (float) $v;
        }

        $transaction_detail_id = empty($item_row['id']) ? 0 : $item_row['id'];
        if (!empty($transaction_detail_id)) {
          $transaction_detail_ids[] = $transaction_detail_id;
        }

        //ksort($item_row);
        $insert_arr = $update_arr = array();
        foreach ($all_table_field_names as $field_name) {
          $arg_id = ':arg_' . count($insert_args);
          $value = isset($item_row[$field_name]) ? $item_row[$field_name] : NULL;
          if (empty($transaction_detail_id)) {
            $insert_arr[] = $arg_id;
            $insert_args[$arg_id] = $value;
            //$matching_table_field_names[$k] = $k;
          }
          elseif (!is_null($value)) {
            $update_rows[$field_name][$transaction_detail_id] = $value;
          }
        }

        //foreach ($item_row as $k => $v) {
        //  if (!in_array($k, $all_table_field_names)) {
        //    continue;
        //  }
        //  $arg_id = ':arg_' . count($insert_args);
        //  if (empty($transaction_detail_id)) {
        //    $insert_arr[] = $arg_id;
        //    $insert_args[$arg_id] = $v;
        //    $matching_table_field_names[$k] = $k;
        //  }
        //  else {
        //    $update_rows[$k][$transaction_detail_id] = $v;
        //  }
        //}

        if (!empty($insert_arr)) {
          $insert_rows[] = "(" . implode(',', $insert_arr) . ")";
        }

        // Find transaction IDs to close.
        //if ($transaction_type == 'spay') {
        //  $close_transaction_ids[] = $item_row['reference_id'];
        //}
        //elseif ($transaction_type == 'gr') {
        //  $close_transaction_ids['po'][$item_row['po_id']] = $item_row['po_id'];
        //}
        //elseif ($transaction_type == 'dr') {
        //  $close_transaction_ids['so'][$item_row['so_id']] = $item_row['so_id'];
        //}
        ////elseif ($transaction_type == 'ret') {
        ////  $close_transaction_ids['dr'][] = $item_row['dr_id'];
        ////}
        //elseif ($transaction_type == 'sinv') {
        //  $reference_type =  empty($item_row['reference_type']) ? '' : $item_row['reference_type'];
        //  $close_transaction_ids[$reference_type][$item_row[$reference_type . '_id']] = $item_row[$reference_type . '_id'];
        //}

        if (!empty($item_row['ref_txn_id'])) {
          $close_transaction_ids[$item_row['ref_txn_id']] = $item_row['ref_txn_id'];
        }

        // No need to update inventory for specific transactions.
        if (!isset($ttd['qty_change'])) {
          continue;
        }

        // Update this product's inventory count.
        $this_prod_total_incoming_base_qty = empty($incoming_base_qty[$item_row['item_id']]) ? 0 : $incoming_base_qty[$item_row['item_id']];
        $previous_incoming_base_qty = empty($item_row['incoming_base_qty']) ? 0 : $item_row['incoming_base_qty'];
        $previous_incoming_base_qty = empty($item_row['id']) ? 0 : $previous_incoming_base_qty;
        $item_row_incoming_base_qty = ($item_row['qty'] * $item_row['qty_per_uom']);
        if ($transaction_type == AVTXNS_TXN_TYPE_SALES_RETURN && empty($item_row['good_stock'])) {
          $item_row_incoming_base_qty = 0;
        }
        $incoming_base_qty[$item_row['item_id']] = $this_prod_total_incoming_base_qty + $item_row_incoming_base_qty - $previous_incoming_base_qty;
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
      if (!empty($incoming_base_qty)) {
        $qty_update_query = '';
        if (isset($ttd['qty_change'])) {
          if ($ttd['qty_change'] == 'increment') {
            $qty_update_query = "UPDATE {avtbl_products} SET qty = qty + CASE id";
          }
          elseif ($ttd['qty_change'] == 'decrement') {
            $qty_update_query = "UPDATE {avtbl_products} SET qty = qty - CASE id";
          }
        }
        if (!empty($qty_update_query)) {
          foreach ($incoming_base_qty as $k => $v) {
            $qty_update_query .= " WHEN $k THEN $v";
          }
          $qty_update_query .= " END WHERE id IN (" . implode(', ', array_keys($incoming_base_qty)) . ")";
          if (db_query($qty_update_query)) {
            avbase_custom_watchdog("{$transaction_type}__qty_update", "ID: {$row['id']}", array('#row' => $row, '#incoming_base_qty' => $incoming_base_qty, '#entity_name' => $transaction_type));
          }
        }
        else {
          drupal_set_message('Product quantities were not updated', 'warning');
          avbase_custom_watchdog("{$transaction_type}__qty_update_warn1", "ID: {$row['id']}", array('#row' => $row, '#incoming_base_qty' => $incoming_base_qty, '#entity_name' => $transaction_type));
        }

      }

      // Update cost and price.
      if (!empty($ttd['cost_change'])) {
        avtxns_cost_changes_db_update($form_state, $row, $item_ids, $transaction_type);
      }

      if (!empty($close_transaction_ids) && $transaction_type != AVTXNS_TXN_TYPE_SALES_RETURN) {
        //if ($transaction_type != 'spay') {
        //  foreach ($close_transaction_ids as $ttype => $tids) {
            if (db_query("UPDATE {avtbl_transactions} SET status = 0 WHERE id IN (:ids)", array(':ids' => $close_transaction_ids))) {
              avbase_custom_watchdog("{$transaction_type}__close", "ID: multiple", array('#ids' => $close_transaction_ids, '#manner' => 'transaction-auto'));
            }
          //}
        //}
      }
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

        //$proposed_cost = avtrans_compute_product_cost($current_cost, $product_row->qty, $new_cost, $incoming_base_qty);
        $proposed_cost = $new_cost;
        if (round($current_cost, 2) == round($proposed_cost, 2)) {
          continue;
        }

        $price = avtrans_compute_sales_price($current_sales_price, $current_cost, $proposed_cost);
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
