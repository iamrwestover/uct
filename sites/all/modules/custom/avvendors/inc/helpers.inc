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

/**
 * Menu callback - retrieve a JSON object containing autocomplete suggestions for existing vendors.
 * @param string $string
 */
function avvendors_autocomplete($string = '') {
  $matches = array();
  $string = trim($string);
  if ($string) {
    $result = db_select('avtbl_vendors')
      ->fields('avtbl_vendors', array('id', 'display_name', 'company'))
      ->condition('display_name', '%' . db_like($string) . '%', 'LIKE')
      ->range(0, 10)
      ->execute();
    foreach ($result as $row) {
      $row->display_name = check_plain($row->display_name);
      $row_html = $row->display_name . '<span class="uk-text-muted"> / ' . check_plain($row->company) .  '</span>';
      //$row_html .= '<div style="display: none;" id="a-vendor-id">' . $row->display_name . '</div>';
      $row_html .= '<div style="display: none;" id="a-vendor-text">' . $row->display_name . '</div>';
      $matches[$row->id] = $row_html;
    }
  }

  drupal_json_output($matches);
}
