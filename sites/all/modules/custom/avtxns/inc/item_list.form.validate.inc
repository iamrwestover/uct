<?php

/**
 * General validation for item list form fields.
 * @param $form
 * @param $form_state
 */
function avtxns_item_list_fields_validate(&$form, &$form_state) {
  // Get form storage.
  $form_state['storage'] = isset($form_state['storage']) ? $form_state['storage'] : array();
  $_storage = &$form_state['storage'];

  // Get item rows.
  $_storage['item_rows'] = isset($_storage['item_rows']) ? $_storage['item_rows'] : array();
  $_item_rows = &$_storage['item_rows'];
  $transaction_type = empty($form_state['values']['transaction_type']) ? '' : $form_state['values']['transaction_type'];


  // Make sure at least one product is added.
  if ($transaction_type != AVTXNS_TXN_TYPE_PAYMENT) {
    $detected_product_ids = array();
    foreach ($_item_rows as $k => $row) {
      if (!empty($row['product_id'])) {
        $detected_product_ids[$k] = $row['product_id'];
        $form['product_rows'][$k]['product_title']['#attributes']['data-selected-product-id'] = $row['product_id'];
      }
    }

    // No valid product detected.
    if (empty($detected_product_ids)) {
      form_set_error('', t('Please add at least one valid item.'));
    }

    // Save product row details into drupal JS settings.
    if (!empty($detected_product_ids)) {
      $product_rows = avproduct_load_ids($detected_product_ids);
      drupal_add_js(array('avbase' => array('products' => $product_rows)), 'setting');
    }
  }


  // Merge inputted product rows into $_item_rows.
  $inputted_rows = isset($form_state['input']['product_rows']) ? $form_state['input']['product_rows'] : array();
  $merged = array();
  //$rearranged_input_rows = array();
  foreach ($inputted_rows as $k => $v) {
    //$_item_rows[$k] = isset($_item_rows[$k]) ? array_merge($_item_rows[$k], $v) : $v;
    $merged[$k] = isset($_item_rows[$k]) ? array_merge($_item_rows[$k], $v) : $v;
    //$rearranged_input_rows[$k] = $v;
  }
  $_item_rows = $merged;

  // Make sure all included reference transaction IDs are still open.
  $ref_txn_ids = array();
  foreach ($_item_rows as $k => $v) {
    $ref_txn_id = empty($v['ref_txn_id']) ? NULL : $v['ref_txn_id'];
    if ($ref_txn_id) {
      $ref_txn_ids[$ref_txn_id] = $ref_txn_id;
    }
  }

  $query = "SELECT id, status FROM {avtbl_transactions} WHERE id IN (:txn_ids)";
  $rs = db_query($query, array(':txn_ids' => $ref_txn_ids));
  foreach ($rs as $txn) {
    if ($txn->status != AVTXNS_TXN_STATUS_OPEN) {
      form_set_error('', t('Cannot proceed with items from Transaction # @txn_id. Transaction is not OPEN anymore. Try removing these items before proceeding.', array('@txn_id' => $txn->id)));
    }
  }

  if ($transaction_type == AVTXNS_TXN_TYPE_PAYMENT && empty($_item_rows)) {
    form_set_error('', t('No payment received.'));
  }
}

/**
 * Only validate a product row element if a product is actually selected.
 * @param $element
 * @param $form_state
 * @param $form
 */
function avtxns_item_list_row_element_validate($element, &$form_state, $form) {
  // Get form storage.
  $form_state['storage'] = isset($form_state['storage']) ? $form_state['storage'] : array();
  $_storage = &$form_state['storage'];

  // Get item rows.
  $_storage['item_rows'] = isset($_storage['item_rows']) ? $_storage['item_rows'] : array();
  $_item_rows = &$_storage['item_rows'];
  //$transaction_type = empty($_storage['transaction_type']) ? '' : $_storage['transaction_type'];

  // If a valid product is detected for this element's row, do validate this element.
  $item_row_key = isset($element['#item_row_key']) ? $element['#item_row_key'] : '';
  if (!empty($_item_rows[$item_row_key]['product_id'])) {
    $element_value = $element['#value'];
    if (empty($element['#dont_require']) && trim($element_value) == '') {
      form_error($element, t('@field_title is required.', array('@field_title' => $element['#title'])));
    }

    if (!empty($element['#av_prod_element_validate'])) {
      foreach ($element['#av_prod_element_validate'] as $func) {
        $func($element, $form_state);
      }
    }
  }
}

/**
 * Check if element value is in a valid discount format.
 * @param $element
 * @param $form_state
 */
function avtxns_element_validate_product_discount($element, &$form_state) {
  $element_value = trim($element['#value']);
  if ($element_value == '') {
    return;
  }

  $parts = explode('/', $element_value);
  if (count($parts) > 4) {
    form_error($element, t('Only up to 4 discount parts is allowed.'));
    return;
  }

  foreach ($parts as $k => $v) {
    $error = !is_numeric($v);
    if (!$error) {
      $v = (float) $v;
      $error = $v < 1 || $v > 99;
    }
    if ($error) {
      form_error($element, t('Invalid discount format. Valid format is numeric values (1-99) separated by /.'));
      return;
    }
    $parts[$k] = (float) $v;
  }

  $parents = $element['#parents'];
  drupal_array_set_nested_value($form_state['values'], $parents, implode('/', $parts));
}
