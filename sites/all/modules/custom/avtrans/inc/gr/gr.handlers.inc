<?php

/**
 * Perform actions depending on the kind of change that will be made
 * on the database for Goods received.
 * @param $op
 * @param $row
 * @param $form_state
 * @return object|NULL
 */
function avtrans_avbase_crud_dbchange_gr($op, $row, $form_state) {
  switch ($op) {
    case 'pre-save':
      if (empty($row['id'])) {
        $row['transaction_date'] = time();
      }
      return $row;
      break;

    case 'after-save':
      // Get form storage.
      $form_state['storage'] = isset($form_state['storage']) ? $form_state['storage'] : array();
      $_storage = &$form_state['storage'];

      // Get transaction type.
      $transaction_type = empty($_storage['transaction_type']) ? 'gr' : $_storage['transaction_type'];

      // Get item rows.
      $_storage['item_rows'] = isset($_storage['item_rows']) ? $_storage['item_rows'] : array();
      $_item_rows = &$_storage['item_rows'];

      //$product_rows = empty($values['product_rows']) ? array() : $values['product_rows'];
      $incoming_base_qty = array();

      // Save po items.
      $table_name = "avtbl_{$transaction_type}_items";
      if ($transaction_type == 'spay') {
        $table_field_names = array("{$transaction_type}_id", 'paid_amount', 'reference_id', 'reference_type');
      }
      else {
        $table_field_names = array("{$transaction_type}_id", 'product_id', 'uom_id', 'qty_per_uom', 'qty', 'cost', 'reference_id', 'discount');
        if (in_array($transaction_type, array('ret', 'rept'))) {
          $table_field_names[] = 'good_stock';
        }
        if (in_array($transaction_type, array('sinv'))) {
          $table_field_names[] = 'reference_type';
        }
      }


      sort($table_field_names);
      $insert_rows = array();
      $insert_args = array();
      $update_rows = array();
      $update_args = array();
      $item_ids = array();
      $close_transaction_ids = array();
      foreach ($_item_rows as $item_key => $item_row) {
        if ($transaction_type != 'spay' && empty($item_row['product_id'])) {
          continue;
        }

        $item_row['paid_amount'] = $item_row['total'];
        $pending_item = strstr($item_key, 'db_item_') ? TRUE : FALSE;
        if ($pending_item && !empty($item_row["{$transaction_type}_id"])) {
          // Make sure only item rows owned by this owner get saved.
          //if ($item_row['client_id'] != $row['client_id']) {
          //  continue;
          //}
          // Close this PO if all incoming base qty have already been received.'
          //$received_base_qty = $item_row['qty_per_uom'] * $item_row['qty'];
          //if (isset($item_row['remaining_base_qty']) && ($item_row['remaining_base_qty'] <= $received_base_qty)) {
          //  // Yay close it!
          //  db_query("UPDATE {avtbl_po} SET status = 0 WHERE id = :id", array(':id' => $item_row['po_id']));
          //}
        }
        $item_row["{$transaction_type}_id"] = $row['id'];
        //dpm($item_key);
        //dpm($pending_item);
        //dpm(strstr($item_key, 'db_item_'));
        //if ($pending_item) {
        //  $item_row['reference_id'] = isset($item_row['id']) ? $item_row['id'] : NULL;
        //  $item_id = NULL;
        //}
        //else {
          $item_row['reference_id'] = isset($item_row['reference_id']) ? $item_row['reference_id'] : NULL;
          $item_id = empty($item_row['id']) ? 0 : $item_row['id'];
          if (!empty($item_id)) {
            $item_ids[] = $item_id;
          }
        //}

        ksort($item_row);
        $insert_arr = $update_arr = array();
        foreach ($item_row as $k => $v) {
          if (!in_array($k, $table_field_names)) {
            continue;
          }
          $arg_id = ':arg_' . count($insert_args);
          if (empty($item_id)) {
            $insert_arr[] = $arg_id;
            $insert_args[$arg_id] = $v;
          }
          else {
            $update_rows[$k][$item_id] = $v;
          }
        }

        if (!empty($insert_arr)) {
          $insert_rows[] = "(" . implode(',', $insert_arr) . ")";
        }

        if ($transaction_type == 'spay') {
          $close_transaction_ids[] = $item_row['reference_id'];
        }
        elseif ($transaction_type == 'gr') {
          $close_transaction_ids['po'][$item_row['po_id']] = $item_row['po_id'];
        }
        elseif ($transaction_type == 'dr') {
          $close_transaction_ids['so'][$item_row['so_id']] = $item_row['so_id'];
        }
        //elseif ($transaction_type == 'ret') {
        //  $close_transaction_ids['dr'][] = $item_row['dr_id'];
        //}
        elseif ($transaction_type == 'sinv') {
          $reference_type =  empty($item_row['reference_type']) ? '' : $item_row['reference_type'];
          $close_transaction_ids[$reference_type][$item_row[$reference_type . '_id']] = $item_row[$reference_type . '_id'];
        }

        // No need to update inventory if this is sales return and product isn't good stock.
        if ($transaction_type == 'ret' && empty($item_row['good_stock'])) {
          continue;
        }
        // No need to update inventory for invoice transactions.
        if (in_array($transaction_type, array('sinv', 'spay'))) {
          continue;
        }
        // Update this product's inventory count.
        $this_prod_incoming_base_qty = empty($incoming_base_qty[$item_row['product_id']]) ? 0 : $incoming_base_qty[$item_row['product_id']];
        $previous_incoming_base_qty = empty($item_row['incoming_base_qty']) ? 0 : $item_row['incoming_base_qty'];
        $previous_incoming_base_qty = empty($item_row['id']) ? 0 : $previous_incoming_base_qty;
        $incoming_base_qty[$item_row['product_id']] = $this_prod_incoming_base_qty + (($item_row['qty'] * $item_row['qty_per_uom']) - $previous_incoming_base_qty);
      }
      //$query->execute();

      if (!empty($insert_rows)) {
        $query = "INSERT INTO {{$table_name}} (" . implode(', ', $table_field_names) . ") VALUES " . implode(', ', $insert_rows);
        db_query($query, $insert_args);
      }
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
          $query = "UPDATE {{$table_name}} SET " . implode(', ', $set) . " WHERE id IN (:item_ids)";
          $update_args[':item_ids'] = $item_ids;
          db_query($query, $update_args);
        }
      }
      /////////////////////
      //$query = db_insert($table_name)->fields($table_field_names);
      //foreach ($_item_rows as $item_row) {
      //  if (empty($item_row['product_id'])) {
      //    continue;
      //  }
      //
      //  if (!empty($item_row["{$transaction_type}_id"])) {
      //    // Make sure only item rows owned by this owner get saved.
      //    if ($item_row['client_id'] != $row['client_id']) {
      //      continue;
      //    }
      //    // Close this PO if all incoming base qty have already been received.'
      //    //$received_base_qty = $item_row['qty_per_uom'] * $item_row['qty'];
      //    //if (isset($item_row['remaining_base_qty']) && ($item_row['remaining_base_qty'] <= $received_base_qty)) {
      //    //  // Yay close it!
      //    //  db_query("UPDATE {avtbl_po} SET status = 0 WHERE id = :id", array(':id' => $item_row['po_id']));
      //    //}
      //  }
      //
      //  // Do save this item row.
      //  $item_row["{$transaction_type}_id"] = $row['id'];
      //  $item_row['reference_id'] = isset($item_row['id']) ? $item_row['id'] : NULL;
      //  $query->values($item_row);
      //
      //  // No need to update inventory if this is sales return and product isn't good stock.
      //  if ($transaction_type == 'ret' && empty($item_row['good_stock'])) {
      //    continue;
      //  }
      //  // Update this product's inventory count.
      //  $this_prod_incoming_base_qty = empty($incoming_base_qty[$item_row['product_id']]) ? 0 : $incoming_base_qty[$item_row['product_id']];
      //  $incoming_base_qty[$item_row['product_id']] = $this_prod_incoming_base_qty + ($item_row['qty'] * $item_row['qty_per_uom']);
      //}
      //$query->execute();

      // Update product on-hand qty.
      if (!empty($incoming_base_qty)) {
        $qty_update_query = '';
        switch ($transaction_type) {
          case 'gr':
          case 'ret':
            $qty_update_query = "UPDATE {avtbl_products} SET qty = qty + CASE id";
            break;
          case 'dr':
          case 'rept':
            $qty_update_query = "UPDATE {avtbl_products} SET qty = qty - CASE id";
            break;
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

        // Update cost and price.
        avtrans_cost_changes_db_update($form_state, $row, array_keys($incoming_base_qty));
      }

      // Close pos.
      if (!empty($close_transaction_ids)) {
        if ($transaction_type != 'spay') {
          foreach ($close_transaction_ids as $ttype => $tids) {
            if (db_query("UPDATE {avtbl_{$ttype}} SET status = 0 WHERE id IN (:ids)", array(':ids' => $tids))) {
              avbase_custom_watchdog("{$ttype}__close", "ID: multiple", array('#ids' => $tids, '#manner' => 'transaction-auto'));
            }
          }
        }
      }
      break;
  }
  return NULL;
}

/**
 * Submit handler for Goods Received form.
 * @param $form
 * @param $form_state
 */
function avtrans_gr_form_submit($form, &$form_state) {
  $form_state['rebuild'] = TRUE;
  $trigger = $form_state['triggering_element'];

  // Get form storage.
  $form_state['storage'] = isset($form_state['storage']) ? $form_state['storage'] : array();
  $_storage = &$form_state['storage'];

  // Get item rows.
  $_storage['item_rows'] = isset($_storage['item_rows']) ? $_storage['item_rows'] : array();
  $_item_rows = &$_storage['item_rows'];

  // Get transaction type.
  $transaction = empty($_storage['transaction']) ? '' : $_storage['transaction'];
  $transaction_type = empty($_storage['transaction_type']) ? 'gr' : $_storage['transaction_type'];
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
        $incoming_base_qty = $entered_product['qty'] * $entered_product['qty_per_uom'];
        $current_cost = (float) $product_row->cost;
        if ($transaction == 'sales') {
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
  $arg1 = $transaction_type == 'dr' ? 'delivery' :  $transaction_type;
  $redirect_query = array();
  if ($trigger['#id'] == 'submit_and_print') {
    $redirect_query['print'] = 1;
  }
  $form_state['redirect'] = url("av/$arg1/$transaction_id/view", array('absolute' => TRUE, 'query' => $redirect_query));

  // Reset form.
  unset($form_state['input']);
  unset($form_state['storage']);
  $form_state['values'] = array();
}

/**
 * Buttons for Goods Received form.
 * @param $form
 * @param $form_state
 * @param $row
 */
function avtrans_gr_form_buttons(&$form, $form_state, $row, $view_mode = FALSE) {
  // Get form storage.
  $form_state['storage'] = isset($form_state['storage']) ? $form_state['storage'] : array();
  $_storage = &$form_state['storage'];

  // Get transaction type.
  $transaction_type = empty($_storage['transaction_type']) ? 'gr' : $_storage['transaction_type'];
  $form['buttons'] = avtrans_transaction_buttons($transaction_type, $view_mode);
  $form['buttons']['submit']['#id'] = 'detect-cost-change-btn';
  $form['buttons']['submit']['#attributes']['class'][] = 'av-ajax-trigger';
  $form['buttons']['submit']['#ajax'] = array(
    'callback' => 'avtrans_transaction_form_ajax',
    'wrapper' => 'avtrans-transaction-form-wrapper',
    'effect' => 'none',
    'event' => 'click',
  );
  $form['buttons']['submit_and_print']['#attributes'] = $form['buttons']['submit']['#attributes'];
  $form['buttons']['submit_and_print']['#ajax'] = $form['buttons']['submit']['#ajax'];
  //$form['buttons']['submit_and_print']['#id'] = 'detect-cost-change-btn-print';
  //$form['buttons']['submit_and_print']['#value'] = t('Submit and print');
  //$form['buttons']['submit_and_print']['#icon_key'] = 'print';

  //$form['buttons']['#theme'] = 'avbase_crud_button_group';
  //$form['buttons']['submit'] = array(
  //  '#id' => 'detect-cost-change-btn',
  //  '#type' => 'submit',
  //  '#value' => t('Receive these items'),
  //  '#attributes' => array(
  //    'class' => array('av-ajax-trigger', 'uk-button-primary'),
  //  ),
  //  '#ajax' => array(
  //    'callback' => 'avtrans_transaction_form_ajax',
  //    'wrapper' => 'avtrans-transaction-form-wrapper',
  //    'effect' => 'none',
  //    'event' => 'click',
  //  ),
  //);
  //if (empty($view_mode)) {
  //  $form['buttons']['cancel'] = array(
  //    '#markup' => l('Cancel', 'av/transactions', array('attributes' => array('class' => array('uk-button')))),
  //  );
  //}
  //else {
  //  $form['buttons']['print'] = array(
  //    '#id' => 'av-print-btn',
  //    '#type' => 'button',
  //    '#value' => t('Print...'),
  //    '#visible_in_view_mode' => TRUE,
  //    '#attributes' => array('class' => array('uk-button-primary')),
  //  );
  //}
}
