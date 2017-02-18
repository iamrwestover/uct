<?php
module_load_include('inc', 'avtrans', 'inc/po.helpers');
module_load_include('inc', 'avtrans', 'inc/so.helpers');
module_load_include('inc', 'avtrans', 'inc/gr/gr.helpers');
module_load_include('inc', 'avtrans', 'inc/delivery/delivery.helpers');
module_load_include('inc', 'avtrans', 'inc/ret/ret.helpers');
module_load_include('inc', 'avtrans', 'inc/rept/rept.helpers');
module_load_include('inc', 'avtrans', 'inc/sinv/sinv.helpers');
module_load_include('inc', 'avtrans', 'inc/spay/spay.helpers');

/**
 * Advanced transaction load.
 * @return array
 */
function avtrans_transactions_query_load($param_where = array(), $param_args = array(), $limit_string = NULL, $order_by_string = NULL) {
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

  $transaction_types = empty($param_where['transaction_types']) ? array() : $param_where['transaction_types'];
  unset($param_where['transaction_types']);

  $queries = array();
  $queries['po'] = "SELECT
    avtrans.id AS transaction_id,
    avtrans.status,
    'po' AS transaction_type,
    avcat_agent.title AS agent_name,
    avcat_term.title AS term_name,
    avtrans.transaction_date,
    avclients.display_name,
    SUM((avtrans_items.qty * avtrans_items.qty_per_uom) * avtrans_items.cost) AS grand_total
    FROM {avtbl_po} AS avtrans
    INNER JOIN {avtbl_vendors} AS avclients ON avclients.id = avtrans.client_id
    LEFT JOIN {avtbl_po_items} AS avtrans_items ON avtrans_items.po_id = avtrans.id
    LEFT JOIN {avtbl_categories} AS avcat_agent ON avcat_agent.id = avtrans.agent_id AND avcat_agent.group_id = 'agent_vend'
    LEFT JOIN {avtbl_categories} AS avcat_term ON avcat_term.id = avtrans.term_id AND avcat_term.group_id = 'payment_term'
  ";
  $queries['so'] = "SELECT
    avtrans.id AS transaction_id,
    avtrans.status,
    'so' AS transaction_type,
    avcat_agent.title AS agent_name,
    avcat_term.title AS term_name,
    avtrans.transaction_date,
    avclients.display_name,
    SUM((avtrans_items.qty * avtrans_items.qty_per_uom) * avtrans_items.cost) AS grand_total
    FROM {avtbl_so} AS avtrans
    INNER JOIN {avtbl_customers} AS avclients ON avclients.id = avtrans.client_id
    LEFT JOIN {avtbl_so_items} AS avtrans_items ON avtrans_items.so_id = avtrans.id
    LEFT JOIN {avtbl_categories} AS avcat_agent ON avcat_agent.id = avtrans.agent_id AND avcat_agent.group_id = 'agent_cust'
    LEFT JOIN {avtbl_categories} AS avcat_term ON avcat_term.id = avtrans.term_id AND avcat_term.group_id = 'payment_term'
  ";
  $queries['gr'] = "SELECT
    avtrans.id AS transaction_id,
    avtrans.status,
    'gr' AS transaction_type,
    avcat_agent.title AS agent_name,
    avcat_term.title AS term_name,
    avtrans.transaction_date,
    avclients.display_name,
    SUM((avtrans_items.qty * avtrans_items.qty_per_uom) * avtrans_items.cost) AS grand_total
    FROM {avtbl_gr} AS avtrans
    INNER JOIN {avtbl_vendors} AS avclients ON avclients.id = avtrans.client_id
    LEFT JOIN {avtbl_gr_items} AS avtrans_items ON avtrans_items.gr_id = avtrans.id
    LEFT JOIN {avtbl_categories} AS avcat_agent ON avcat_agent.id = avtrans.agent_id AND avcat_agent.group_id = 'agent_vend'
    LEFT JOIN {avtbl_categories} AS avcat_term ON avcat_term.id = avtrans.term_id AND avcat_term.group_id = 'payment_term'
  ";
  $queries['dr'] = "SELECT
    avtrans.id AS transaction_id,
    avtrans.status,
    'dr' AS transaction_type,
    avcat_agent.title AS agent_name,
    avcat_term.title AS term_name,
    avtrans.transaction_date,
    avclients.display_name,
    SUM((avtrans_items.qty * avtrans_items.qty_per_uom) * avtrans_items.cost) AS grand_total
    FROM {avtbl_dr} AS avtrans
    INNER JOIN {avtbl_customers} AS avclients ON avclients.id = avtrans.client_id
    LEFT JOIN {avtbl_dr_items} AS avtrans_items ON avtrans_items.dr_id = avtrans.id
    LEFT JOIN {avtbl_categories} AS avcat_agent ON avcat_agent.id = avtrans.agent_id AND avcat_agent.group_id = 'agent_cust'
    LEFT JOIN {avtbl_categories} AS avcat_term ON avcat_term.id = avtrans.term_id AND avcat_term.group_id = 'payment_term'
  ";
  $queries['ret'] = "SELECT
    avtrans.id AS transaction_id,
    avtrans.status,
    'ret' AS transaction_type,
    avcat_agent.title AS agent_name,
    avcat_term.title AS term_name,
    avtrans.transaction_date,
    avclients.display_name,
    SUM((avtrans_items.qty * avtrans_items.qty_per_uom) * avtrans_items.cost) AS grand_total
    FROM {avtbl_ret} AS avtrans
    INNER JOIN {avtbl_customers} AS avclients ON avclients.id = avtrans.client_id
    LEFT JOIN {avtbl_ret_items} AS avtrans_items ON avtrans_items.ret_id = avtrans.id
    LEFT JOIN {avtbl_categories} AS avcat_agent ON avcat_agent.id = avtrans.agent_id AND avcat_agent.group_id = 'agent_cust'
    LEFT JOIN {avtbl_categories} AS avcat_term ON avcat_term.id = avtrans.term_id AND avcat_term.group_id = 'payment_term'
  ";
  $queries['rept'] = "SELECT
    avtrans.id AS transaction_id,
    avtrans.status,
    'rept' AS transaction_type,
    avcat_agent.title AS agent_name,
    avcat_term.title AS term_name,
    avtrans.transaction_date,
    avclients.display_name,
    SUM((avtrans_items.qty * avtrans_items.qty_per_uom) * avtrans_items.cost) AS grand_total
    FROM {avtbl_rept} AS avtrans
    INNER JOIN {avtbl_vendors} AS avclients ON avclients.id = avtrans.client_id
    LEFT JOIN {avtbl_rept_items} AS avtrans_items ON avtrans_items.rept_id = avtrans.id
    LEFT JOIN {avtbl_categories} AS avcat_agent ON avcat_agent.id = avtrans.agent_id AND avcat_agent.group_id = 'agent_vend'
    LEFT JOIN {avtbl_categories} AS avcat_term ON avcat_term.id = avtrans.term_id AND avcat_term.group_id = 'payment_term'
  ";
  $queries['sinv'] = "SELECT
    avtrans.id AS transaction_id,
    avtrans.status,
    'sinv' AS transaction_type,
    avcat_agent.title AS agent_name,
    avcat_term.title AS term_name,
    avtrans.transaction_date,
    avclients.display_name,
    SUM((avtrans_items.qty * avtrans_items.qty_per_uom) * avtrans_items.cost) AS grand_total
    FROM {avtbl_sinv} AS avtrans
    INNER JOIN {avtbl_customers} AS avclients ON avclients.id = avtrans.client_id
    LEFT JOIN {avtbl_sinv_items} AS avtrans_items ON avtrans_items.sinv_id = avtrans.id
    LEFT JOIN {avtbl_categories} AS avcat_agent ON avcat_agent.id = avtrans.agent_id AND avcat_agent.group_id = 'agent_cust'
    LEFT JOIN {avtbl_categories} AS avcat_term ON avcat_term.id = avtrans.term_id AND avcat_term.group_id = 'payment_term'
  ";
  $queries['spay'] = "SELECT
    avtrans.id AS transaction_id,
    avtrans.status,
    'spay' AS transaction_type,
    avcat_agent.title AS agent_name,
    avcat_term.title AS term_name,
    avtrans.transaction_date,
    avclients.display_name,
    SUM(avtrans_items.paid_amount) AS grand_total
    FROM {avtbl_spay} AS avtrans
    INNER JOIN {avtbl_customers} AS avclients ON avclients.id = avtrans.client_id
    LEFT JOIN {avtbl_spay_items} AS avtrans_items ON avtrans_items.spay_id = avtrans.id
    LEFT JOIN {avtbl_categories} AS avcat_agent ON avcat_agent.id = avtrans.agent_id AND avcat_agent.group_id = 'agent_cust'
    LEFT JOIN {avtbl_categories} AS avcat_term ON avcat_term.id = avtrans.term_id AND avcat_term.group_id = 'payment_term'
  ";

  // Only include queries for specified transaction types.
  if (!empty($transaction_types)) {
    $queries = array_intersect_key($queries, array_combine($transaction_types, $transaction_types));
  }
  //dpm($param_args);
  //dpm($param_where);
  //$param_where[] = "avcat_agent.title = :SSS";
  //$param_args[':SSS'] = "sss";
  foreach ($queries as $k => $v) {
    if (!empty($param_where)) {
      $queries[$k] .= " WHERE " . implode(' AND ', $param_where);
    }
    $queries[$k] .= " GROUP BY avtrans.id";
  }

  $query = implode(' UNION ', $queries);
  if (is_null($order_by_string)) {
    $query .= " ORDER BY transaction_date DESC";
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
function avtrans_compute_product_cost($current_cost, $onhand_qty, $new_cost, $incoming_base_qty) {
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
function avtrans_compute_sales_price($current_sales_price, $current_cost, $new_cost) {
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
function avtrans_purchase_fields(&$form, &$form_state, $row, $view_mode = FALSE){
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
function avtrans_sales_fields(&$form, &$form_state, $row, $view_mode = FALSE){
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
function avtrans_transaction_form_ajax($form, $form_state) {
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
function avtrans_element_validate_enough_qty(&$element, &$form_state) {
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
function avtrans_redirect_transaction_form($form, &$form_state) {
  $triggering_element = isset($form_state['triggering_element']) ? $form_state['triggering_element'] : array();
  $transaction_id = empty($form_state['values']['id']) ? NULL : $form_state['values']['id'];
  $redirect_query = array();
  if ($triggering_element['#id'] == 'submit_and_print') {
    $redirect_query['print'] = 1;
  }
  $form_state['redirect'] = array(arg(0) . '/' . arg(1) . '/' . $transaction_id . '/view', array('query' => $redirect_query));
}

function avtrans_transaction_types($type) {
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
function avtrans_get_return_types() {
  return array(AVTRANS_RETURN_TYPE_RS => 'RS', AVTRANS_RETURN_TYPE_RUD => 'RUD');
}

/**
 * Transaction buttons.
 * @param $transaction_type
 * @param $view_mode
 *
 * @return array
 */
function avtrans_transaction_buttons($transaction_type, $view_mode) {
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
      '#markup' => l('Cancel', 'av/transactions', array('attributes' => array('class' => array('uk-button')))),
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