<?php

/**
 * Load product form.
 * @param object $row
 *    Product row object.
 *
 * @return string
 */
function avproducts_product_form_load($row) {
  return drupal_get_form('avproducts_product_form', $row);
}

/**
 * Product form.
 *
 * @param $form
 * @param $form_state
 * @param $row
 * @return array
 */
function avproducts_product_form($form, &$form_state, $row) {
  drupal_set_title('New product');
  $form['id'] = array('#type' => 'value', '#value' => isset($row->id) ? $row->id : NULL);

  // Info group.
  avproducts_product_form_info_fields($form, $row);

  $form['buttons']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  $form['buttons']['cancel'] = array(
    '#markup' => l('Cancel', 'av/products'),
  );
  return $form;
}

/**
 * Product form - generate info fields.
 * @param $form
 */
function avproducts_product_form_info_fields(&$form, $row) {
  $form['info'] = array(
    '#type' => 'fieldset',
    '#title' => 'Info',
  );
  $info['title'] = array(
    '#type' => 'textfield',
    '#title' => 'Title',
    '#default_value' => isset($row->title) ? check_plain($row->title) : '',
    '#maxlength' => 255,
    '#required' => TRUE,
  );
  $info['code'] = array(
    '#type' => 'textfield',
    '#title' => 'Product code',
    '#default_value' => isset($row->code) ? check_plain($row->code) : '',
    '#maxlength' => 32,
  );
  $info['category_id'] = array(
    '#type' => 'select',
    '#title' => 'Category',
    '#options' => avbase_get_category_options_array(AVCATEGORY_ID_PRODUCT_CATEGORY),
  );
  $info['description'] = array(
    '#type' => 'textarea',
    '#title' => 'Description',
    '#default_value' => isset($row->description) ? check_plain($row->description) : '',
    '#maxlength' => 65000,
  );
  $info['qty'] = array(
    '#type' => empty($row->id) ? 'textfield' : 'item',
    '#title' => 'Quantity',
//    '#default_value' => isset($row->qty) ? $row->qty : 0,
    '#maxlength' => 10,
    '#access' => !empty($row->id),
    '#element_validate' => array('avproducts_qty_validate'),
  );
  if (!empty($row->id)) {
    $info['qty']['#markup'] = (isset($row->qty) ? $row->qty : 0) . ' ' . l('( adjust quantity )', '#');
  }
  $info['initial_qty'] = array(
    '#type' => 'textfield',
    '#title' => 'Beginning quantity',
    '#default_value' => isset($row->qty) ? $row->qty : '',
    '#maxlength' => 10,
    '#access' => empty($row->id),
    '#required' => TRUE,
    '#element_validate' => array('element_validate_integer_positive'),
  );
  $initial_qty_date_format = 'M. d, Y';
  $info['initial_qty_date'] = array(
    '#type' => 'textfield',
    '#title' => 'Beginning quantity start date',
    '#default_value' => isset($row->initial_qty_date) ? date($initial_qty_date_format, $row->initial_qty_date) : '',
    '#access' => empty($row->id),
    '#required' => TRUE,
    '#element_validate' => array('avbase_element_validate_date'),
    '#av_date_format' => $initial_qty_date_format,
    '#attributes' => array(
      'autocomplete' => 'off',
    ),
  );
  $info['uom_id'] = array(
    '#type' => 'select',
    '#title' => 'Unit of measure',
    '#options' => avbase_get_category_options_array(AVCATEGORY_ID_UNIT_OF_MEASURE),
  );
  $info['rop'] = array(
    '#type' => 'textfield',
    '#title' => 'Reorder point',
    '#default_value' => isset($row->rop) ? $row->rop : '',
    '#maxlength' => 10,
    '#element_validate' => array('element_validate_integer_positive'),
  );
  $info['eoq'] = array(
    '#type' => 'textfield',
    '#title' => 'EOQ',
    '#default_value' => isset($row->eoq) ? $row->eoq : '',
    '#maxlength' => 10,
    '#element_validate' => array('element_validate_integer_positive'),
  );
  $info['price'] = array(
    '#type' => 'textfield',
    '#title' => 'Sales price',
    '#default_value' => isset($row->price) ? avbase_money_format('%01.2f', $row->price) : '',
    '#maxlength' => 19,
    '#element_validate' => array('element_validate_number'),
  );
  $info['cost'] = array(
    '#type' => 'textfield',
    '#title' => 'Unit cost',
    '#default_value' => isset($row->cost) ? avbase_money_format('%01.2f', $row->cost) : '',
    '#maxlength' => 19,
    '#element_validate' => array('element_validate_number'),
  );
  $form['info'] += $info;
}

/**
 * Validate qty field.
 * @param $form
 * @param $form_state
 * @param $form
 */
function avproducts_qty_validate($element, &$form_state, $form) {
  // If this is a new record, set qty to be the same as initial qty.
  if (empty($form_state['values']['id'])) {
    $form_state['values']['qty'] = $form_state['values']['initial_qty'];
    $element['#value'] = $form_state['values']['qty'];
  }
  element_validate_integer_positive($element, $form_state);
}

/**
 * Submit handler to avproducts_product_form().
 * @param $form
 * @param $form_state
 *
 * @throws
 */
function avproducts_product_form_submit($form, &$form_state) {
  $transaction = db_transaction();
  try {
    $values = $form_state['values'];
    $record_id = isset($values['id']) ? $values['id'] : NULL;
    $save_field_ids = array(
      'code',
      'title',
      'category_id',
      'uom_id',
      'description',
      'qty',
      'rop',
      'eoq',
      'cost',
      'price',
    );
    if (empty($record_id)) {
      $save_field_ids[] = 'initial_qty';
      $save_field_ids[] = 'initial_qty_date';
    }

    // Get $form_state values and store them on $row array.
    $row = array();
    foreach ($save_field_ids as $field_id) {
      if (!isset($values[$field_id]) || trim($values[$field_id]) == '') {
        continue;
      }
      $row[$field_id] = $values[$field_id];
    }

    // Get values for "data" field.
    $data_ids = array(

    );
    $row['data'] = array();
    foreach ($data_ids as $data_id) {
      $row['data'][$data_id] = isset($values[$data_id]) ? $values[$data_id] : NULL;
    }
    $row['data'] = serialize($row['data']);

    // Do save.
    if (empty($record_id) || !is_numeric($record_id)) {
      // Insert new record.
      // Save timestamp when record is created.
      $row['created'] = time();
      if (db_insert('avtbl_products')->fields($row)->execute()) {
        drupal_set_message('New product saved.');
      }
    }
    else {
      // Update record.
      if (db_update('avtbl_products')->fields($row)->condition('id', $record_id)->execute()) {
        drupal_set_message('Changes saved.');
      }
    }
  }
  catch (Exception $e) {
    $transaction->rollback();
//    watchdog_exception('avproducts', $e);
    throw $e;
  }
}

/**
 * Menu callback: confirm product delete.
 * @param $form
 * @param $form_state
 * @param object $row
 *
 * @return array
 */
function avproducts_product_delete_confirm_form($form, &$form_state, $row) {
  $prompt = 'Are you sure you want to delete this product?';
  $row_title = check_plain($row->title);
  $row_description = check_plain($row->description);
  $description = <<<HTML
  <h1 class="uk-article-title">$row_title</h1>$row_description
  <hr />
  <p>This action cannot be undone.</p>
HTML;
  $form_state['row'] = $row;
  return confirm_form(array(), $prompt, '', $description);
}

/**
 * Handler for product delete.
 *
 * @see avproducts_product_delete_confirm_form()
 */
function avproducts_product_delete_confirm_form_submit($form, &$form_state) {
  if (isset($form_state['row']->id)) {
    if (avproducts_product_delete($form_state['row']->id)) {
      drupal_set_message('Record deleted.');
    }
  }
  $form_state['redirect'] = '';
}