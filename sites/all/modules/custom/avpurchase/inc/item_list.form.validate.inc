<?php

/**
 * General validation for item list form fields.
 * @param $form
 * @param $form_state
 */
function avpurchase_item_list_fields_validate(&$form, &$form_state) {
  // Get form storage.
  $form_state['storage'] = isset($form_state['storage']) ? $form_state['storage'] : array();
  $_storage = &$form_state['storage'];

  // Get item rows.
  $_storage['item_rows'] = isset($_storage['item_rows']) ? $_storage['item_rows'] : array();
  $_item_rows = &$_storage['item_rows'];

  // Make sure at least one product is added.
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

  // Merge inputted product rows into $_item_rows.
  $inputted_rows = isset($form_state['input']['product_rows']) ? $form_state['input']['product_rows'] : array();
  $merged = array();
  foreach ($inputted_rows as $k => $v) {
    $merged[$k] = isset($_item_rows[$k]) ? array_merge($_item_rows[$k], $v) : $v;
  }
  $_item_rows = $merged;
}

/**
 * Only validate a product row element if a product is actually selected.
 * @param $element
 * @param $form_state
 * @param $form
 */
function avpurchase_item_list_row_element_validate($element, &$form_state, $form) {
  // Get form storage.
  $form_state['storage'] = isset($form_state['storage']) ? $form_state['storage'] : array();
  $_storage = &$form_state['storage'];

  // Get item rows.
  $_storage['item_rows'] = isset($_storage['item_rows']) ? $_storage['item_rows'] : array();
  $_item_rows = &$_storage['item_rows'];

  // If a valid product is detected for this element's row, do validate this element.
  $item_row_key = isset($element['#item_row_key']) ? $element['#item_row_key'] : '';
  if (!empty($_item_rows[$item_row_key]['product_id'])) {
    $element_value = $element['#value'];
    if (trim($element_value) == '') {
      form_error($element, t('@field_title is required.', array('@field_title' => $element['#title'])));
    }

    if (!empty($element['#av_prod_element_validate'])) {
      foreach ($element['#av_prod_element_validate'] as $func) {
        $func($element, $form_state);
      }
    }
  }
}
