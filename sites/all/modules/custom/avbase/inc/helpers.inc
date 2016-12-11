<?php

define('AV_STYLE_CLASS_BTN', 'uk-button');
define('AV_STYLE_CLASS_BTN_PRIMARY', 'uk-button-primary');

/**
 * Get categories.
 * @param string $group_id
 * @return array()
 */
function avbase_get_categories($group_id = NULL) {
  $data = &drupal_static(__FUNCTION__, array());
  if (empty($data)) {
    $query = "SELECT * FROM {avtbl_categories}";
    $result = db_query($query);
    $rows = array();
    foreach ($result as $k => $row) {
      $row->data = unserialize($row->data);
      $rows[$row->group_id][$row->id] = $row;
    }
    $data = $rows;
    $data[$group_id] = isset($data[$group_id]) ? $data[$group_id] : array();
  }

  if (empty($group_id)) {
    return $data;
  }
  return isset($data[$group_id]) ? $data[$group_id] : array();
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
 * @param string $group_id
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
 * Get a category db row.
 * @param array $conditions
 * @return array
 */
function avbase_get_category($conditions) {
  if (isset($conditions['group_id'])) {
    $group_id = $conditions['group_id'];
    unset($conditions['group_id']);
    $rows[$group_id] = avbase_get_categories($group_id);
  }
  else {
    $rows = avbase_get_categories();
  }

  foreach ($rows as $groups) {
    foreach ($groups as $category) {
      $match = FALSE;
      foreach ($conditions as $k => $v) {

        $match = isset($category->$k) && $category->$k == $v;
        if (!$match) {
          break;
        }
      }
      if ($match) {
        return $category;
      }
    }
  }
  return array();
}

/**
 * Get data value of a category.
 * @param $data_id
 * @param $conditions
 * @return string
 */
function avbase_get_category_data_value($data_id, $conditions) {
  $category = avbase_get_category($conditions);
  return isset($category->data[$data_id]) ? $category->data[$data_id] : NULL;
}

function avbase_get_category_plural_form($conditions) {
  $category = avbase_get_category($conditions);
  if (empty($category->data)) {
    return '';
  }

  return check_plain(!empty($category->data['plural_form']) ? $category->data['plural_form'] : $category->title);
}

/**
 * Delete category.
 * @param $id
 *
 * @return bool
 */
function avbase_category_delete($id) {
  if (!empty($id) && is_numeric($id)) {
    if (db_query("DELETE FROM {avtbl_categories} WHERE id = :row_id", array(':row_id' => $id))) {
      return TRUE;
    }
  }
  return FALSE;
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
 * Check if element is a valid and unique display_name.
 * @param $element
 * @param $form_state
 * @param $form
 */
function avbase_element_validate_display_name($element, &$form_state, $form) {
  $values = isset($form_state['values']) ? $form_state['values'] : array();
  $record_id = isset($values['id']) ? $values['id'] : 0;
  //$table_name = $values['table_name'];
  $display_name = $element['#value'];
  $exists = db_query("SELECT id FROM {avtbl_customers} WHERE display_name = :display_name AND id <> :id", array(':display_name' => $display_name, ':id' => $record_id))->fetch();
  if (!$exists) {
    $exists = db_query("SELECT id FROM {avtbl_vendors} WHERE display_name = :display_name AND id <> :id", array(':display_name' => $display_name, ':id' => $record_id))->fetch();
  }
  if ($exists) {
    form_error($element, t('Another customer or vendor is already using this @field_title', array('@field_title' => $element['#title'])));
  }
}


/**
 * Check if element contains an existing display_name.
 * @param $element
 * @param $form_state
 * @param $form
 */
function avbase_element_validate_display_name_exists($element, &$form_state, $form) {
  $values = isset($form_state['values']) ? $form_state['values'] : array();
  $record_id = isset($values['id']) ? $values['id'] : 0;

  $table_name = $element['#table_name'];
  $display_name = $element['#value'];
  $row = db_query("SELECT id, agent_id FROM {{$table_name}} WHERE display_name = :display_name", array(':display_name' => $display_name))->fetch();
  if (!$row) {
    form_error($element, t('Selected @field_title doesn\'t exist.', array('@field_title' => $element['#title'])));
  }
  elseif (!empty($row->agent_id) && $record_id != $row->agent_id) {
    form_error($element, t('Another agent has already been assigned to this @field_title.', array('@field_title' => $element['#title'])));
  }
  else {
    // Set this element's value as matching row id.
    $form_state['values'][$element['#name']] = $row->id;
  }
}

/**
 * Check if category title is unique to its group.
 * @param $element
 * @param $form_state
 * @param $form
 */
function avbase_element_validate_category_title_unique($element, &$form_state, $form) {
  $values = isset($form_state['values']) ? $form_state['values'] : array();
  $record_id = isset($values['id']) ? $values['id'] : 0;
  $title = $element['#value'];
  $exists = db_query("SELECT id FROM {avtbl_categories} WHERE title = :title AND id <> :id", array(':title' => $title, ':id' => $record_id))->fetch();
  if ($exists) {
    form_error($element, t('@field_title already exists.', array('@field_title' => $element['#title'])));
  }
}

/**
 * Custom money format.
 * @param $format
 * @param $number
 *
 * @return string
 */
function avbase_money_format($format, $number) {
  return sprintf($format, $number);
}

/**
 * Load category group details.
 *
 * @param $row_id
 * @return mixed
 */
function avcategory_load($row_id) {
  $data = &drupal_static(__FUNCTION__, array());
  if (isset($data[$row_id])) {
    return $data[$row_id];
  }

  $category_id = arg(2);
  $query = db_select('avtbl_categories', 'avc')->fields('avc');
  $query->condition('avc.id', $row_id);

  // Let other modules alter category load query.
  $query->addTag('avcategory_load_' . $category_id);

  $row = $query->execute()->fetchObject();
  if (isset($row->data)) {
    $row->data = unserialize($row->data);
  }

  $data[$row_id] = $row;
  return $row;
}

/**
 * Check if element contains an existing category.
 * @param $element
 * @param $form_state
 * @param $form
 */
function avbase_element_validate_category_exists($element, &$form_state, $form) {
  if (trim($element['#value']) == '') {
    return;
  }
  $conditions = $element['#element_validate_conditions'];
  $conditions['title'] = check_plain($element['#value']);
  $category = avbase_get_category($conditions);
  if (empty($category->id)) {
    form_error($element, t('Selected @field_title doesn\'t exist.', array('@field_title' => $element['#title'])));
  }
  else {
    $form_state['values'][$element['#name']] = $category->id;
  }
}
