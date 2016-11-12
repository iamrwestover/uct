<?php

/**
 * New Product page.
 * @param object $row
 *    Product row object.
 *
 * @return string
 */
function avproducts_product_form_load($row) {
  return drupal_get_form('avproducts_product_form', $row);
}

function avproducts_product_form($form, &$form_state, $row) {
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
 * New product - generate info fields.
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
  );
  $info['title'] = array(
    '#type' => 'textfield',
    '#title' => 'Product code',
    '#default_value' => isset($row->code) ? check_plain($row->code) : '',
    '#maxlength' => 255,
  );

  $form['info'] += $info;
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
      'first_name',
      'last_name',
      'middle_name',
      'company',
      'display_name',
      'email',
      'address',
      'city',
      'province',
      'zip_code',
    );

    // Get $form_state values and store them on $row array.
    $row = array();
    foreach ($save_field_ids as $field_id) {
      $row[$field_id] = isset($values[$field_id]) ? $values[$field_id] : NULL;
    }

    // Get values for "data" field.
    $data_ids = array(
      'title',
      'contact_number',
      'website',
      'notes',
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
  $display_name = check_plain($row->display_name);
  $company = check_plain($row->company);
  $description = <<<HTML
  <h1 class="uk-article-title">$display_name</h1>$company
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
