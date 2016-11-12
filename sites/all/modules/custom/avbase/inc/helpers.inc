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
 * Validate date.
 * @param $element
 * @param $form_state
 * @param $form
 */
function avbase_date_validate($element, &$form_state, $form) {
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
}
