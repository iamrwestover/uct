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
  $select[] = 'avvendors.display_name AS client_name';


  $query = db_select('avtbl_transactions', 'avtxns');
  $query->leftJoin('avtbl_vendors', 'avvendors', 'avvendors.id = avtxns.client_id');
  $query->leftJoin('avtbl_categories', 'avcat_agent', "avcat_agent.id = avtxns.agent_id AND avcat_agent.group_id = 'agent_vend'");
  $query->addField('avvendors', 'display_name', 'client_name');
  $query->addField('avcat_agent', 'title', 'agent_name');
  $query->fields('avtxns');
  //->fields('avvendors', array('display_name'))
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

  $query->orderBy('transaction_date', 'DESC');
  $rs = $query->execute();
  $rows = array();
  foreach ($rs as $row) {
    $subtotal = 0;
    if (!empty($row->id)) {
      $query = db_select('avtbl_transaction_details', 'avtxn_items');
      $query->leftJoin('avtbl_products', 'avprod', 'avprod.id = avtxn_items.product_id');
      $query->leftJoin('avtbl_categories', 'avcat', "avcat.id = avtxn_items.uom_id AND avcat.group_id = 'uom'");
      $query->leftJoin('avtbl_categories', 'avcat_base', "avcat_base.id = avprod.uom_id AND avcat.group_id = 'uom'");
      $query->addField('avprod', 'title', 'product_title');
      $query->addField('avprod', 'uom_id', 'base_uom_id');
      $query->addField('avcat', 'title', 'uom_title');
      $query->addField('avcat_base', 'title', 'base_uom_title');
      $query->fields('avtxn_items')
        ->condition('avtxn_items.po_id', $row->id)
        ->execute();
      $rs = $query->execute();
      foreach ($rs as $item) {
        $item->total = $item->cost * $item->qty;
        $discount = empty($item->discount) ? array() : explode('/', $item->discount);
        foreach ($discount as $v) {
          if (is_numeric($v)) {
            $v = (float) $v;
            $item->total = $item->total - (($item->total * $v) / 100);
          }
        }
        $item->qty_per_uom = empty($item->qty_per_uom) ? 1 : $item->qty_per_uom;
        $item->incoming_base_qty = $item->qty * $item->qty_per_uom;
        $item->remaining_base_qty = $item->incoming_base_qty;
        $row->items[$item->id] = $item;
        $subtotal += $item->total;
      }
    }
    $row->discount_value_text = $row->discount_type == AVBASE_DISCOUNT_PERCENT ? (round($row->discount_value, 2) . '%') : number_format($row->discount_value, 2);
    $row->subtotal = $subtotal;
    $row->discounted_value = $row->discount_type == AVBASE_DISCOUNT_PERCENT ? (($row->discount_value / 100) * $subtotal) : $row->discount_value;
    $row->grand_total = $subtotal - $row->discounted_value;

    // Attach received items.
    avtxns_txn_attach_received_items($row);

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
 * @param $po_id
 * @param string $email
 *
 * @return bool
 */
function avtxns_send_txn_to_email($po_id, $email = '') {
  if (empty($po_id)) {
    return FALSE;
  }

  $po = avtxns_txn_load($po_id);
  $email = $po->email;
  if ($error = user_validate_mail($email)) {
    drupal_set_message($error, 'error');
    return FALSE;
  }

  $po->discount_value = round($po->discount_value, 2);
  $params = array(
    'po' => $po,
    'po_num' => $po->id,
    'transaction_date' => format_date($po->created, 'custom', 'F d, Y'),
    'site_name' => variable_get('site_name', 'UCT'),
    'shipping_address' => variable_get('av_company_address', '-'),
    'vendor_name' => check_plain($po->client_name),
    //'payment_term_title' => check_plain($po->payment_term_title),
  );

  $params['po_table'] = theme('avtxns_txn_mail_table', $params);
  $message = drupal_mail('avtrans', 'po', $email, language_default(), $params);
  if (!empty($message['result'])) {
    drupal_set_message(t('Purchase order sent successfully to %mail.', array('%mail' => $email)));
    avbase_custom_watchdog("crud_mail_po", "Sent Purchase Order #@po_id to @vendor_name",
      array('@po_id' => $po->id, '@vendor_name' => $po->client_name , '#message' => $message)
    );
    return TRUE;
  }
  return FALSE;
}

/**
 * Advanced transaction load.
 * @return array
 */
//function avtxns_txns_query_load($param_where = array(), $param_args = array(), $limit_string = NULL, $order_by_string = NULL) {
//  // Load vendors static variable.
//  //$avbase = &drupal_static('avbase', array());
//  //$avbase['vendors'] = empty($avbase['vendors']) ? array() : $avbase['vendors'];
//  //$_vendors = &$avbase['vendors'];
//
//  // Set / load static variable for this query.
//  $data = &drupal_static(__FUNCTION__, array());
//  $_data_id = $param_where;
//  sort($_data_id);
//  $data_id = md5(json_encode($_data_id) . json_encode($param_args) . json_encode($limit_string));
//
//  if (isset($data[$data_id])) {
//    return $data[$data_id];
//  }
//
//  $transaction_types = empty($param_where['transaction_types']) ? array() : $param_where['transaction_types'];
//  unset($param_where['transaction_types']);
//
//  $queries = array();
//  $queries['po'] = "SELECT
//    avtxns.id AS transaction_id,
//    avtxns.status,
//    'po' AS transaction_type,
//    avcat_agent.title AS agent_name,
//    avcat_term.title AS term_name,
//    avtxns.transaction_date,
//    avclient.display_name,
//    SUM((avtxn_items.qty * avtxn_items.qty_per_uom) * avtxn_items.cost) AS grand_total
//    FROM {avtbl_po} AS avtxns
//    INNER JOIN {avtbl_vendors} AS avclient ON avclient.id = avtxns.client_id
//    LEFT JOIN {avtbl_po_items} AS avtxn_items ON avtxn_items.po_id = avtxns.id
//    LEFT JOIN {avtbl_categories} AS avcat_agent ON avcat_agent.id = avtxns.agent_id AND avcat_agent.group_id = 'agent_vend'
//    LEFT JOIN {avtbl_categories} AS avcat_term ON avcat_term.id = avtxns.term_id AND avcat_term.group_id = 'payment_term'
//  ";
//  $queries['so'] = "SELECT
//    avtxns.id AS transaction_id,
//    avtxns.status,
//    'so' AS transaction_type,
//    avcat_agent.title AS agent_name,
//    avcat_term.title AS term_name,
//    avtxns.transaction_date,
//    avclient.display_name,
//    SUM((avtxn_items.qty * avtxn_items.qty_per_uom) * avtxn_items.cost) AS grand_total
//    FROM {avtbl_so} AS avtxns
//    INNER JOIN {avtbl_customers} AS avclient ON avclient.id = avtxns.client_id
//    LEFT JOIN {avtbl_so_items} AS avtxn_items ON avtxn_items.so_id = avtxns.id
//    LEFT JOIN {avtbl_categories} AS avcat_agent ON avcat_agent.id = avtxns.agent_id AND avcat_agent.group_id = 'agent_cust'
//    LEFT JOIN {avtbl_categories} AS avcat_term ON avcat_term.id = avtxns.term_id AND avcat_term.group_id = 'payment_term'
//  ";
//  $queries['gr'] = "SELECT
//    avtxns.id AS transaction_id,
//    avtxns.status,
//    'gr' AS transaction_type,
//    avcat_agent.title AS agent_name,
//    avcat_term.title AS term_name,
//    avtxns.transaction_date,
//    avclient.display_name,
//    SUM((avtxn_items.qty * avtxn_items.qty_per_uom) * avtxn_items.cost) AS grand_total
//    FROM {avtbl_gr} AS avtxns
//    INNER JOIN {avtbl_vendors} AS avclient ON avclient.id = avtxns.client_id
//    LEFT JOIN {avtbl_gr_items} AS avtxn_items ON avtxn_items.gr_id = avtxns.id
//    LEFT JOIN {avtbl_categories} AS avcat_agent ON avcat_agent.id = avtxns.agent_id AND avcat_agent.group_id = 'agent_vend'
//    LEFT JOIN {avtbl_categories} AS avcat_term ON avcat_term.id = avtxns.term_id AND avcat_term.group_id = 'payment_term'
//  ";
//  $queries['dr'] = "SELECT
//    avtxns.id AS transaction_id,
//    avtxns.status,
//    'dr' AS transaction_type,
//    avcat_agent.title AS agent_name,
//    avcat_term.title AS term_name,
//    avtxns.transaction_date,
//    avclient.display_name,
//    SUM((avtxn_items.qty * avtxn_items.qty_per_uom) * avtxn_items.cost) AS grand_total
//    FROM {avtbl_dr} AS avtxns
//    INNER JOIN {avtbl_customers} AS avclient ON avclient.id = avtxns.client_id
//    LEFT JOIN {avtbl_dr_items} AS avtxn_items ON avtxn_items.dr_id = avtxns.id
//    LEFT JOIN {avtbl_categories} AS avcat_agent ON avcat_agent.id = avtxns.agent_id AND avcat_agent.group_id = 'agent_cust'
//    LEFT JOIN {avtbl_categories} AS avcat_term ON avcat_term.id = avtxns.term_id AND avcat_term.group_id = 'payment_term'
//  ";
//  $queries['ret'] = "SELECT
//    avtxns.id AS transaction_id,
//    avtxns.status,
//    'ret' AS transaction_type,
//    avcat_agent.title AS agent_name,
//    avcat_term.title AS term_name,
//    avtxns.transaction_date,
//    avclient.display_name,
//    SUM((avtxn_items.qty * avtxn_items.qty_per_uom) * avtxn_items.cost) AS grand_total
//    FROM {avtbl_ret} AS avtxns
//    INNER JOIN {avtbl_customers} AS avclient ON avclient.id = avtxns.client_id
//    LEFT JOIN {avtbl_ret_items} AS avtxn_items ON avtxn_items.ret_id = avtxns.id
//    LEFT JOIN {avtbl_categories} AS avcat_agent ON avcat_agent.id = avtxns.agent_id AND avcat_agent.group_id = 'agent_cust'
//    LEFT JOIN {avtbl_categories} AS avcat_term ON avcat_term.id = avtxns.term_id AND avcat_term.group_id = 'payment_term'
//  ";
//  $queries['rept'] = "SELECT
//    avtxns.id AS transaction_id,
//    avtxns.status,
//    'rept' AS transaction_type,
//    avcat_agent.title AS agent_name,
//    avcat_term.title AS term_name,
//    avtxns.transaction_date,
//    avclient.display_name,
//    SUM((avtxn_items.qty * avtxn_items.qty_per_uom) * avtxn_items.cost) AS grand_total
//    FROM {avtbl_rept} AS avtxns
//    INNER JOIN {avtbl_vendors} AS avclient ON avclient.id = avtxns.client_id
//    LEFT JOIN {avtbl_rept_items} AS avtxn_items ON avtxn_items.rept_id = avtxns.id
//    LEFT JOIN {avtbl_categories} AS avcat_agent ON avcat_agent.id = avtxns.agent_id AND avcat_agent.group_id = 'agent_vend'
//    LEFT JOIN {avtbl_categories} AS avcat_term ON avcat_term.id = avtxns.term_id AND avcat_term.group_id = 'payment_term'
//  ";
//  $queries['sinv'] = "SELECT
//    avtxns.id AS transaction_id,
//    avtxns.status,
//    'sinv' AS transaction_type,
//    avcat_agent.title AS agent_name,
//    avcat_term.title AS term_name,
//    avtxns.transaction_date,
//    avclient.display_name,
//    SUM((avtxn_items.qty * avtxn_items.qty_per_uom) * avtxn_items.cost) AS grand_total
//    FROM {avtbl_sinv} AS avtxns
//    INNER JOIN {avtbl_customers} AS avclient ON avclient.id = avtxns.client_id
//    LEFT JOIN {avtbl_sinv_items} AS avtxn_items ON avtxn_items.sinv_id = avtxns.id
//    LEFT JOIN {avtbl_categories} AS avcat_agent ON avcat_agent.id = avtxns.agent_id AND avcat_agent.group_id = 'agent_cust'
//    LEFT JOIN {avtbl_categories} AS avcat_term ON avcat_term.id = avtxns.term_id AND avcat_term.group_id = 'payment_term'
//  ";
//  $queries['spay'] = "SELECT
//    avtxns.id AS transaction_id,
//    avtxns.status,
//    'spay' AS transaction_type,
//    avcat_agent.title AS agent_name,
//    avcat_term.title AS term_name,
//    avtxns.transaction_date,
//    avclient.display_name,
//    SUM(avtxn_items.paid_amount) AS grand_total
//    FROM {avtbl_spay} AS avtxns
//    INNER JOIN {avtbl_customers} AS avclient ON avclient.id = avtxns.client_id
//    LEFT JOIN {avtbl_spay_items} AS avtxn_items ON avtxn_items.spay_id = avtxns.id
//    LEFT JOIN {avtbl_categories} AS avcat_agent ON avcat_agent.id = avtxns.agent_id AND avcat_agent.group_id = 'agent_cust'
//    LEFT JOIN {avtbl_categories} AS avcat_term ON avcat_term.id = avtxns.term_id AND avcat_term.group_id = 'payment_term'
//  ";
//
//  // Only include queries for specified transaction types.
//  if (!empty($transaction_types)) {
//    $queries = array_intersect_key($queries, array_combine($transaction_types, $transaction_types));
//  }
//  //dpm($param_args);
//  //dpm($param_where);
//  //$param_where[] = "avcat_agent.title = :SSS";
//  //$param_args[':SSS'] = "sss";
//  foreach ($queries as $k => $v) {
//    if (!empty($param_where)) {
//      $queries[$k] .= " WHERE " . implode(' AND ', $param_where);
//    }
//    $queries[$k] .= " GROUP BY avtxns.id";
//  }
//
//  $query = implode(' UNION ', $queries);
//  if (is_null($order_by_string)) {
//    $query .= " ORDER BY transaction_date DESC";
//  }
//  else {
//    $query .= " $order_by_string";
//  }
//  if (!is_null($limit_string)) {
//    $query .= " LIMIT " . $limit_string;
//  }
//
//
//  $rs = db_query($query, $param_args);
//  $rows = array();
//  foreach ($rs as $row) {
//    $rows[] = $row;
//  }
//
//
//  $data[$data_id] = $rows;
//  return $rows;
//}

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
function avtxns_transaction_form_ajax($form, $form_state) {
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
 * @param $form
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

/**
 * Redirect transaction form.
 * @param $form
 * @param $form_state
 */
function avtxns_redirect_transaction_form($form, &$form_state) {
  $triggering_element = isset($form_state['triggering_element']) ? $form_state['triggering_element'] : array();
  $transaction_id = empty($form_state['values']['id']) ? NULL : $form_state['values']['id'];
  $redirect_query = array();
  if ($triggering_element['#id'] == 'submit_and_print') {
    $redirect_query['print'] = 1;
  }
  $form_state['redirect'] = array(arg(0) . '/' . arg(1) . '/' . $transaction_id . '/view', array('query' => $redirect_query));
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

/**
 * Transaction buttons.
 * @param $transaction_type
 * @param $view_mode
 *
 * @return array
 */
function avtxns_transaction_buttons($transaction_type, $view_mode) {
  $buttons = array('#theme' => 'avbase_crud_button_group');
  $buttons['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#icon_key' => 'save',
  );
  if ($transaction_type == 'po') {
    $buttons['submit_and_send'] = array(
      '#id' => 'submit_and_send',
      '#type' => 'submit',
      '#value' => t('Save and send'),
      '#icon_key' => 'mail-forward',
    );
  }

  $buttons['submit_and_print'] = array(
    '#id' => 'submit_and_print',
    '#type' => 'submit',
    '#value' => t('Save and print'),
    '#icon_key' => 'print',
  );
  if (empty($view_mode)) {
    $buttons['cancel'] = array(
      '#markup' => l('Cancel', 'av/txns', array('attributes' => array('class' => array('uk-button')))),
    );
  }
  else {
    $buttons['print'] = array(
      '#id' => 'av-print-btn',
      '#type' => 'button',
      '#value' => t('Print...'),
      '#visible_in_view_mode' => TRUE,
      '#attributes' => array('class' => array('suk-button-primary')),
      '#icon_key' => 'print',
    );
  }
  return $buttons;
}