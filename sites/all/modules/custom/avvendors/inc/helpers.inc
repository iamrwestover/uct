<?php

/**
 * Load vendor details.
 *
 * @param $vendor_id
 * @return mixed
 */
function avvendor_load($vendor_id) {
  $data = &drupal_static(__FUNCTION__, array());
  if (isset($data[$vendor_id])) {
    return $data[$vendor_id];
  }

  $query = "SELECT * FROM {avtbl_vendors} WHERE id = :vendor_id";
  $args = array(':vendor_id' => $vendor_id);
  $row = db_query($query, $args)->fetchObject();
  if (isset($row->data)) {
    $row->data = unserialize($row->data);
  }

  $data[$vendor_id] = $row;
  return $row;
}

/**
 * Delete vendor.
 * @param $id
 *
 * @return bool
 */
function avvendors_vendor_delete($id) {
  if (!empty($id) && is_numeric($id)) {
    if (db_query("DELETE FROM {avtbl_vendors} WHERE id = :vendor_id", array(':vendor_id' => $id))) {
      return TRUE;
    }
  }
}
