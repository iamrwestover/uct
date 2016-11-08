<?php

define('AV_STYLE_CLASS_BTN', 'uk-button');
define('AV_STYLE_CLASS_BTN_PRIMARY', 'uk-button-primary');

/**
 * Get categories.
 * @param int $group_id
 * @return array()
 */
function avbase_get_categories($group_id) {
  $data = &drupal_static(__FUNCTION__, array());
  if (isset($data[$group_id])) {
    return $data[$group_id];
  }

  $query = "SELECT * FROM {avtbl_category_groups} WHERE group_id = :group_id";
  $result = db_query($query, array(':group_id' => $group_id));
  $rows = array();
  foreach ($result as $k => $row) {
    $rows[] = $row;
  }

  return $data[$group_id] = $rows;
}

/**
 * Get payment methods array for use with options form api.
 */
function avbase_get_payment_method_options_array() {
  $payment_methods = avbase_get_categories(AV_CATEGORY_ID_PAYMENT_METHOD);
  $options = array();
  foreach ($payment_methods as $v) {
    $options[$v->id] = check_plain($v->title);
  }
  return $options;
}
