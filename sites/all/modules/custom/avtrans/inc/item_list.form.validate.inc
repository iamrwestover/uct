<?php

/**
 * General validation for item list form fields.
 * @param $form
 * @param $form_state
 */
function avtrans_item_list_fields_validate(&$form, &$form_state) {
  // Get form storage.
  $form_state['storage'] = isset($form_state['storage']) ? $form_state['storage'] : array();
  $_storage = &$form_state['storage'];

  // Get item rows.
  $_storage['item_rows'] = isset($_storage['item_rows']) ? $_storage['item_rows'] : array();
  $_item_rows = &$_storage['item_rows'];
  $transaction_type = empty($_storage['transaction_type']) ? '' : $_storage['transaction_type'];

  // Make sure at least one product is added.
  if ($transaction_type != 'spay') {
    $detected_product_ids = array();
    foreach ($_item_rows as $k => $row) {
      if (!empty($row['product_id'])) {
        $detected_product_ids[$k] = $row['product_id'];
        $form['product_rows'][$k]['product_title']['#attributes']['data-selected-product-id'] = $row['product_id'];
      }
    }

    // No valid product detected.
    if (empty($detected_product_ids)) {
      form_set_error('', t('Please add at least one valid product.'));
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

  if ($transaction_type == 'spay' && empty($_item_rows)) {
    form_set_error('', t('No payment received.'));
  }
}

/**
 * Only validate a product row element if a product is actually selected.
 * @param $element
 * @param $form_state
 * @param $form
 */
function avtrans_item_list_row_element_validate($element, &$form_state, $form) {
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
function avtrans_element_validate_product_discount($element, &$form_state) {
  $element_value = trim($element['#value']);
  if ($element_value == '') {
    return;
  }

  $parts = explode('/', $element_value);
  foreach ($parts as $k => $v) {
    if (!is_numeric($v)) {
      form_error($element, t('Invalid discount format. Valid format is numeric values separated by /.'));
      return;
    }
    $parts[$k] = (float) $v;
  }

  $parents = $element['#parents'];
  drupal_array_set_nested_value($form_state['values'], $parents, implode('/', $parts));
}
