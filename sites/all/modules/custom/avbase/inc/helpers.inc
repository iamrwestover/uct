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
    $query = "SELECT * FROM {avtbl_categories} ORDER BY title";
    $result = db_query($query);
    $rows = array();
    foreach ($result as $k => $row) {
      $row->data = unserialize($row->data);
      if ($group_id == 'payment_term' && isset($row->data['days'])) {
        $row->data['due_date'] = format_date(strtotime("+{$row->data['days']} day"), 'custom', 'M. d, Y');
      }
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
    //'@date_format' => $date_format,
    '%sample_date' => format_date(time(), 'custom', $date_format)
  );
  if (!empty($date_info['error_count'])) {
    form_error($element, t('Date format for %element_title is invalid. Kindly follow this format: %sample_date', $t_args));
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
function avbase_element_validate_client_name($element, &$form_state, $form) {
  $values = isset($form_state['values']) ? $form_state['values'] : array();
  $record_id = isset($values['id']) ? $values['id'] : 0;
  //$table_name = $values['table_name'];
  $display_name = $element['#value'];
  $exists = db_query("SELECT id FROM {avtbl_clients} WHERE display_name = :display_name AND id <> :id", array(':display_name' => $display_name, ':id' => $record_id))->fetch();
  if ($exists) {
    form_error($element, t('Another customer, vendor, or branch is already using this @field_title', array('@field_title' => $element['#title'])));
  }
}

/**
 * Check if element contains an existing client name.
 * @param $element
 * @param $form_state
 * @param $form
 */
function avbase_element_validate_client_name_exists($element, &$form_state) {
  $display_name = $element['#value'];
  if (trim($display_name) == '') {
    return;
  }
  $row = db_query("SELECT id FROM {avtbl_clients} WHERE display_name = :display_name", array(':display_name' => $display_name))->fetch();
  if (!$row) {
    form_error($element, t('Selected @field_title doesn\'t exist.', array('@field_title' => $element['#title'])));
  }
  else {
    // Set extra form state value for this element.
    avbase_element_set_form_state_extra_value($form_state, $element, $row->id);

    // Update form_state storage item_rows.
    avtrans_item_list_update_item_row($form_state, $element, $row->id);
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

  $query = "SELECT * FROM {avtbl_users} avusers WHERE uid = :uid";
  $row = db_query($query, array(':uid' => $account->uid))->fetch();

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
  $element_value = trim($element_value);
  if ($element_value == '') {
    return;
  }

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

  $query = "SELECT id FROM {avtbl_products} WHERE title = :title";
  $row_id = db_query($query, array(':title' => $element_value))->fetchField();

  //$rows = avproduct_query_load(array('avprod.title = :title'), array(':title' => $element_value));
  //$row = array_shift($rows);
  if (empty($row_id)) {
    form_error($element, t('@field_title doesn\'t exist.', array('@field_title' => $element['#title'])));
  }
  else {
    // Set extra form state value for this element.
    avbase_element_set_form_state_extra_value($form_state, $element, $row_id);

    // Update form_state storage item_rows.
    avtrans_item_list_update_item_row($form_state, $element, $row_id);
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
          $element['#markup'] = trim($default_value) == '' ? '' : (empty($element['#avhtml']) ? check_plain($default_value) : $default_value);
          break;
        case 'textarea':
          $element['#type'] = 'item';
          $default_value = isset($element['#default_value']) ? $element['#default_value'] : '';
          $default_value = empty($element['#avhtml']) ? check_plain($default_value) : $default_value;
          $element['#markup'] = trim($default_value) == '' ? '' : nl2br($default_value);
          break;
        case 'select':
          $options = isset($element['#options']) ? $element['#options'] : array();
          $default_value = isset($element['#default_value']) ? $element['#default_value'] : NULL;
          $empty_value = isset($element['#empty_value']) ? $element['#empty_value'] : '';
          $default_value = is_null($default_value) ? $empty_value : $default_value;
          $element['#type'] = 'item';
          $default_option_value = isset($options[$default_value]) ? $options[$default_value] : '';
          $default_option_value = trim($default_option_value) == '' ? '' : $default_option_value;
          $element['#markup'] = check_plain($default_option_value);
          break;
        case 'checkbox':
          $default_value = isset($element['#default_value']) ? $element['#default_value'] : 0;
          $icon_class = $default_value ? 'check-square-o' : 'square-o';
          $element['#type'] = 'markup';
          $title = (empty($element['#title_display']) || $element['#title_display'] != 'none') ? (' ' . $element['#title']) : '';
          $element['#markup'] = '<span><i class="uk-icon-' . $icon_class . '"></i>' . check_plain($title) . '</span>';
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
  return (floor($x * $e) == $x * $e) ? sprintf("%.${min}f", $x) : (empty($max) ? $x : number_format($x, $max, '.', ''));
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
    //'variables' => serialize($variables),
    'variables' => '',
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
    'clients' =>  array('title' => t('Client')),
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
  if ($value !== '' && (!is_numeric($value) || floatval($value) != $value || $value < 0)) {
    form_error($element, t('%name must be a valid number and is not less than 0.', array('%name' => $element['#title'])));
  }
}

/**
 * Form element validation handler for number elements.
 */
function avbase_element_validate_non_negative_integer($element, &$form_state) {
  $value = $element['#value'];
  if ($value !== '' && (!is_numeric($value) || intval($value) != $value || $value < 0)) {
    form_error($element, t('%name must be a whole number and must not be less than 0.', array('%name' => $element['#title'])));
  }
}

/**
 * Append extra user details to user object.
 * @param $account
 */
function avbase_user_extra_details(&$account) {
  if (empty($account->uid) || !empty($account->avdata)) {
    return;
  }

  $row = avbase_avuser_load($account);
  if (empty($row->id)) {
    $account->avdata = new stdClass();
    $account->avdata->full_name = $account->name;
    return;
  }
  $name_details = array(
    'first_name' => empty($row->first_name) ? NULL : $row->first_name,
    'last_name' => empty($row->last_name) ? NULL : $row->last_name,
  );
  $name_details = array_filter($name_details);
  $full_name = implode(' ', $name_details);
  $row->full_name = empty($full_name) ? $account->name : $full_name;
  $account->avdata = $row;
}

/**
 * Check if element value is in a valid discount format.
 * @param $element
 * @param $form_state
 */
function avbase_element_validate_product_discount($element, &$form_state) {
  $element_value = trim($element['#value']);
  if ($element_value == '') {
    return;
  }

  $parts = explode('/', $element_value);
  if (count($parts) > 4) {
    form_error($element, t('Only up to 4 discount parts is allowed.'));
    return;
  }

  foreach ($parts as $k => $v) {
    $error = !is_numeric($v);
    if (!$error) {
      $v = (float) $v;
      $error = $v == 0 || $v < 0 || $v > 100;
    }
    if ($error) {
      form_error($element, t('Invalid discount format. Valid format is positive numbers separated by /.'));
      return;
    }
    $parts[$k] = (float) $v;
  }

  //$parents = $element['#parents'];
  //drupal_array_set_nested_value($form_state['values'], $parents, implode('/', $parts));
}

/**
 * Update product quantities accordingly.
 * @param $item_rows
 * @param $transaction_row
 * @param bool $from_void
 * @param array $to_be_deleted_rows
 */
function avbase_product_qty_update($item_rows, $transaction_row, $from_void = FALSE, $to_be_deleted_rows = array()) {
  $transaction_row = (object) $transaction_row;
  $transaction_type = empty($transaction_row->transaction_type) ? '' : $transaction_row->transaction_type;
  $ttd = avtxns_txn_types($transaction_type);

  // No need to update inventory for specific transactions.
  if (!isset($ttd['qty_change'])) {
    return;
  }

  $incoming_base_qty = array();
  $item_rows += $to_be_deleted_rows;
  foreach ($item_rows as $item_row) {
    $item_row = (array) $item_row;
    $item_id = isset($item_row['item_id']) ? $item_row['item_id'] : NULL;
    $item_id = empty($item_id) ? (isset($item_row['product_id']) ? $item_row['product_id'] : NULL) : $item_id;

    if (empty($item_id)) {
      continue;
    }

    $this_prod_total_incoming_base_qty = empty($incoming_base_qty[$item_id]) ? 0 : $incoming_base_qty[$item_id];
    $previous_incoming_base_qty = empty($item_row['incoming_base_qty']) ? 0 : $item_row['incoming_base_qty'];
    $previous_incoming_base_qty = empty($item_row['id']) ? 0 : $previous_incoming_base_qty;
    $previous_incoming_base_qty = $from_void ? 0 : $previous_incoming_base_qty;

    $item_row_qty = isset($item_row['qty']) ? $item_row['qty'] : 0;
    $item_row_qty_per_uom = isset($item_row['qty_per_uom']) ? $item_row['qty_per_uom'] : 1;
    $item_row_incoming_base_qty = ($item_row_qty * $item_row_qty_per_uom);
    //if ($transaction_type == AVTXNS_TXN_TYPE_SALES_RETURN && empty($item_row['good_stock'])) {
    //  $item_row_incoming_base_qty = 0;
    //}

    if (!empty($item_id)) {
      $incoming_base_qty[$item_id] = $this_prod_total_incoming_base_qty + $item_row_incoming_base_qty - $previous_incoming_base_qty;
    }
  }

  if (!empty($incoming_base_qty)) {
    $qty_update_query = '';
    if (isset($ttd['qty_change'])) {
      if ($ttd['qty_change'] == 'increment') {
        $operator = $from_void ? '-' : '+';
        $qty_update_query = "UPDATE {avtbl_products} SET qty = qty $operator CASE id";
      }
      elseif ($ttd['qty_change'] == 'decrement') {
        $operator = $from_void ? '+' : '-';
        $qty_update_query = "UPDATE {avtbl_products} SET qty = qty $operator CASE id";
      }
    }
    if (!empty($qty_update_query)) {
      foreach ($incoming_base_qty as $k => $v) {
        $qty_update_query .= " WHEN $k THEN $v";
      }
      $qty_update_query .= " END WHERE id IN (" . implode(', ', array_keys($incoming_base_qty)) . ")";
      if (db_query($qty_update_query)) {
        avbase_custom_watchdog("{$transaction_type}__qty_update", "ID: {$transaction_row->id}", array('#row' => $transaction_row, '#incoming_base_qty' => $incoming_base_qty, '#entity_name' => $transaction_type));
      }
    }
    else {
      drupal_set_message('Product quantities were not updated', 'warning');
      avbase_custom_watchdog("{$transaction_type}__qty_update_warn1", "ID: {$transaction_row->id}", array('#row' => $transaction_row, '#incoming_base_qty' => $incoming_base_qty, '#entity_name' => $transaction_type));
    }
  }
}

/**
 * Generic crud form validation.
 * @param $form
 * @param $form_state
 */
function avbase_crud_form_validate($form, $form_state) {
  $row_id = empty($form_state['values']['id']) ? NULL : $form_state['values']['id'];
  $table_name = isset($form_state['values']['table_name']) ? $form_state['values']['table_name'] : NULL;
  $last_changed = isset($form_state['values']['last_changed']) ? $form_state['values']['last_changed'] : '-';
  // Check if no other user saved changes on this record.
  if (!empty($row_id) && $last_changed != '-') {
    $query = "SELECT changed FROM {{$table_name}} WHERE id = :row_id";
    $db_last_changed = db_query($query, array(':row_id' => $row_id))->fetchField();
    if ($db_last_changed != $last_changed) {
      form_set_error('', t('Changes cannot be saved. Someone has recently made changes on this record while you were editing it.'));
    }
  }
}

function avbase_pagination_and_sort_settings($user_filter) {
  if (!empty($user_filter['settings']['table_href'])) {
    $table_ajax_href = $user_filter['settings']['table_href'];
    $href_parts = parse_url($table_ajax_href);
    if (!empty($href_parts['query'])) {
      parse_str($href_parts['query'], $href_query);
      foreach ($href_query as $k => $v) {
        $_GET[$k] = $v;
      }
    }
  }
}

/**
 * Custom order by builder for static drupal queries.
 * @param $header
 *
 * @return array|null
 */
function avbase_build_header_order_array($header) {
  $order_by = NULL;
  if (isset($_GET['order'])) {
    $order_by = array();
    $order_by_field_name = '';
    foreach ($header as $h) {
      if ($h['data'] == $_GET['order'] && isset($h['field'])) {
        $order_by_field_name = $h['field'];
        break;
      }
    }
    if (!empty($order_by_field_name)) {
      $order_by[$order_by_field_name] = $order_by_field_name . ' ' . strtoupper($_GET['sort']);
    }
  }
  return $order_by;
}

/**
 * Check if client reached the set credit limit.
 * @param $client_id
 * @param null $transaction_id
 * @return bool
 */
function avbase_client_limit_reached($client_id, $transaction_id = NULL) {
  $data = &drupal_static(__FUNCTION__, array());
  $static_key = "$client_id:$transaction_id";
  if (isset($data[$static_key])) {
    return $data[$static_key];
  }

  $balance_ages_form = avreports_client_balance_aging_form($client_id);
  if (empty($balance_ages_form)) {
    return FALSE;
  }

  $transaction_row = NULL;
  if (!empty($transaction_id)) {
    $transaction_row = avtxns_txn_load($transaction_id);
  }

  $credit_limit = $balance_ages_form['#credit_limit'];
  $client_balance = $balance_ages_form['#total_balance'];
  $this_transaction_total = empty($transaction_row->grand_total) ? 0 : $transaction_row->grand_total;
  $limit_reached = !empty($credit_limit) && round(($client_balance + $this_transaction_total), 2) >= round($credit_limit, 2);

  $data[$static_key] = $limit_reached;
  return $data[$static_key];
}

/**
 * Return payment method array list.
 */
function avbase_payment_method_array() {
  return array(
    AVBASE_PMT_METHOD_CASH => t('Cash'),
    AVBASE_PMT_METHOD_CHECK => t('Check'),
  );
}

/**
 * Return nested version of rows.
 *
 * @param array $rows
 * @param int $parent
 * @param bool|TRUE $hide_fixed
 * @param bool|TRUE $hide_inactive
 *
 * @return array
 */
function avbase_rows_build_tree(array $rows, $parent = 0, $hide_fixed = TRUE, $hide_inactive = TRUE) {
  $branch = array();

  foreach ($rows as $row) {
    $row = (array) $row;
    if ($hide_fixed && !empty($row['fixed'])) {
      continue;
    }
    if ($hide_inactive && empty($row['status'])) {
      continue;
    }
    //$account_type = empty($row['account_type']) ? '' : $row['account_type'];
    //if (in_array($account_type, array(AVTXN_AC_TYPE_AR, AVTXN_AC_TYPE_AP))) {
    //  continue;
    //}

    if ($row['parent'] == $parent) {
      $children = avbase_rows_build_tree($rows, $row['id'], $hide_fixed, $hide_inactive);
      if ($children) {
        $row['children'] = $children;
      }
      $branch[$row['id']] = $row;
    }
  }

  return $branch;
}

function avbase_account_chart_sum_parent_balance(&$tree) {
  $x = $tree;
  foreach ($x as $k => $row) {
    $row = (object) $row;
    $tree[$k]['children_account_balance'] = 0;
    if (!empty($row->children)) {
      foreach ($row->children as $ch) {
        $tree[$k]['children_account_balance'] += $ch['account_balance'];
        //$tree[$k]['account_balance'] += $ch['account_balance'];
      }
      avbase_account_chart_sum_parent_balance($tree[$k]['children']);

      // Update this parents children_account_balance with total from children's children.
      foreach ($tree[$k]['children'] as $c_k => $c_row) {
        $tree[$k]['children_account_balance'] += $c_row['children_account_balance'];
      }
    }
  }
}

/**
 * @param $rows
 * @param $field_name
 * @param $search_string
 * @return array
 */
function avbase_order_db_rows_by_best_match($rows, $field_name, $search_string = '') {
  if (empty($search_string)) {
    return $rows;
  }

  $grouped_rows = array();
  $string_arr = explode(' ', $search_string);
  foreach ($rows as $row) {
    //if (strtolower($row->$field_name) == strtolower($search_string)) {
    if (strpos(strtolower($row->$field_name), strtolower($search_string)) === 0) {
      // Prioritize exact match.
      $grouped_rows[0][] = $row;
    }
    else {
      if (count($string_arr) == 1) {
        // No exact match. Only one word is searched
        if (stripos($row->$field_name, $string_arr[0] . ' ') === 0) {
          // Prioritize first word of $field_name if it matches "$string ".
          $grouped_rows[1][] = $row;
          continue;
        }
        elseif (stripos($row->$field_name, $string_arr[0]) === 0) {
          // Prioritize first word of $field_name if it matches "$string".
          $grouped_rows[2][] = $row;
          continue;
        }
      }

      $all_words_found = TRUE;
      foreach ($string_arr as $s) {
        if (stripos($row->$field_name, $s . ' ') === FALSE && stripos($row->$field_name, ' ' . $s) === FALSE) {
          $all_words_found = FALSE;
          break;
        }
      }
      if ($all_words_found) {
        $grouped_rows[3][] = $row;
      }
      else {
        $grouped_rows[4][] = $row;
      }
    }
  }
  $ordered_rows = array();
  ksort($grouped_rows);
  foreach ($grouped_rows as $gr) {
    $ordered_rows = array_merge($ordered_rows, $gr);
  }
  return $ordered_rows;
}

function avbase_accounting_number_format($number) {
  $rounded = round((float)$number, 2);
  $text = number_format($rounded, 2);
  if ($rounded < 0) {
    $text = '(' . number_format(($rounded * -1), 2) . ')';
  }
  return $text;
}

function avbase_set_window_title($title = NULL, $output = CHECK_PLAIN) {
  $stored_title = &drupal_static(__FUNCTION__);
  if (isset($title)) {
    $stored_title = ($output == PASS_THROUGH) ? $title : check_plain($title);
  }

  return $stored_title;
}

function avbase_txn_number_format($type, $id) {
  return strtoupper($type) . '# ' . $id;
}

function avbase_query_supplements(&$query, $where = array(), $order_by = array(), $limit = NULL, $offset = NULL, $group_by = array()) {
  if (!empty($where)) {
    $query .= " WHERE " . implode(' AND ', $where);
  }
  if (!empty($group_by)) {
    $query .= " GROUP BY " . implode(', ', $group_by);
  }
  if (!empty($order_by)) {
    $query .= " ORDER BY " . implode(', ', $order_by);
  }
  if (!is_null($limit)) {
    $query .= " LIMIT $limit";
    if (!is_null($offset)) {
      $query .= " OFFSET $offset";
    }
  }
}

function avbase_query_supplements_stock_status(&$query, $where = array(), $order_by = array(), $limit = NULL, $offset = NULL, $group_by = array(), $joins, $all_products, $args) {
  // Limit by product ids instead of limit value.
  if (!empty($limit)) {
    if ($all_products) {
      unset($joins['avtxn']);
      unset($joins['avtxn_detail']);
      unset($args[':void_type']);
      unset($args[':ttypes']);
      unset($args[':date_to_stamp']);
      unset($args[':date_from_stamp']);
    }
    $product_query = "SELECT avprod.id FROM {avtbl_products} AS avprod " . implode(' ', $joins);
    avbase_query_supplements($product_query, $where, array('id' => 'id ASC'), $limit, $offset);
    $product_ids = db_query($product_query, $args)->fetchAllKeyed(0, 0);
    if ($product_ids) {
      $where[] = 'avprod.id IN (' . implode(',', $product_ids) . ')';
      $limit = NULL;
    }
  }

  if (!empty($where)) {
    $query .= " WHERE " . implode(' AND ', $where);
  }
  if (!empty($group_by)) {
    $query .= " GROUP BY " . implode(', ', $group_by);
  }
  if (!empty($order_by)) {
    $query .= " ORDER BY " . implode(', ', $order_by);
  }
  if (!is_null($limit)) {
    $query .= " LIMIT $limit";
    if (!is_null($offset)) {
      $query .= " OFFSET $offset";
    }
  }
}
