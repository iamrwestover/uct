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
    $rows[$row->id] = $row;
  }

  return $data[$group_id] = $rows;
}

/**
 * Get multiple categories.
 * @see avbase_get_categories()
 * @param $group_ids
 * @return array
 */
function avbase_get_categories_multiple($group_ids) {
  if (is_numeric($group_ids)) {
    $group_ids = array($group_ids);
  }

  $rows = array();
  foreach ($group_ids as $id) {
    $rows += avbase_get_categories($id);
  }

  return $rows;
}

/**
 * Get list of categories under a group.
 * @param int $group_id
 * @return array
 */
function avbase_get_category_options_array($group_id) {
  $categories = avbase_get_categories($group_id);
  $options = array();
  foreach ($categories as $v) {
    $options[$v->id] = check_plain($v->title);
  }
  return $options;
}

/**
 * Check if element is a valid date.
 * @param $element
 * @param $form_state
 * @param $form
 */
function avbase_element_validate_date($element, &$form_state, $form) {
  $date = $element['#value'];
  $date_format = isset($element['#av_date_format']) ? $element['#av_date_format'] : 'M. d, Y';
  $date_info = date_parse_from_format($date_format, $date);

  $t_args = array(
    '%element_title' => $element['#title'],
    '@date_format' => $date_format,
    '@sample_date' => format_date(time(), 'custom', $date_format)
  );
  if (!empty($date_info['error_count'])) {
    form_error($element, t('Date format for %element_title is invalid. Kindly follow this format: @date_format (e.g. @sample_date)', $t_args));
  }
  elseif (!empty($date_info['warning_count'])) {
    form_error($element, t('%element_title is invalid.', $t_args));
  }
  else {
    // Set this element's value as timestamp equivalent.
    $form_state['values'][$element['#name']] = strtotime($date);
  }
}

/**
 * Custom money format.
 * @param $format
 * @param $number
 */
function avbase_money_format($format, $number) {
  return sprintf($format, $number);
}
