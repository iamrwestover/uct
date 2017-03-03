<?php
module_load_include('inc', 'avtxns', 'inc/po.helpers');
//module_load_include('inc', 'avtxns', 'inc/so.helpers');
//module_load_include('inc', 'avtxns', 'inc/gr/gr.helpers');
//module_load_include('inc', 'avtxns', 'inc/delivery/delivery.helpers');
//module_load_include('inc', 'avtxns', 'inc/ret/ret.helpers');
//module_load_include('inc', 'avtxns', 'inc/rept/rept.helpers');
//module_load_include('inc', 'avtxns', 'inc/sinv/sinv.helpers');
//module_load_include('inc', 'avtxns', 'inc/spay/spay.helpers');

/**
 * Load transaction details.
 * @param $id
 *
 * @return object
 */
function avtxns_txn_load($id) {
  if (empty($id) || $id == 'new') {
    return FALSE;
  }

  $id = (int) $id;
  if (empty($id)) {
    return NULL;
  }

  $data = &drupal_static('avbase', array());
  $txns = &$data['txns'];
  if (isset($txns[$id])) {
    return $txns[$id];
  }

  $conditions = array();
  $conditions[] = array('avtxns.id', $id);
  $rows = avtxns_txn_query_load($conditions);
  $row = empty($rows[$id]) ? FALSE : $rows[$id];

  // Update static variable.
  $txns[$id] = $row;
  return $row;
}

/**
 * Load transaction details by ids.
 * @param array $ids
 *
 * @return array
 */
function avtxns_txn_load_ids($ids) {
  if (empty($ids)) {
    return FALSE;
  }

  $data = &drupal_static('avbase', array());
  $txns = &$data['txns'];

  $return = array();

  $query_ids = array();
  foreach ($ids as $id) {
    if (!empty($id)) {
      if (isset($txns[$id])) {
        $return[$id] = $txns[$id];
      }
      else {
        $query_ids[] = $id;
      }
    }
  }

  if (!empty($query_ids)) {
    $conditions = array();
    $conditions[] = array('avtxns.id', $query_ids, 'IN');
    $rows = avtxns_txn_query_load($conditions);
    if (!empty($rows)) {
      $return += $rows;
    }
  }

  return $return;
}

/**
 * Advanced transaction load.
 * @param array $conditions
 *
 * @return array
 */
function avtxns_txn_query_load($conditions = array(), $offset = NULL, $limit = NULL) {
  //if (empty($conditions)) {
  //  return array();
  //}

  // Load products static variable.
  $avbase = &drupal_static('avbase', array());
  $avbase['txns'] = empty($avbase['txns']) ? array() : $avbase['txns'];
  $txns = &$avbase['txns'];

  // Set/load static variable for this query.
  $data = &drupal_static(__FUNCTION__, array());
  $_data_id = $conditions;
  sort($_data_id);
  $data_id = md5(json_encode($_data_id));
  if (isset($data[$data_id])) {
    return $data[$data_id];
  }

  $query = db_select('avtbl_transactions', 'avtxns');
  $query->leftJoin('avtbl_clients', 'avclients', 'avclients.id = avtxns.client_id');
  $query->leftJoin('avtbl_categories', 'avcat_agent', "avcat_agent.id = avtxns.agent_id AND avcat_agent.group_id = CONCAT('agent', avclients.client_type)");
  $query->leftJoin('avtbl_categories', 'avcat_terms', 'avcat_terms.id = avtxns.term_id');
  $query->addField('avclients', 'display_name', 'client_name');
  $query->addField('avclients', 'address');
  $query->addField('avclients', 'city');
  $query->addField('avclients', 'province');
  $query->addField('avclients', 'zip_code');
  $query->addField('avclients', 'status', 'client_status');
  $query->addField('avcat_agent', 'title', 'agent_name');
  $query->addField('avcat_terms', 'title', 'term_name');
  $query->addField('avcat_terms', 'data', 'term_data');
  $query->fields('avtxns');
  //->fields('avclient', array('display_name'))
  //->condition('avtxns.id', $id)
  //->execute();
  foreach ($conditions as $condition) {
    $field_name = $condition[0];
    $field_value = isset($condition[1]) ? $condition[1] : NULL;
    $field_operator = isset($condition[2]) ? $condition[2] : NULL;
    $query->condition($field_name, $field_value, $field_operator);
  }

  if (isset($offset) && isset($limit)) {
    $query->range($offset, $limit);
  }

  $query->orderBy('created', 'DESC');
  $rs = $query->execute();
  $rows = array();
  foreach ($rs as $row) {
    $subtotal = 0;
    $credit_total = 0;
    $debit_total = 0;
    if (!empty($row->id)) {
      $transaction_date_formatted = format_date($row->transaction_date, 'custom', 'Y M d');
      //$row->term_data = unserialize($row->term_data);
      $query = db_select('avtbl_transaction_details', 'avtxn_details');
      $query->fields('avtxn_details');
      if ($row->transaction_type != AVTXNS_TXN_TYPE_PAYMENT) {
        $query->leftJoin('avtbl_products', 'avprod', 'avprod.id = avtxn_details.item_id');
        $query->leftJoin('avtbl_categories', 'avcat', 'avcat.id = avtxn_details.uom_id');
        $query->leftJoin('avtbl_categories', 'avcat_base', 'avcat_base.id = avprod.uom_id');
        $query->leftJoin('avtbl_categories', 'avcat_catprod', 'avcat_catprod.id = avprod.category_id');
        //$query->leftJoin('avtbl_transaction_details', 'avtxn_reference', 'avtxn_reference.id = avtxn_details.ref_txn_dtl_id');
        //$query->leftJoin('avtbl_transactions', 'avtxn', 'avtxn.id = avtxn_reference.transaction_id');
        $query->addField('avprod', 'title', 'product_title');
        $query->addField('avprod', 'uom_id', 'base_uom_id');
        if ($row->status == AVTXNS_TXN_STATUS_PENDING) {
          $query->addField('avprod', 'discount1', 'prod_discount1');
          $query->addField('avprod', 'discount2', 'prod_discount2');
          $query->addField('avprod', 'discount3', 'prod_discount3');
          $query->addField('avprod', 'discount4', 'prod_discount4');
        }
        $query->addField('avcat', 'title', 'uom_title');
        $query->addField('avcat_base', 'title', 'base_uom_title');
        $query->addField('avcat_catprod', 'title', 'category_title');
        //$query->addField('avtxn_reference', 'transaction_id', 'ref_txn_id');
        //$query->addField('avtxn', 'transaction_type', 'ref_txn_type');
        $query->orderBy('category_title', 'ASC');
        $query->orderBy('product_title', 'ASC');
      }
      $query->condition('avtxn_details.transaction_id', $row->id)->execute();
      $details_rs = $query->execute();
      foreach ($details_rs as $item) {
        $item->transaction_date_formatted = $transaction_date_formatted;
        $item->total = $item->cost * $item->qty;
        $discount = array();
        $prod_discount = array();
        for ($x = 1; $x <= 4; $x++) {
          $_discount = 0;
          $_max_discount = 0;
          $_discount_name = "discount$x";
          if (isset($item->$_discount_name)) {
            $_discount = round($item->$_discount_name, 2);
            $discount[] = $_discount;
          }

          $_prod_discount_name = "prod_discount$x";
          if (isset($item->$_prod_discount_name)) {
            $_max_discount = round($item->$_prod_discount_name * AVTXN_DISCOUNT_MULTIPLIER, 2);
            $prod_discount[] = $_max_discount;
          }

          if ($row->status == AVTXNS_TXN_STATUS_PENDING && $_max_discount && $_discount > $_max_discount) {
            $item->discount_over_limit = TRUE;
          }
        }
        $discount = array_filter($discount);
        $prod_discount = array_filter($prod_discount);
        $item->discount_text = implode('/', $discount);
        $item->max_discount_text = implode('/', $prod_discount);
        foreach ($discount as $v) {
          if (is_numeric($v)) {
            $v = (float)$v;
            $item->total = $item->total - (($item->total * $v) / 100);
          }
        }

        if (!empty($item->ref_txn_type)) {
          if ($item->ref_txn_type == AVTXNS_TXN_TYPE_SALES_RETURN) {
            $credit_total += $item->total;
            $item->total *= -1;
          } else {
            $debit_total += $item->total;
          }
        }

        $item->qty_per_uom = empty($item->qty_per_uom) ? 1 : $item->qty_per_uom;
        $item->incoming_base_qty = $item->qty * $item->qty_per_uom;
        if ($row->transaction_type == AVTXNS_TXN_TYPE_SALES_RETURN && empty($item->good_stock)) {
          $item->incoming_base_qty = 0;
        }
        $item->remaining_base_qty = $item->incoming_base_qty;
        $row->items[$item->id] = $item;

        // Special fields for payment transactions.
        if ($row->transaction_type == AVTXNS_TXN_TYPE_PAYMENT) {
          $subtotal += $item->paid_amount;
          $invoice = avtxns_txn_load($item->ref_txn_dtl_id);
          if (!empty($invoice->id)) {
            // Get total paid - excluding payments before this transaction.
            $query = "SELECT SUM(avtxn_details.paid_amount) FROM {avtbl_transaction_details} avtxn_details
              INNER JOIN {avtbl_transactions} avtxn ON avtxn.id = avtxn_details.transaction_id
              WHERE avtxn_details.ref_txn_id = :invoice_id AND avtxn_details.transaction_id < :payment_id
                 AND avtxn.status <> " . AVTXNS_TXN_STATUS_VOID;
            $total_paid_for_invoice = db_query($query, array(':invoice_id' => $invoice->id, ':payment_id' => $row->id))->fetchField();
            $total_paid_for_invoice = empty($total_paid_for_invoice) ? 0 : $total_paid_for_invoice;
            $item->balance = $invoice->grand_total - $total_paid_for_invoice;
            $item->previous_payment = $total_paid_for_invoice;
            $item->grand_total = $invoice->grand_total;
          }
        }
        else {
          $subtotal += $item->total;
        }
      }
    }
    //$row->discount_value_text = $row->discount_type == AVBASE_DISCOUNT_PERCENT ? (round($row->discount_value, 2) . '%') : number_format($row->discount_value, 2);
    //$row->subtotal = $subtotal;
    //$row->discounted_value = $row->discount_type == AVBASE_DISCOUNT_PERCENT ? (($row->discount_value / 100) * $subtotal) : $row->discount_value;
    //$row->grand_total = $subtotal - $row->discounted_value;
    $row->grand_total = $subtotal;
    $row->credit_total = $credit_total;
    $row->debit_total = $debit_total;

    // Client address string.
    $row->address_string = avclients_address_string_format($row);

    // Attach received items.
    //avtxns_txn_attach_received_items($row);

    if ($row->transaction_type == AVTXNS_TXN_TYPE_INVOICE) {
      $query = "SELECT SUM(avtxn_details.paid_amount) FROM {avtbl_transaction_details} avtxn_details
        INNER JOIN {avtbl_transactions} avtxn ON avtxn.id = avtxn_details.transaction_id
        WHERE avtxn_details.ref_txn_dtl_id = :invoice_id AND avtxn.status <> " . AVTXNS_TXN_STATUS_VOID;
      $total_paid_for_invoice = db_query($query, array(':invoice_id' => $row->id))->fetchField();
      $total_paid_for_invoice = empty($total_paid_for_invoice) ? 0 : $total_paid_for_invoice;
      $row->total_paid = $total_paid_for_invoice;
      $row->balance = $row->grand_total - $total_paid_for_invoice;
      //if ($row->status && (round($total_paid_for_invoice, 2) == round($row->grand_total, 2))) {
      //  // Close invoice.
      //  if (db_query("UPDATE {avtbl_transactions} SET status = 0 WHERE id = :invoice_id", array(':invoice_id' => $row->id))) {
      //    avbase_custom_watchdog(AVTXNS_TXN_TYPE_INVOICE . "__close", "ID: {$row->id}", array('#row' => $row, '#manner' => 'auto'));
      //    $row->status = 0;
      //  }
      //}
    }

    // Term data.
    if (isset($row->term_data)) {
      $row->term_data = unserialize($row->term_data);
    }

    $rows[$row->id] = $row;
  }

  // Update static variables.
  $data[$data_id] = $rows;
  $txns += $rows;
  return $rows;
}

/**
 * Loads transaction details and sends it to specified email.
 * If email is not specified, po email will be used.
 * @param $transaction_id
 * @param string $email
 *
 * @return bool
 */
function avtxns_send_txn_to_email($transaction_id, $email = '') {
  if (empty($transaction_id)) {
    return FALSE;
  }

  $transaction = avtxns_txn_load($transaction_id);
  $email = empty($email) ? $transaction->email : $email;
  if ($error = user_validate_mail($email)) {
    drupal_set_message($error, 'error');
    return FALSE;
  }

  //$transaction->discount_value = round($transaction->discount_value, 2);
  $params = array(
    'transaction' => $transaction,
    'transaction_id' => $transaction->id,
    'transaction_date' => format_date($transaction->created, 'custom', 'F d, Y'),
    'site_name' => variable_get('site_name', 'UCT'),
    'shipping_address' => variable_get('av_company_address', '-'),
    'client_name' => check_plain($transaction->client_name),
    //'payment_term_title' => check_plain($transaction->payment_term_title),
  );

  $params['transaction_table'] = theme('avtxns_txn_mail_table', $params);
  $message = drupal_mail('avtxns', 'transaction', $email, language_default(), $params);
  if (!empty($message['result'])) {
    drupal_set_message(t('Purchase order sent successfully to %mail.', array('%mail' => $email)));
    avbase_custom_watchdog("crud_mail_po", "Sent Purchase Order #@transaction_id to @client_name",
      array('@transaction_id' => $transaction->id, '@client_name' => $transaction->client_name , '#message' => $message)
    );
    return TRUE;
  }
  return FALSE;
}

/**
 * Advanced transaction load.
 * @param array $param_where
 * @param array $param_args
 * @param null $limit
 * @param null $offset
 * @param null $order_by
 * @param $count_query
 *
 * @return array
 */
function avtxns_txns_query_load($param_where = array(), $param_args = array(), $limit = NULL, $offset = NULL, $order_by = NULL, $more_select = array(), $more_joins = array(), $group_by = array(), $count_query = FALSE) {
  // Load vendors static variable.
  //$avbase = &drupal_static('avbase', array());
  //$avbase['vendors'] = empty($avbase['vendors']) ? array() : $avbase['vendors'];
  //$_vendors = &$avbase['vendors'];

  // Set / load static variable for this query.
  $data = &drupal_static(__FUNCTION__, array());
  //$_data_id = $param_where;
  //sort($_data_id);

  // Set and check if data for data_id already exists.
  $func_args = func_get_args();
  foreach ($func_args as $func_k => $func_arg) {
    if (is_array($func_arg)) {
      asort($func_args[$func_k]);
    }
  }
  $data_id = md5(json_encode($func_args));
  if (isset($data[$data_id])) {
    return $data[$data_id];
  }

  //$transaction_types = empty($param_where['transaction_types']) ? array() : $param_where['transaction_types'];
  //unset($param_where['transaction_types']);

  //$queries = array();
  $select = array();
  if ($count_query) {
    $select[] = 'COUNT(avtxn.id)';
  }
  else {
    $select[] = 'avtxn.id';
    $select[] = 'avtxn.uid';
    $select[] = 'avtxn.status';
    $select[] = 'avtxn.transaction_type';
    $select[] = 'avtxn.transaction_date';
    $select[] = 'avcat_agent.title AS agent_name';
    $select[] = 'avclients.display_name';
    //$select[] = 'avtxn.term_id';
    $select[] = 'avcat_term.title AS term_name';
    $select[] = 'u.name';
    //$select[] = 'avtxn_detail.ref_txn_type';
    //$select[] = 'avtxn_detail.ref_txn_dtl_id';
    //$select[] = "IF (
    //    avtxn.transaction_type = '" . AVTXNS_TXN_TYPE_PAYMENT . "',
    //    SUM(avtxn_detail.paid_amount),
    //    (SUM(
    //        avtxn_detail.qty
    //        * avtxn_detail.cost
    //        * (IF(avtxn_detail.discount1, (1 - avtxn_detail.discount1 / 100), 1))
    //        * (IF(avtxn_detail.discount2, (1 - avtxn_detail.discount2 / 100), 1))
    //        * (IF(avtxn_detail.discount3, (1 - avtxn_detail.discount3 / 100), 1))
    //        * (IF(avtxn_detail.discount4, (1 - avtxn_detail.discount4 / 100), 1))
    //      )
    //    )
    //  ) AS grand_total";
  }
  $select = array_merge($select, $more_select);

  $joins = array();
  $joins[] = 'INNER JOIN {avtbl_clients} AS avclients ON avclients.id = avtxn.client_id';
  $joins[] = 'LEFT JOIN {avtbl_categories} AS avcat_agent ON avcat_agent.id = avtxn.agent_id';
  $joins[] = 'LEFT JOIN {avtbl_categories} AS avcat_term ON avcat_term.id = avtxn.term_id';
  $joins[] = 'LEFT JOIN {users} AS u ON u.uid = avtxn.uid';
  $joins = array_merge($joins, $more_joins);
  //$joins[] = 'LEFT JOIN {avtbl_transaction_details} AS avtxn_detail ON avtxn_detail.transaction_id = avtxn.id';
  //$joins[] = "LEFT JOIN {avtbl_transaction_details} AS avtxn_detail_ref ON
  //  IF (avtxn.transaction_type = '" . AVTXNS_TXN_TYPE_PAYMENT . "', 0, avtxn_detail_ref.id = avtxn_detail.ref_txn_dtl_id)";

  $query = "SELECT " . implode(', ', $select) . " FROM {avtbl_transactions} AS avtxn " . implode(' ', $joins);
  if (!empty($param_where)) {
    $query .= " WHERE " . implode(' AND ', $param_where);
  }

  if (empty($count_query)) {
    if (!empty($group_by)) {
      $query .= " GROUP BY " . implode(', ', $group_by);
    }
    //$query .= " GROUP BY avtxn.id, avtxn_detail.ref_txn_type";
    $order_by = is_null($order_by) ? array('avtxn.transaction_date DESC', 'avtxn.id DESC') : $order_by;
    if (!empty($order_by)) {
      $query .= " ORDER BY " . implode(', ', $order_by);
    }

    if (!is_null($limit)) {
      $query .= " LIMIT $limit";
      if (!is_null($offset)) {
        $query .= " OFFSET $offset";
      }
    }
  }


  if ($count_query) {
    $rows = db_query($query, $param_args)->fetchField();
  }
  else {
    $rs = db_query($query, $param_args);
    $rows = array();
    foreach ($rs as $row) {
      $rows[] = $row;
    }
  }

  $data[$data_id] = $rows;
  return $rows;
}

/**
 * Compute new product cost.
 * @param float $current_cost
 * @param $onhand_qty
 * @param float $new_cost
 * @param $incoming_base_qty
 *
 * @return float
 */
function avtxns_compute_product_cost($current_cost, $onhand_qty, $new_cost, $incoming_base_qty) {
  $current_cost = empty($current_cost) ? $new_cost : $current_cost;
  return ((($current_cost * $onhand_qty) + ($new_cost * $incoming_base_qty)) / ($onhand_qty + $incoming_base_qty));
}

/**
 * Compute new sales price by using the markup difference of current sales price
 * and current cost and adding it to the new cost.
 * @param float $current_sales_price
 * @param float $current_cost
 * @param float $new_cost
 *
 * @return float
 */
function avtxns_compute_sales_price($current_sales_price, $current_cost, $new_cost) {
  if ($current_sales_price <= $current_cost || empty($current_cost)) {
    $markup = 0.2;
  }
  else {
    $markup = ($current_sales_price - $current_cost) / $current_cost;
  }

  return array('markup' => $markup, 'price' => ($new_cost + ($new_cost * $markup)));
}

/**
 * Set common fields when purchasing items.
 *
 * @param $form
 * @param $form_state
 * @param $row
 */
function avtxns_purchase_fields(&$form, &$form_state, $txn_type, $row, $view_mode = FALSE){
  // Set parameters for client name field.
  $form['client_name']['#title'] = t('Vendor');
  $form['client_name']['#table_name'] = 'avtbl_vendors';
  $form['client_name']['#autocomplete_path'] = 'av/vendors/autocomplete';
  $form['client_name']['#avbase_autocomplete'] = array(
    'entity_group' => 'vendors'
  );
}

/**
 * Set common fields when selling items.
 *
 * @param $form
 * @param $form_state
 * @param $row
 */
function avtxns_sales_fields(&$form, &$form_state, $row, $view_mode = FALSE){
  // Set parameters for client name field.
  $form['client_name']['#title'] = t('Customer');
  $form['client_name']['#table_name'] = 'avtbl_customers';
  $form['client_name']['#autocomplete_path'] = 'av/customers/autocomplete';
  $form['client_name']['#avbase_autocomplete'] = array(
    'entity_group' => 'customers'
  );
}

/**
 * Ajax callback before submitting a Receive Items form.
 */
function avtxns_txn_form_ajax($form, $form_state) {
  if (form_get_errors()) {
    // Prevent modal from popping out.
    unset($form['cost_changes']['product_cost_changes']);
    unset($form['pending_orders']['selected_pos']);
  }

  if (!empty($form_state['redirect'])) {
    $commands = array();
    $commands[] = array(
      // Note: we will use this command in the .js file
      'command' => 'redirectUser',
      // The path the user is directed to is given here
      'path' => $form_state['redirect'],
    );
    return array('#type' => 'ajax', '#commands' => $commands);
  }
  else {
    return $form;
  }
}

/**
 * Check if product quantity is enough to give out.
 * @param $element
 * @param $form_state
 */
function avtxns_element_validate_enough_qty(&$element, &$form_state) {
  $element_value = $element['#value'];
  if (is_numeric($element_value) == '') {
    return;
  }

  $item_row_key = isset($element['#item_row_key']) ? $element['#item_row_key'] : '';
  if (empty($item_row_key)) {
    return;
  }

  // Get form storage.
  $transaction_type = empty($form_state['values']['transaction_type']) ? '' : $form_state['values']['transaction_type'];

  // Get item rows.
  $item_rows = empty($form_state['values']['product_rows']) ? array() : $form_state['values']['product_rows'];
  $element_item_row = empty($item_rows[$item_row_key]) ? array() : $item_rows[$item_row_key];
  $item_id = empty($element_item_row['product_id']) ? NULL : (int)$element_item_row['product_id'];
  if (empty($item_id)) {
    form_error($element, t('Product not specified.'));
    return;
  }

  $element_parents = $element['#parents'];
  $default_item_rows = drupal_array_get_nested_value($form_state['complete form'], array_slice($element_parents, 0, count($element_parents) - 2));
  $total_product_qty = 0;
  foreach ($item_rows as $item_row_key => $item_row) {
    if (strtolower($item_row['product_title']) != strtolower($element_item_row['product_title'])) {
      continue;
    }

    // Get default qty if available.
    $item_row_default_value = empty($default_item_rows[$item_row_key]['qty']['#default_value']) ? 0 : $default_item_rows[$item_row_key]['qty']['#default_value'];
    $item_row_default_value = (int) $item_row_default_value;
    $item_row_default_qty_per_uom = empty($default_item_rows[$item_row_key]['qty_per_uom']['#default_value']) ? 1 : $default_item_rows[$item_row_key]['qty_per_uom']['#default_value'];
    $item_row_default_qty_per_uom = (int) $item_row_default_qty_per_uom;
    $item_row_default_base_qty = $item_row_default_value * $item_row_default_qty_per_uom;

    $qty_per_uom = empty($item_row['qty_per_uom']) ? 1 : (int)$item_row['qty_per_uom'];
    $qty = empty($item_row['qty']) ? 0 : (int)$item_row['qty'];
    $inputted_base_qty = $qty_per_uom * $qty;
    $total_product_qty += $inputted_base_qty - $item_row_default_base_qty;
  }

  $qty_details = avtxns_item_qty_details($item_id);
  $user_available_qty = (int) $qty_details['user_available'];
  $user_available_qty = $user_available_qty < 0 ? 0 : $user_available_qty;
  if ($user_available_qty < $total_product_qty) {
    form_error($element, t('@verb @qty1 %product but available quantity is only @qty2.', array(
      '@qty1' => (empty($form_state['values']['id']) ? '' : 'additional ') . number_format($total_product_qty),
      '@qty2' => number_format($user_available_qty),
      '%product' => $element_item_row['product_title'],
      '@verb' => ($transaction_type == AVTXNS_TXN_TYPE_PURCHASE_RETURN ? 'Returning' : 'Requesting'),
    )));
  }

  //$item_row = empty($item_rows[$item_row_key]) ? array() : $item_rows[$item_row_key];
  //$product_id = empty($item_row['product_id']) ? NULL : $item_row['product_id'];
  //$qty_per_uom = empty($item_row['qty_per_uom']) ? NULL : $item_row['qty_per_uom'];
  //if (!empty($qty_per_uom)) {
  //  $inputted_base_qty = $qty_per_uom * $element_value;
  //  //$current_qty = empty($product->qty) ? 0 : $product->qty;
  //
  //  $qty_details = avtxns_item_qty_details($product_id);
  //  $user_available_qty = (int) $qty_details['user_available'];
  //  if ($user_available_qty < $inputted_base_qty) {
  //    form_error($element, t('@verb @qty1 %product but available quantity is only @qty2.', array(
  //      '@qty1' => number_format($inputted_base_qty),
  //      '@qty2' => number_format($user_available_qty),
  //      '%product' => $item_row['product_title'],
  //      '@verb' => ($transaction_type == AVTXNS_TXN_TYPE_PURCHASE_RETURN ? 'Returning' : 'Requesting'),
  //    )));
  //  }
  //}
  //else {
  //  drupal_set_message('Validation against on-hand quantity failed.', 'warning');
  //}


}

/**
 * Return transaction type details.
 * @param $type
 *
 * @return mixed
 */
function avtxns_transaction_types($type) {
  $transaction_types = array(
    'po' => array(
      'title' => t('Purchase Order'),
      'base_path' => 'av/po',
      'id_prefix' => 'PO',
    ),
    'so' => array(
      'title' => t('Sales Order'),
      'base_path' => 'av/sales-order',
      'id_prefix' => 'SO',
    ),
    'gr' => array(
      'title' => t('Goods Received'),
      'base_path' => 'av/gr',
      'id_prefix' => 'GR',
    ),
    'dr' => array(
      'title' => t('Delivery'),
      'base_path' => 'av/delivery',
      'id_prefix' => 'DR',
    ),
    'ret' => array(
      'title' => t('Sales Return'),
      'base_path' => 'av/ret',
      'id_prefix' => 'RET',
    ),
    'rept' => array(
      'title' => t('Purchase Return'),
      'base_path' => 'av/rept',
      'id_prefix' => 'RET',
    ),
    'sinv' => array(
      'title' => t('Invoice'),
      'base_path' => 'av/sinv',
      'id_prefix' => 'INV',
    ),
    'spay' => array(
      'title' => t('Payment received'),
      'base_path' => 'av/spay',
      'id_prefix' => 'OR',
    ),
  );
  return $transaction_types[$type];
}

/**
 * Return return types.
 * @return array
 */
function avtxns_get_return_types() {
  return array(AVTXNS_RETURN_TYPE_RS => 'RS', AVTXNS_RETURN_TYPE_RUD => 'RUD');
}

/**
 * Ajax validation if entered qty is less than on-hand qty.
 *
 * @return array
 */
function avtxns_qty_check_and_reserve() {
  $data = array(
    //'post' => $_POST,
  );
  extract($_POST);

  $entered_item_id = (int)$entered_item_id;
  $entered_qty_per_uom = (int)$entered_qty_per_uom;
  $transaction_id = (int)$transaction_id;

  if (!empty($entered_item_id)) {
    $default_base_qty = 0;
    if (!empty($transaction_id)) {
      $transaction = avtxns_txn_load($transaction_id);
      if (!empty($transaction->id)) {
        $transaction_items = empty($transaction->items) ? array() : $transaction->items;
        foreach ($transaction_items as $ti) {
          if ($ti->item_id != $entered_item_id) {
            continue;
          }
          $default_qty = empty($ti->qty) ? 0 : $ti->qty;
          $default_qty_per_uom = empty($ti->qty_per_uom) ? 1 : $ti->qty_per_uom;
          $default_base_qty += ($default_qty * $default_qty_per_uom);
        }
      }
    }

    $qty_details = avtxns_item_qty_details($entered_item_id);
    $user_available = (int)$qty_details['user_available'];
    //$total_available = (int)$qty_details['total_available'];
    $entered_base_qty = (is_numeric($entered_qty) ? $entered_qty : 0) * $entered_qty_per_uom;
    if ($entered_base_qty <= $user_available) {
      avtxns_reserve_user_qty($entered_item_id, $entered_base_qty);
    }

    //$data['valid_qty'] = !empty($entered_qty_is_valid);
    $data['user_available'] = $user_available + $default_base_qty;
    //$data['total_available'] = $total_available;
  }

  print drupal_json_encode($data);
  drupal_exit();
}

/**
 * Get item on hand qty.
 * @param $item_id
 * @return array|NULL
 */
function avtxns_item_on_hand_qty($item_id) {
  if (empty($item_id)) {
    return NULL;
  }
  $data = &drupal_static(__FUNCTION__, array());
  if (!isset($data[$item_id])) {
    $data[$item_id] = db_query("SELECT qty FROM {avtbl_products} WHERE id = :id", array(':id' => $item_id))->fetchField();
  }
  return $data[$item_id];
}

/**
 * Get total qty reserved for this item.
 * @param $item_id
 * @return array|NULL
 */
function avtxns_user_reserved_qty($item_id) {
  global $user;
  if (empty($item_id) || empty($user->uid)) {
    return NULL;
  }

  $data = &drupal_static(__FUNCTION__, array());
  $now = time();
  if (!isset($data[$item_id][$user->uid])) {
    $user_reserved_qty = variable_get('avbase_user_reserved_qty', array());
    $total_reserved = $other_users_reserved = 0;
    $minutes = 5;
    foreach ($user_reserved_qty as $reserved_item_id => $locked_qty) {
      foreach ($locked_qty as $uid => $dtls) {
        if (empty($uid) || !isset($dtls['qty'])) {
          continue;
        }
        $timestamp = empty($dtls['timestamp']) ? 0 : $dtls['timestamp'];
        $qty = (int) $dtls['qty'];
        if (!empty($qty) && ($timestamp + (60 * $minutes) >= $now)) {
          if ($reserved_item_id == $item_id) {
            $other_users_reserved += ($user->uid && $user->uid == $uid) ? 0 : $dtls['qty'];
            $total_reserved += $dtls['qty'];
          }
        } else {
          unset($user_reserved_qty[$reserved_item_id][$uid]);
        }
      }
      if (empty($user_reserved_qty[$reserved_item_id])) {
        unset($user_reserved_qty[$reserved_item_id]);
      }
    }

    variable_set('avbase_user_reserved_qty', $user_reserved_qty);
    $data[$item_id][$user->uid] = array(
      'total_reserved' => $total_reserved,
      'other_users_reserved' => $other_users_reserved,
    );
  }
  return $data[$item_id][$user->uid];
}

/**
 * Reserve qty for user.
 * @param $item_id
 * @param $qty
 */
function avtxns_reserve_user_qty($item_id, $qty) {
  global $user;
  if (!is_numeric($qty) || empty($user->uid)) {
    return;
  }
  $user_reserved_qty = variable_get('avbase_user_reserved_qty', array());
  $user_reserved_qty[$item_id][$user->uid]['qty'] = $qty;
  // Reset timestamp of all reservations for this user.
  foreach ($user_reserved_qty as $k => $item_locks) {
    if (!isset($item_locks[$user->uid])) {
      continue;
    }
    $user_reserved_qty[$k][$user->uid]['timestamp'] = time();
  }

  variable_set('avbase_user_reserved_qty', $user_reserved_qty);
}

/**
 * Remove user's qty reservation for all items.
 */
function avtxns_remove_user_qty_reservation() {
  global $user;
  if (empty($user->uid)) {
    return;
  }

  $user_reserved_qty = variable_get('avbase_user_reserved_qty', array());
  foreach ($user_reserved_qty as $k => $item_locks) {
    unset($user_reserved_qty[$k][$user->uid]);
  }

  variable_set('avbase_user_reserved_qty', $user_reserved_qty);
}

/**
 * Get item qty details.
 * @param $item_id
 * @return int
 */
function avtxns_item_qty_details($item_id) {
  $on_hand_qty = (int) avtxns_item_on_hand_qty($item_id);
  $reservation = avtxns_user_reserved_qty($item_id);
  return array(
    'on_hand_qty' => $on_hand_qty,
    'user_available' => $on_hand_qty - $reservation['other_users_reserved'],
    'total_available' => $on_hand_qty - $reservation['total_reserved'],
  );
}

/**
 * Return form array containing notes on special discounts.
 * @param $term_days
 * @return array
 */
function avtxns_special_discount_form($term_days) {
  if (empty($term_days)) {
    return array();
  }

  $notes = array();
  for ($x = 1; $x <= 5; $x++) {
    $notes[$term_days * $x] = 0;
  }

  $notes_count = 0;
  $form = array('#theme' => 'avtxns_special_discount_form');
  foreach ($notes as $k => $balance) {
    $form[$k] = array(
      '#type' => 'item',
      '#title' => t('within @to days', array('@to' => $k)),
      '#title_display' => 'inline-before',
      '#markup' => '___________',
    );

    $notes_count++;
    if ($notes_count == count($notes)) {
      $form[$k]['#title'] = t('after @from days', array('@from' => $k - $term_days));
    }
  }
  return $form;
}

/**
 * Return transaction status details.
 * @param $status
 *
 * @return array
 */
function avtxns_txn_status_details($status = NULL) {
  $statuses = array(
    AVTXNS_TXN_STATUS_CLOSED => array(
      'status' => AVTXNS_TXN_STATUS_CLOSED,
      'title' => t('Locked'),
    ),
    AVTXNS_TXN_STATUS_OPEN => array(
      'status' => AVTXNS_TXN_STATUS_OPEN,
      'title' => t('Open'),
    ),
    AVTXNS_TXN_STATUS_PENDING => array(
      'status' => AVTXNS_TXN_STATUS_PENDING,
      'title' => t('Pending Approval'),
    ),
    AVTXNS_TXN_STATUS_VOID => array(
      'status' => AVTXNS_TXN_STATUS_VOID,
      'title' => t('Void'),
    ),
  );

  if (isset($statuses[$status])) {
    return $statuses[$status];
  }
  else {
    return $statuses;
  }
}

/**
 * Change transaction status.
 * @param $id
 * @param $status
 *
 * @return bool
 */
function avtxns_txn_change_status($id, $status) {
  if (empty($id) || !is_numeric($id)) {
    return FALSE;
  }

  $row = avtxns_txn_load($id);
  if (empty($row->id)) {
    drupal_set_message('Transaction not found.', 'error');
    return FALSE;
  }
  $tsd = avtxns_txn_status_details($status);
  if (!isset($tsd['status'])) {
    drupal_set_message('Unknown transaction status.', 'error');
    return FALSE;
  }
  if ($status == $row->status) {
    drupal_set_message(t('Transaction is already @status', array('@status' => $tsd['title'])), 'error');
    return FALSE;
  }

  switch ($row->status) {
    case AVTXNS_TXN_STATUS_CLOSED:
    case AVTXNS_TXN_STATUS_VOID:
      drupal_set_message('Status of Closed and Void transactions cannot be changed.', 'error');
      return FALSE;
      break;
    case AVTXNS_TXN_STATUS_OPEN:
      if (!in_array($status, array(AVTXNS_TXN_STATUS_PENDING, AVTXNS_TXN_STATUS_VOID))) {
        drupal_set_message(t('Cannot change transaction to %status', array('%status' => $tsd['title'])), 'error');
        return FALSE;
      }
      break;
    case AVTXNS_TXN_STATUS_PENDING:
      if (!in_array($status, array(AVTXNS_TXN_STATUS_OPEN, AVTXNS_TXN_STATUS_VOID))) {
        drupal_set_message(t('Cannot change transaction to %status', array('%status' => $tsd['title'])), 'error');
        return FALSE;
      }
      break;
  }

  $ttd = avtxns_txn_types($row->transaction_type);
  if ($status == AVTXNS_TXN_STATUS_VOID) {
    // Re-open related transactions if possible.
    $item_rows = empty($row->items) ? array() : $row->items;
    $related_transaction_ids = array();
    foreach ($item_rows as $item_row) {
      $ref_txn_id = empty($item_row->ref_txn_id) ? NULL : $item_row->ref_txn_id;
      if ($ref_txn_id) {
        $related_transaction_ids[$ref_txn_id] = $ref_txn_id;
      }
    }
    // Check if each related transaction IDs can be re-opened.
    if (!empty($related_transaction_ids)) {
      $query = "SELECT avtxn_details.ref_txn_id, COUNT(avtxn_details.id) AS active_txn_count
        FROM {avtbl_transaction_details} avtxn_details
        INNER JOIN {avtbl_transactions} avtxns ON avtxns.id = avtxn_details.transaction_id
        WHERE avtxn_details.ref_txn_id IN (:rel_txn_ids) AND avtxn_details.transaction_id <> :txn_id AND avtxns.status <> :txn_status
        GROUP BY avtxn_details.ref_txn_id";
      $rs = db_query($query, array(':txn_id' => $row->id, ':txn_status' => AVTXNS_TXN_STATUS_VOID, ':rel_txn_ids' => $related_transaction_ids));
      $cannot_open = array();
      foreach ($rs as $rel_row) {
        $cannot_open[] = $rel_row->ref_txn_id;
      }

      $can_be_reopened = array_diff($related_transaction_ids, $cannot_open);
    }

    // Refresh product quantities.
    avbase_product_qty_update($item_rows, $row, TRUE);
  }

  $now = time();
  if (db_query("UPDATE {avtbl_transactions} SET changed = :changed, status = :status WHERE id = :txn_id", array(':txn_id' => $row->id, ':status' => $status, ':changed' => $now))) {
    avbase_custom_watchdog("txn__status_change", "ID: {$row->id}", array('#status' => $status, '#row' => $row));
    // Reopen transactions.
    if (!empty($can_be_reopened)) {
      if (db_query("UPDATE {avtbl_transactions} SET changed = :changed, status = :status WHERE id IN (:txn_ids)", array(':txn_ids' => $can_be_reopened, ':status' => AVTXNS_TXN_STATUS_OPEN, ':changed' => $now))) {
        avbase_custom_watchdog("txn__status_change", "Reopened, IDs: multiple", array('#ids' => $can_be_reopened, '#status' => AVTXNS_TXN_STATUS_OPEN, '#row' => $row));
      }
    }
    return TRUE;
  }
  return FALSE;
}

/**
 * Return transaction type list as options array.
 */
function avtxns_txn_types_options_array() {
  $options = array();
  $types = avtxns_txn_types();
  foreach ($types as $t_k => $t) {
    $options[$t_k] = $t['name'];
  }
  return $options;
}

/**
 * Return transaction type list as options array.
 */
function avtxns_txn_status_options_array() {
  $options = array();
  $statuses = avtxns_txn_status_details();
  foreach ($statuses as $s_k => $s) {
    $options[$s_k] = $s['title'];
  }
  return $options;
}
