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
