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

  $query = "SELECT avv.*, avc.title AS agent_name
  FROM {avtbl_vendors} avv
  LEFT JOIN {avtbl_categories} avc ON avc.id = avv.agent_id AND avc.group_id = :group_id
  WHERE avv.id = :vendor_id";
  $args = array(':vendor_id' => $vendor_id, ':group_id' => 'agent_vend');
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
    $db_or = db_or();
    $db_or->condition('avv.display_name', '%' . db_like($string) . '%', 'LIKE');
    $db_or->condition('avv.company', '%' . db_like($string) . '%', 'LIKE');
    $db_or->condition('avv.first_name', '%' . db_like($string) . '%', 'LIKE');
    $db_or->condition('avv.last_name', '%' . db_like($string) . '%', 'LIKE');

    $query = db_select('avtbl_vendors', 'avv')->fields('avv', array('id', 'display_name', 'company', 'email', 'term_id', 'discount_type', 'discount_value'));
    $query->condition($db_or);
    $query->range(0, 15);
    $result = $query->execute();

    foreach ($result as $row) {
      $row->display_name = check_plain($row->display_name);
      $row_html = $row->display_name . '<span class="uk-text-muted"> / ' . check_plain($row->company) .  '</span>';
      $row_html .= '<div style="display: none;" id="av-row-id">' . $row->id . '</div>';
      $row_html .= '<div style="display: none;" id="av-row-json">' . drupal_json_encode($row) . '</div>';
      $matches[$row->display_name] = $row_html;
    }
  }

  drupal_json_output($matches);
}

/**
 * Return list of vendor categories.
 */
function avvendors_get_vendor_categories() {
  return array(
    AVVENDORS_CATEGORY_SUPPLIER => 'Supplier',
    AVVENDORS_CATEGORY_EXPENSES => 'Expenses',
  );
}