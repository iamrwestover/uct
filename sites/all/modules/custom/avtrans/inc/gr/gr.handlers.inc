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
      $query = db_insert("avtbl_{$transaction_type}_items")->fields(array("{$transaction_type}_id", 'product_id', 'uom_id', 'qty_per_uom', 'qty', 'cost', 'reference_id'));
      foreach ($_item_rows as $item_row) {
        if (empty($item_row['product_id'])) {
          continue;
        }

        if (!empty($item_row["{$transaction_type}_id"])) {
          // Make sure only item rows owned by this owner get saved.
          if ($item_row['client_id'] != $row['client_id']) {
            continue;
          }
          // Close this PO if all incoming base qty have already been received.'
          //$received_base_qty = $item_row['qty_per_uom'] * $item_row['qty'];
          //if (isset($item_row['remaining_base_qty']) && ($item_row['remaining_base_qty'] <= $received_base_qty)) {
          //  // Yay close it!
          //  db_query("UPDATE {avtbl_po} SET status = 0 WHERE id = :id", array(':id' => $item_row['po_id']));
          //}
        }

        // Do save this item row.
        $item_row["{$transaction_type}_id"] = $row['id'];
        $item_row['reference_id'] = isset($item_row['id']) ? $item_row['id'] : NULL;
        $query->values($item_row);

        $this_prod_incoming_base_qty = empty($incoming_base_qty[$item_row['product_id']]) ? 0 : $incoming_base_qty[$item_row['product_id']];
        $incoming_base_qty[$item_row['product_id']] = $this_prod_incoming_base_qty + ($item_row['qty'] * $item_row['qty_per_uom']);
      }
      $query->execute();

      // Update product on-hand qty.
      if (!empty($incoming_base_qty)) {
        $qty_update_query = '';
        switch ($transaction_type) {
          case 'gr':
            $qty_update_query = "UPDATE {avtbl_products} SET qty = qty + CASE id";
            break;
          case 'dr':
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
        }

        // Update cost and price.
        avtrans_cost_changes_db_update($form_state, $row, array_keys($incoming_base_qty));
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
  $arg1 = $transaction_type == 'dr' ? 'delivery' :  'gr';
  $form_state['redirect'] = url("av/$arg1/$transaction_id/view", array('absolute' => TRUE));

  // Reset form.
  unset($form_state['input']);
  unset($form_state['storage']);
  $form_state['values'] = array();
}
