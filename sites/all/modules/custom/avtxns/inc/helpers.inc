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
  if (empty($conditions)) {
    return array();
  }

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

  $select = array();
  $select[] = 'avtxns.*';
  $select[] = 'avclients.display_name AS client_name';


  $query = db_select('avtbl_transactions', 'avtxns');
  $query->leftJoin('avtbl_clients', 'avclients', 'avclients.id = avtxns.client_id');
  $query->leftJoin('avtbl_categories', 'avcat_agent', "avcat_agent.id = avtxns.agent_id AND avcat_agent.group_id = CONCAT('agent', avclients.client_type)");
  $query->addField('avclients', 'display_name', 'client_name');
  $query->addField('avcat_agent', 'title', 'agent_name');
  $query->fields('avtxns');
  //->fields('avclients', array('display_name'))
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
      $query = db_select('avtbl_transaction_details', 'avtxn_details');
      if ($row->transaction_type != AVTXNS_TXN_TYPE_PAYMENT) {
        $query->leftJoin('avtbl_products', 'avprod', 'avprod.id = avtxn_details.item_id');
        $query->leftJoin('avtbl_categories', 'avcat', "avcat.id = avtxn_details.uom_id AND avcat.group_id = 'uom'");
        $query->leftJoin('avtbl_categories', 'avcat_base', "avcat_base.id = avprod.uom_id AND avcat.group_id = 'uom'");
        $query->leftJoin('avtbl_transaction_details', 'avtxn_reference', 'avtxn_reference.id = avtxn_details.ref_txn_dtl_id');
        $query->leftJoin('avtbl_transactions', 'avtxn', 'avtxn.id = avtxn_reference.transaction_id');
        $query->addField('avprod', 'title', 'product_title');
        $query->addField('avprod', 'uom_id', 'base_uom_id');
        $query->addField('avcat', 'title', 'uom_title');
        $query->addField('avcat_base', 'title', 'base_uom_title');
        $query->addField('avtxn_reference', 'transaction_id', 'ref_txn_id');
        $query->addField('avtxn', 'transaction_type', 'ref_txn_type');
      }
      $query->fields('avtxn_details')->condition('avtxn_details.transaction_id', $row->id)->execute();
      $details_rs = $query->execute();
      foreach ($details_rs as $item) {
        $item->total = $item->cost * $item->qty;
        $discount = array();
        for ($x = 1; $x <= 4; $x++) {
          $_discount_name = "discount$x";
          if (isset($item->$_discount_name)) {
            $discount[] = $item->$_discount_name;
          }
        }
        $item->discount_text = implode('/', $discount);
        foreach ($discount as $v) {
          if (is_numeric($v)) {
            $v = (float) $v;
            $item->total = $item->total - (($item->total * $v) / 100);
          }
        }

        if (isset($item->ref_txn_type)) {
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
        $subtotal += $item->total;
      }
    }
    //$row->discount_value_text = $row->discount_type == AVBASE_DISCOUNT_PERCENT ? (round($row->discount_value, 2) . '%') : number_format($row->discount_value, 2);
    //$row->subtotal = $subtotal;
    //$row->discounted_value = $row->discount_type == AVBASE_DISCOUNT_PERCENT ? (($row->discount_value / 100) * $subtotal) : $row->discount_value;
    //$row->grand_total = $subtotal - $row->discounted_value;
    $row->grand_total = $subtotal;
    $row->credit_total = $credit_total;
    $row->debit_total = $debit_total;

    // Attach received items.
    //avtxns_txn_attach_received_items($row);

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
 * @return array
 */
function avtxns_txns_query_load($param_where = array(), $param_args = array(), $limit_string = NULL, $order_by_string = NULL) {
  // Load vendors static variable.
  //$avbase = &drupal_static('avbase', array());
  //$avbase['vendors'] = empty($avbase['vendors']) ? array() : $avbase['vendors'];
  //$_vendors = &$avbase['vendors'];

  // Set / load static variable for this query.
  $data = &drupal_static(__FUNCTION__, array());
  $_data_id = $param_where;
  sort($_data_id);
  $data_id = md5(json_encode($_data_id) . json_encode($param_args) . json_encode($limit_string));

  if (isset($data[$data_id])) {
    return $data[$data_id];
  }

  //$transaction_types = empty($param_where['transaction_types']) ? array() : $param_where['transaction_types'];
  //unset($param_where['transaction_types']);

  //$queries = array();
  $query = "SELECT
    avtxn.id,
    avtxn.status,
    avtxn.transaction_type,
    avtxn.transaction_date,
    avcat_agent.title AS agent_name,
    avcat_term.title AS term_name,
    avclient.display_name
    FROM {avtbl_transactions} AS avtxn
    INNER JOIN {avtbl_clients} AS avclient ON avclient.id = avtxn.client_id
    LEFT JOIN {avtbl_transaction_details} AS avtxn_detail ON avtxn_detail.transaction_id = avtxn.id
    LEFT JOIN {avtbl_categories} AS avcat_agent ON avcat_agent.id = avtxn.agent_id
    LEFT JOIN {avtbl_categories} AS avcat_term ON avcat_term.id = avtxn.term_id
  ";

  // Only include queries for specified transaction types.
  //if (!empty($transaction_types)) {
  //  $queries = array_intersect_key($queries, array_combine($transaction_types, $transaction_types));
  //}
  //dpm($param_args);
  //dpm($param_where);
  //$param_where[] = "avcat_agent.title = :SSS";
  //$param_args[':SSS'] = "sss";
  //foreach ($queries as $k => $v) {
    if (!empty($param_where)) {
      $query .= " WHERE " . implode(' AND ', $param_where);
    }
    $query .= " GROUP BY avtxn.id";
  //}

  //$query = implode(' UNION ', $queries);
  if (is_null($order_by_string)) {
    $query .= " ORDER BY avtxn.transaction_date DESC, avtxn.id DESC";
  }
  else {
    $query .= " $order_by_string";
  }
  if (!is_null($limit_string)) {
    $query .= " LIMIT " . $limit_string;
  }


  $rs = db_query($query, $param_args);
  $rows = array();
  foreach ($rs as $row) {
    $rows[] = $row;
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
  $form_state['storage'] = isset($form_state['storage']) ? $form_state['storage'] : array();
  $_storage = &$form_state['storage'];
  $transaction_type = empty($_storage['transaction_type']) ? '' : $_storage['transaction_type'];

  // Get item rows.
  $item_rows = empty($form_state['values']['product_rows']) ? array() : $form_state['values']['product_rows'];
  $item_row = empty($item_rows[$item_row_key]) ? array() : $item_rows[$item_row_key];
  $product_id = empty($item_row['product_id']) ? NULL : $item_row['product_id'];
  $qty_per_uom = empty($item_row['qty_per_uom']) ? NULL : $item_row['qty_per_uom'];
  if (!empty($qty_per_uom)) {
    $product = avproduct_load($product_id);
    if (!empty($product)) {
      $inputted_base_qty = $qty_per_uom * $element_value;
      $current_qty = empty($product->qty) ? 0 : $product->qty;
      if ($current_qty < $inputted_base_qty) {
        form_error($element, t('@verb %qty1 %product but on-hand quantity is only %qty2.', array(
          '%qty1' => number_format($inputted_base_qty),
          '%qty2' => number_format($current_qty),
          '%product' => $product->title,
          '@verb' => ($transaction_type == 'rept' ? 'Returning' : 'Requesting'),
        )));
      }
    }
  }
  else {
    drupal_set_message('Validation against on-hand quantity failed.', 'warning');
  }


}

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
