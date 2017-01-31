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
    $options[$v->id] = $v->title;
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
  if (trim($date) == '') {
    return;
  }

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
  $table_name = $element['#table_name'];
  $display_name = $element['#value'];
  if (trim($display_name) == '') {
    return;
  }
  $row = db_query("SELECT id FROM {{$table_name}} WHERE display_name = :display_name", array(':display_name' => $display_name))->fetch();
  if (!$row) {
    form_error($element, t('Selected @field_title doesn\'t exist.', array('@field_title' => $element['#title'])));
  }
  else {
    // Set extra form state value for this element.
    avbase_element_set_form_state_extra_value($form_state, $element, $row->id);
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
 * Check if element value is a valid email.
 * @param $element
 * @param $form_state
 * @param $form
 */
function avbase_element_validate_email($element, &$form_state, $form) {
  $mail = $element['#value'];
  $triggering_element = isset($form_state['triggering_element']) ? $form_state['triggering_element'] : array();
  $do_validate = $triggering_element['#id'] == 'submit_and_send' ? TRUE : (trim($mail) != '');
  if ($do_validate && $error = user_validate_mail($mail)) {
    form_error($element, $error);
  }
}

/**
 * Load user extra details.
 * @param object $account
 * @return
 */
function avbase_avuser_load($account = NULL) {
  if (empty($account->uid)) {
    return NULL;
  }

  $data = &drupal_static(__FUNCTION__, array());
  if (isset($data[$account->uid])) {
    return $data[$account->uid];
  }

  $query = "SELECT * FROM {avtbl_users} WHERE uid = :uid";
  $rs = db_query($query, array(':uid' => $account->uid));
  $row = $rs->fetchObject();
  $data[$account->uid] = $row;
  return $row;
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
  //$save_value = &drupal_array_get_nested_value($form_state['values'], $element['#parents']);
  $conditions = $element['#element_validate_conditions'];
  $conditions['title'] = check_plain($element['#value']);
  $category = avbase_get_category($conditions);
  if (empty($category->id)) {
    form_error($element, t('Selected @field_title doesn\'t exist.', array('@field_title' => $element['#title'])));
  }
  else {
    // Set extra form state value for this element.
    avbase_element_set_form_state_extra_value($form_state, $element, $category->id);

    // Update form_state storage item_rows.
    avtrans_item_list_update_item_row($form_state, $element, $category->id);
  }
}

/**
 * Check if element contains a unique field.
 * @param $element
 * @param $form_state
 * @param $form
 */
function avbase_element_validate_unique_field($element, &$form_state, $form) {
  $values = isset($form_state['values']) ? $form_state['values'] : array();
  $record_id = isset($values['id']) ? $values['id'] : 0;
  $table_name = $values['table_name'];
  $element_value = $element['#value'];
  $element_name = $element['#name'];
  $exists = db_query("SELECT id FROM {{$table_name}} WHERE $element_name = :element_value AND id <> :id", array(':element_value' => $element_value, ':id' => $record_id))->fetch();
  if ($exists) {
    form_error($element, t('Another record is already using this @field_title', array('@field_title' => $element['#title'])));
  }
}

/**
 * Check if element contains an existing product name.
 * @param $element
 * @param $form_state
 * @param $form
 */
function avbase_element_validate_product_name_exists(&$element, &$form_state, $form) {
  $element_value = $element['#value'];
  if (trim($element_value) == '') {
    return;
  }

  $rows = avproduct_query_load(array('avprod.title = :title'), array(':title' => $element_value));
  $row = array_shift($rows);
  if (empty($row->id)) {
    form_error($element, t('@field_title doesn\'t exist.', array('@field_title' => $element['#title'])));
  }
  else {
    // Set extra form state value for this element.
    avbase_element_set_form_state_extra_value($form_state, $element, $row->id);

    // Update form_state storage item_rows.
    avtrans_item_list_update_item_row($form_state, $element, $row->id);
  }
}

/**
 * Return discount types.
 * @return array
 */
function avbase_get_discount_types() {
  return array(AVBASE_DISCOUNT_PERCENT => 'Percent', AVBASE_DISCOUNT_FIXED => 'Fixed amount');
}

/**
 * Turn form edit form into view only mode.
 * @param $form
 * @param $view_mode
 */
function avbase_allow_form_view_mode(&$form, $view_mode = FALSE) {
  if (empty($view_mode)) {
    return;
  }
  $form['#av_view_mode'] = TRUE;
  foreach (element_children($form) as $k) {
    $element = &$form[$k];
    if (!empty($element['#visible_in_view_mode'])) {
      continue;
    }

    unset($element['#element_validate']);
    if (isset($element['#type'])) {
      $element['#required'] = FALSE;
      switch ($element['#type']) {
        case 'textfield':
          $element['#type'] = 'item';
          $default_value = isset($element['#default_value']) ? $element['#default_value'] : '';
          $element['#markup'] = trim($default_value) == '' ? '' : $default_value;
          break;
        case 'textarea':
          $element['#type'] = 'item';
          $default_value = isset($element['#default_value']) ? $element['#default_value'] : '';
          $element['#markup'] = trim($default_value) == '' ? '' : check_markup($default_value);
          break;
        case 'select':
          $options = isset($element['#options']) ? $element['#options'] : array();
          $default_value = isset($element['#default_value']) ? $element['#default_value'] : NULL;
          $empty_value = isset($element['#empty_value']) ? $element['#empty_value'] : '';
          $default_value = is_null($default_value) ? $empty_value : $default_value;
          $element['#type'] = 'item';
          $default_option_value = isset($options[$default_value]) ? $options[$default_value] : '';
          $default_option_value = trim($default_option_value) == '' ? '' : $default_option_value;
          $element['#markup'] = $default_option_value;
          break;
        case 'checkbox':
          $default_value = isset($element['#default_value']) ? $element['#default_value'] : 0;
          $icon_class = $default_value ? 'check-square-o' : 'square-o';
          $element['#type'] = 'markup';
          $element['#markup'] = '<span><i class="uk-icon-small uk-icon-' . $icon_class . '"></i> ' . $element['#title'] . '</span>';
          break;
        case 'submit':
        case 'button':
          $element['#access'] = !empty($element['#visible_in_view_mode']);
          break;
      }
    }
    if (element_children($element)) {
      avbase_allow_form_view_mode($element, $view_mode);
    }
  }
}

/**
 * Format a number to minimum and maximum decimal precision.
 *
 * @param $x
 *  The number to format
 * @param $max
 *  The maximum number of decimal places. If empty, no limit.
 * @param $min
 *  The number of minimum decimal places.
 *
 * @return string
 */
function avbase_number_format($x, $max = AVBASE_DECIMAL_FORMAT_MAX, $min = 2) {
  $e = pow(10, $min);
  return (floor($x * $e) == $x * $e) ? sprintf("%.${min}f", $x) : (empty($max) ? $x : round($x, $max));
}

/**
 * Callback to return JSON encoded details of any drupal function.
 * @param $function_name
 * @param $id
 *
 * @throws Exception
 */
function avbase_ajax_load($function_name, $id) {
  $data = array();
  if (function_exists($function_name)) {
    $data = $function_name($id);
  }
  drupal_json_output(array('status' => 0, 'data' => $data));
}

/**
 * Custom watchdog to log user actions. Made because watchdog() wipe is inevitable.
 * @param $type
 * @param $message
 * @param array $variables
 * @param int $severity
 * @param null $link
 */
function avbase_custom_watchdog($type, $message, $variables = array(), $severity = WATCHDOG_INFO, $link = NULL) {
  global $user, $base_root;

  // The user object may not exist in all conditions, so 0 is substituted if needed.
  $user_uid = isset($user->uid) ? $user->uid : 0;

  // Prepare the fields to be logged
  $log_entry = array(
    'type' => drupal_substr($type, 0, 64),
    'message' => $message,
    'variables' => serialize($variables),
    'severity' => $severity,
    'link' => drupal_substr($link, 0, 255),

    'uid' => $user_uid,
    'location' => $base_root . request_uri(),
    'referer' => isset($_SERVER['HTTP_REFERER']) ? $_SERVER['HTTP_REFERER'] : '',
    'hostname' => drupal_substr(ip_address(), 0, 128),
    'timestamp' => time(),
  );

  db_insert('avtbl_watchdog')->fields($log_entry)->execute();
}

/**
 * Check if element value does not exceed AVBASE_DIGITS_MAX.
 * @param $element
 * @param $form_state
 */
function avbase_element_validate_money_length($element, &$form_state) {
  $element_value = round((float) $element['#value'], 0);
  if (strlen($element_value) > AVBASE_DIGITS_MAX) {
    form_error($element, t('Number of digits (excluding decimal places) for %element_title must not exceed %digits.', array('%element_title' => $element['#title'], '%digits' => AVBASE_DIGITS_MAX)));
  }
}

/**
 * Copy element value to a separate form state entry.
 * @param $form_state
 * @param $element
 * @param $value
 */
function avbase_element_set_form_state_extra_value(&$form_state, $element, $value) {
  $save_id = isset($element['#element_save_id']) ? $element['#element_save_id'] : '';
  if (empty($save_id)) {
    return;
  }

  $parents = $element['#parents'];
  array_pop($parents);
  $parents[] = $save_id;

  // Update form_state values.
  drupal_array_set_nested_value($form_state['values'], $parents, $value);
}

/**
 * @return array
 */
function avbase_entity_details() {
  return array(
    'vendors' => array('title' => t('Vendor')),
    'customers' =>  array('title' => t('Customer')),
  );
}

/**
 * Update $form_state['storage']['item_rows'] based on the given $element.
 * @param $form_state
 * @param $element
 * @param $value
 */
function avtrans_item_list_update_item_row(&$form_state, $element, $value) {
  $item_row_key = isset($element['#item_row_key']) ? $element['#item_row_key'] : '';
  if (empty($item_row_key)) {
    return;
  }

  // Get save id.
  $save_id = isset($element['#element_save_id']) ? $element['#element_save_id'] : $element['#name'];

  // Get form storage.
  $form_state['storage'] = isset($form_state['storage']) ? $form_state['storage'] : array();
  $_storage = &$form_state['storage'];

  // Get item rows.
  $_storage['item_rows'] = isset($_storage['item_rows']) ? $_storage['item_rows'] : array();
  $_item_rows = &$_storage['item_rows'];

  if (isset($_item_rows[$item_row_key])) {
    $_item_rows[$item_row_key][$save_id] = $value;
  }
}

/**
 * Checks that the file is a valid excel file.
 *
 * @param $file
 *   A Drupal file object.
 *
 * @return array
 *   An array. If the file is not an image, it will contain an error message.
 *
 * @see hook_file_validate()
 */
function avbase_file_validate_is_excel(stdClass $file) {
  $errors = array();

  //$info = image_get_info($file->uri);
  //if (!$info || empty($info['extension'])) {
  //  $errors[] = t('Only JPEG, PNG and GIF images are allowed.');
  //}

  //$filename = $_POST['your_field_file'];
  $finfo = finfo_open(FILEINFO_MIME_TYPE);
  $file_type = finfo_file($finfo, $file->uri);
  $valid_types = array(
    'application/vnd.openxmlformats-officedocument.spreadsheetml.sheet',
    'application/vnd.ms-excel',
    'application/vnd.ms-office',
  );
  if (!in_array($file_type, $valid_types)) {
    $errors[] = t('Selected file %filename is not a valid .xls / .xlsx file.', array('%filename' => $file->filename));
  }

  return $errors;
}

/**
 * @param $title
 * @param $group_id
 * @param bool|FALSE $insert_if_not_found
 * @return int
 */
function avbase_get_category_id_by_title($title, $group_id, $insert_if_not_found = FALSE) {
  $title = trim($title);
  if (empty($title) || empty($group_id)) {
    return NULL;
  }
  $id = db_query('SELECT id FROM {avtbl_categories} WHERE title = :title AND group_id = :group_id', array(
    ':title' => $title,
    ':group_id' => $group_id
  ))->fetchField();
  if (empty($id) && $insert_if_not_found) {
    $id = db_insert('avtbl_categories')->fields(array('title' => $title, 'group_id' => $group_id))->execute();
  }
  return $id;
}

/**
 * Form element validation handler for number elements.
 */
function avbase_element_validate_non_negative_number($element, &$form_state) {
  $value = $element['#value'];
  if ($value !== '' && (!is_numeric($value) || intval($value) != $value || $value < 0)) {
    form_error($element, t('%name must not be less than 0.', array('%name' => $element['#title'])));
  }
}
