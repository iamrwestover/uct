<?php

/**
 * Load product details.
 *
 * @param $product_id
 * @return mixed
 */
function avproduct_load($product_id) {
  $data = &drupal_static(__FUNCTION__, array());
  if (isset($data[$product_id])) {
    return $data[$product_id];
  }

  $query = "SELECT * FROM {avtbl_products} WHERE id = :product_id";
  $args = array(':product_id' => $product_id);
  $row = db_query($query, $args)->fetchObject();
  if (isset($row->data)) {
    $row->data = unserialize($row->data);
  }

  $data[$product_id] = $row;
  return $row;
}

/**
 * Delete product.
 * @param $id
 *
 * @return bool
 */
function avproducts_product_delete($id) {
  if (!empty($id) && is_numeric($id)) {
    if (db_query("DELETE FROM {avtbl_products} WHERE id = :product_id", array(':product_id' => $id))) {
      return TRUE;
    }
  }
}

function element_validate_av_group_uom($element, &$form_state, $form) {
  $values = $form_state['values'];
  $base_uom_id = $values['uom_id'];
  if (empty($base_uom_id)) {
    form_set_error('uoms', 'Select a Base UOM first before adding new ones.');
    return;
  }

  $element_value = drupal_array_get_nested_value($values, $element['#parents']);
  if ($element_value == $base_uom_id) {
    form_error($element, 'UOM should be different from Base UOM.');
    return;
  }

  $dup_error_msg = 'Duplicate UOMs selected.';
  $errors = form_get_errors();
  //if (in_array($dup_error_msg, $errors))
  $uoms = $values['uoms'];
  foreach ($uoms as $k => $uom) {
    if ($k == $element['#parents'][1]) {
      continue;
    }

    if ($uom['uom_id'] == $element_value) {
      form_error($element, $dup_error_msg);
      return;
    }
  }
}