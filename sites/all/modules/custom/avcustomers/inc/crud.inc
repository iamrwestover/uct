<?php

/**
 * New Customer page.
 * @return string
 */
function avcustomers_new() {
  return drupal_get_form('avcustomers_new_form');
}

function avcustomers_new_form($form, &$form_state) {
//  $form['customer_info'] = array('#type' => 'vertical_tabs');
  // Info group.
  avcustomers_new_form_info_fields($form);

  // Contact group.
  avcustomers_new_form_contact_fields($form);

  // Address group.
  avcustomers_new_form_address_fields($form);

  // Payment group.
  avcustomers_new_form_payment_fields($form);

  // Miscellaneous.
  $form['misc'] = array(
    '#type' => 'fieldset',
    '#title' => 'Miscellaneous',
  );
  $form['misc']['notes'] = array(
    '#type' => 'textarea',
    '#title' => 'Notes',
    '#maxlength' => 255,
  );

  $form['buttons']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  return $form;
}

/**
 * Submit handler to avcustomers_new_form().
 * @param $form
 * @param $form_state
 */
function avcustomers_new_form_submit($form, &$form_state) {
  $values = $form_state['values'];
//  $schema = drupal_get_schema('avtbl_customers');
//  $fields = $schema['fields'];
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
    'same_addr',
    'payment_method_id',
  );

  $row = array();
  foreach ($save_field_ids as $field_id) {
    $row[$field_id] = isset($values[$field_id]) ? $values[$field_id] : NULL;
  }

  // Get values for "data" field.
  $data_ids = array(
    'title',
    'phone_number',
    'notes',
  );
  $row['data'] = array();
  foreach ($data_ids as $data_id) {
    $row['data'][$data_id] = isset($values[$data_id]) ? $values[$data_id] : NULL;
  }
  $row['data'] = serialize($row['data']);

  // Save timestamp when record is created.
  $row['created'] = time();

  // Do insert.
  db_insert('avtbl_customers')
    ->fields($row)
    ->execute();
  drupal_set_message('New customer saved.');
}

/**
 * New customer - generate info fields.
 * @param $form
 */
function avcustomers_new_form_info_fields(&$form) {
  $form['info'] = array(
    '#type' => 'fieldset',
    '#title' => 'Info',
  );
  $info['title'] = array(
    '#type' => 'textfield',
    '#title' => 'Title',
    '#maxlength' => 32,
  );
  $info['first_name'] = array(
    '#type' => 'textfield',
    '#title' => 'First name',
    '#maxlength' => 32,
  );
  $info['middle_name'] = array(
    '#type' => 'textfield',
    '#title' => 'Middle name',
    '#maxlength' => 32,
  );
  $info['last_name'] = array(
    '#type' => 'textfield',
    '#title' => 'Last name',
    '#maxlength' => 32,
  );
  $info['company'] = array(
    '#type' => 'textfield',
    '#title' => 'Company',
    '#maxlength' => 255,
  );
  $info['display_name'] = array(
    '#type' => 'textfield',
    '#title' => 'Display name as',
    '#maxlength' => 255,
    '#required' => TRUE,
  );
  $form['info'] += $info;
}

/**
 * New customer - generate contact fields.
 * @param $form
 */
function avcustomers_new_form_contact_fields(&$form) {
  $form['contact'] = array(
    '#type' => 'fieldset',
    '#title' => 'Contact details',
  );
  $contact['email'] = array(
    '#type' => 'textfield',
    '#title' => 'E-mail address',
    '#maxlength' => 255,
  );
  $contact['contact_number'] = array(
    '#type' => 'textfield',
    '#title' => 'Contact number',
    '#maxlength' => 32,
  );
  $contact['website'] = array(
    '#type' => 'textfield',
    '#title' => 'Website',
    '#maxlength' => 255,
  );
  $form['contact'] += $contact;
}

/**
 * New customer - generate address fields.
 * @param $form
 */
function avcustomers_new_form_address_fields(&$form) {
  $form['address'] = array(
    '#type' => 'fieldset',
    '#title' => 'Address',
  );
  $address['address'] = array(
    '#type' => 'textarea',
    '#title' => 'Address',
    '#maxlength' => 255,
  );
  $address['city'] = array(
    '#type' => 'textfield',
    '#title' => 'City / Town',
    '#maxlength' => 64,
  );
  $address['province'] = array(
    '#type' => 'textfield',
    '#title' => 'Province',
    '#maxlength' => 64,
  );
  $address['zip_code'] = array(
    '#type' => 'textfield',
    '#title' => 'ZIP code',
    '#maxlength' => 16,
  );
  $address['same_addr'] = array(
    '#type' => 'checkbox',
    '#title' => 'Billing address same as shipping address',
    '#default_value' => 1,
  );
  $form['address'] += $address;
}

/**
 * New customer - generate payment fields.
 * @param $form
 */
function avcustomers_new_form_payment_fields(&$form) {
  $form['payment'] = array(
    '#type' => 'fieldset',
    '#title' => 'Payment',
  );

  $payment['payment_method_id'] = array(
    '#type' => 'select',
    '#title' => 'Preferred payment method',
    '#options' => avbase_get_payment_method_options_array(),
  );
  $form['payment'] += $payment;
}
