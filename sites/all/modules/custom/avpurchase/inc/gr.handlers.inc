<?php

/**
 * Perform actions depending on the kind of change that will be made
 * on the database for Goods received.
 * @param $op
 * @param $row
 * @param $form_state
 * @return object|NULL
 */
function avpurchase_avbase_crud_dbchange_gr($op, $row, $form_state) {
  switch ($op) {
    case 'pre-save':
      $row['gr_date'] = time();
      return $row;
      break;

    case 'after-save':
      // Get form storage.
      $form_state['storage'] = isset($form_state['storage']) ? $form_state['storage'] : array();
      $_storage = &$form_state['storage'];

      // Get item rows.
      $_storage['item_rows'] = isset($_storage['item_rows']) ? $_storage['item_rows'] : array();
      $_item_rows = &$_storage['item_rows'];

      // Get form values.
      $values = empty($form_state['values']) ? array() : $form_state['values'];

      //$product_rows = empty($values['product_rows']) ? array() : $values['product_rows'];
      $incoming_base_qty = array();

      // Save po items.
      $query = db_insert('avtbl_gr_items')->fields(array('gr_id', 'product_id', 'uom_id', 'qty_per_uom', 'qty', 'cost', 'po_item_id'));
      foreach ($_item_rows as $item_row) {
        if (empty($item_row['product_id'])) {
          continue;
        }

        if (!empty($item_row['po_id'])) {
          // Make sure only item rows owned by this owner get saved.
          if ($item_row['vendor_id'] != $row['vendor_id']) {
            continue;
          }
          // Close this PO if all incoming base qty have already been receieved.'
          //$received_base_qty = $item_row['qty_per_uom'] * $item_row['qty'];
          //if (isset($item_row['remaining_base_qty']) && ($item_row['remaining_base_qty'] <= $received_base_qty)) {
          //  // Yay close it!
          //  dpm('closed!' . $item_row['po_id']);
          //  db_query("UPDATE {avtbl_po} SET status = 0 WHERE id = :id", array(':id' => $item_row['po_id']));
          //}
        }

        // Do save this item row.
        $item_row['gr_id'] = $row['id'];
        $item_row['po_item_id'] = isset($item_row['id']) ? $item_row['id'] : NULL;
        $query->values($item_row);

        $this_prod_incoming_base_qty = empty($incoming_base_qty[$item_row['product_id']]) ? 0 : $incoming_base_qty[$item_row['product_id']];
        $incoming_base_qty[$item_row['product_id']] = $this_prod_incoming_base_qty + ($item_row['qty'] * $item_row['qty_per_uom']);
      }
      $query->execute();

      // Update product on-hand qty.
      if (!empty($incoming_base_qty)) {
        $query = "UPDATE {avtbl_products} SET qty = qty + CASE id";
        foreach ($incoming_base_qty as $k => $v) {
          $query .= " WHEN $k THEN $v";
        }
        $query .= " END WHERE id IN (" . implode(', ', array_keys($incoming_base_qty)) . ")";
        if (db_query($query)) {
          avbase_custom_watchdog("gr__qty_update", "ID: {$row['id']}", array('#row' => $row, '#incoming_base_qty' => $incoming_base_qty, '#entity_name' => 'gr'));
        }

        // Update cost and price.
        $cost_changes = empty($values['product_cost_changes']) ? array() : $values['product_cost_changes'];
        if (!empty($cost_changes)) {
          $cost_set = array();
          $price_set = array();
          $cost_args = array();
          $price_args = array();
          foreach ($cost_changes['rows'] as $cost_change) {
            $changes = empty($cost_change['values']) ? array() : $cost_change['values'];
            if (empty($changes)) {
              continue;
            }
            $product_id = empty($changes['product_id']) ? 'none' : $changes['product_id'];
            if (!array_key_exists($product_id, $incoming_base_qty)) {
              continue;
            }

            switch ($changes['action']) {
              case 'cost':
              case 'cost_and_price':
                $cost_set[$product_id] = "WHEN $product_id THEN :cost{$product_id}";
                $cost_args[":cost{$product_id}"] = $changes['new_cost'];
                if ($changes['action'] == 'cost_and_price') {
                  $price_set[$product_id] = "WHEN $product_id THEN :price{$product_id}";
                  $price_args[":price{$product_id}"] = $changes['new_sales_price'];
                }
                break;
              default:
                continue;
                break;
            }
          }

          if (!empty($cost_set)) {
            $query = "UPDATE {avtbl_products} SET cost = CASE id " . implode(' ', $cost_set) . " END WHERE ID IN(:product_ids)";
            $cost_args[':product_ids'] = array_keys($cost_set);
            if (db_query($query, $cost_args)) {
              avbase_custom_watchdog("gr__cost_update", "ID: {$row['id']}", array('#row' => $row, '#cost_changes' => $cost_changes, '#cost_set' => $cost_set, '#args' => $cost_args));
            }
          }
          if (!empty($price_set)) {
            $query = "UPDATE {avtbl_products} SET price = CASE id " . implode(' ', $price_set) . " END WHERE ID IN(:product_ids)";
            $price_args[':product_ids'] = array_keys($price_set);
            if (db_query($query, $price_args)) {
              avbase_custom_watchdog("gr__price_update", "ID: {$row['id']}", array('#row' => $row, '#cost_changes' => $cost_changes, '#price_set' => $price_set, '#args' => $price_args));
            }
          }
        }
      }
      break;
  }
  return NULL;
}

/**
 * Ajax callback before submitting a Receive Items form.
 */
function avpurchase_gr_form_ajax($form, $form_state) {
  if (form_get_errors()) {
    // Prevent modal from popping out.
    unset($form['cost_changes']['product_cost_changes']);
  }
  return $form;
}

/**
 * Submit handler for Goods Received form.
 * @param $form
 * @param $form_state
 */
function avpurchase_gr_form_submit($form, &$form_state) {
  $form_state['rebuild'] = TRUE;
  $trigger = $form_state['triggering_element'];
  switch ($trigger['#id']) {
    case 'detect-cost-change-btn':
      // Get form storage.
      $form_state['storage'] = isset($form_state['storage']) ? $form_state['storage'] : array();
      $_storage = &$form_state['storage'];

      // Get item rows.
      $_storage['item_rows'] = isset($_storage['item_rows']) ? $_storage['item_rows'] : array();
      $_item_rows = &$_storage['item_rows'];

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
        $incoming_base_qty = $entered_product['qty'] * $entered_product['qty_per_uom'];
        $current_cost = (float) $product_row->cost;
        $current_sales_price = (float) $product_row->price;
        $new_cost = $entered_product['cost'] / $entered_product['qty_per_uom'];
        $new_cost = round((float) $new_cost, AVBASE_MONEY_DECIMAL_MAX);

        $proposed_cost = avpurchase_compute_product_cost($current_cost, $product_row->qty, $new_cost, $incoming_base_qty);
        if (round($current_cost, 2) == round($proposed_cost, 2)) {
          continue;
        }

        $price = avpurchase_compute_sales_price($current_sales_price, $current_cost, $proposed_cost);
        $price_markup = $price['markup'] * 100;
        $proposed_sales_price = $price['price'];
        $cost_changes[$k] = array(
          'product_id' => $product_row->id,
          'title' => check_plain($product_row->title),
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
  // Reset form.
  unset($form_state['input']);
  unset($form_state['storage']);
  $form_state['values'] = array();
}

