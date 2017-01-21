<?php
module_load_include('inc', 'avtrans', 'inc/po.helpers');
module_load_include('inc', 'avtrans', 'inc/so.helpers');
module_load_include('inc', 'avtrans', 'inc/gr/gr.helpers');
module_load_include('inc', 'avtrans', 'inc/delivery/delivery.helpers');

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
  if ($current_sales_price <= $current_cost) {
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
function avtrans_sales_fields(&$form, &$form_state, $row){
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
  return $form;
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
        form_error($element, t('Requesting %qty1 for %product but on-hand quantity is only %qty2.', array(
          '%qty1' => number_format($inputted_base_qty),
          '%qty2' => number_format($current_qty),
          '%product' => $product->title,
        )));
      }
    }
  }
  else {
    drupal_set_message('Validation against on-hand quantity failed.', 'warning');
  }


}
