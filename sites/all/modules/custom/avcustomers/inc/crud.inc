<?php

/**
 * New Customer page.
 * @param object $row
 *    Customer row object.
 *
 * @return string
 */
function avcustomers_customer_form_load($row) {
  return drupal_get_form('avcustomers_customer_form', $row);
}

/**
 * Customers form.
 *
 * @param $form
 * @param $form_state
 * @param $row
 *
 * @return array
 */
function avcustomers_customer_form($form, &$form_state, $row) {
  $form['id'] = array('#type' => 'value', '#value' => isset($row->id) ? $row->id : NULL);
  $page_title = empty($form['id']['#value']) ? 'New customer' : 'Edit customer';
  drupal_set_title($page_title);

  // Set crud settings.
  avcustomers_customer_form_settings($form);

  // Info group.
  avcustomers_customer_form_info_fields($form, $row);

  // Contact group.
  avcustomers_customer_form_contact_fields($form, $row);

  // Address group.
  avcustomers_customer_form_address_fields($form, $row);

  // Payment group.
  avcustomers_customer_form_payment_fields($form, $row);

  // Miscellaneous.
  $form['misc'] = array(
    '#type' => 'fieldset',
    '#title' => 'Miscellaneous',
  );
  $form['misc']['notes'] = array(
    '#type' => 'textarea',
    '#title' => 'Miscellaneous',
    '#default_value' => isset($row->data['notes']) ? check_plain($row->data['notes']) : '',
    '#maxlength' => 255,
  );

  $form['buttons']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  $form['buttons']['cancel'] = array(
    '#markup' => l('Cancel', 'av/customers'),
  );
  return $form;
}

/**
 * Set crud form settings.
 * @param $form
 */
function avcustomers_customer_form_settings(&$form) {
  $table_name = 'avtbl_customers';

  // Set generic submit handler.
  $form['#submit'] = array('avbase_crud_form_submit');
  $form['table_name'] = array('#type' => 'value', '#value' => $table_name);

  // Set field names to save.
  $save_field_ids = array(
    'created',
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
    'payment_method_id' => array('type' => 'int'),
  );
  $form['save_field_ids'] = array(
    '#type' => 'value',
    '#value' => $save_field_ids,
  );

  // Set which fields belong to data field.
  $data_ids = array(
    //'title',
    'contact_number',
    'website',
    'notes',
  );
  $form['data_ids'] = array(
    '#type' => 'value',
    '#value' => $data_ids,
  );
}

/**
 * New customer - generate info fields.
 * @param $form
 */
function avcustomers_customer_form_info_fields(&$form, $row) {
  $form['info'] = array(
    '#type' => 'fieldset',
    '#title' => 'Info',
  );
//  $info['title'] = array(
//    '#type' => 'textfield',
//    '#title' => 'Title',
//    '#default_value' => isset($row->data['title']) ? check_plain($row->data['title']) : '',
//    '#maxlength' => 32,
//  );
  $info['first_name'] = array(
    '#type' => 'textfield',
    '#title' => 'First name',
    '#default_value' => isset($row->first_name) ? check_plain($row->first_name) : '',
    '#maxlength' => 32,
  );
  $info['middle_name'] = array(
    '#type' => 'textfield',
    '#title' => 'Middle name',
    '#default_value' => isset($row->middle_name) ? check_plain($row->middle_name) : '',
    '#maxlength' => 32,
  );
  $info['last_name'] = array(
    '#type' => 'textfield',
    '#title' => 'Last name',
    '#default_value' => isset($row->last_name) ? check_plain($row->last_name) : '',
    '#maxlength' => 32,
  );
  $info['company'] = array(
    '#type' => 'textfield',
    '#title' => 'Company',
    '#default_value' => isset($row->company) ? check_plain($row->company) : '',
    '#maxlength' => 255,
  );
  $info['display_name'] = array(
    '#type' => 'textfield',
    '#title' => 'Display name as',
    '#default_value' => isset($row->display_name) ? check_plain($row->display_name) : '',
    '#maxlength' => 255,
    '#required' => TRUE,
  );
  $info['credit_limit'] = array(
    '#type' => 'textfield',
    '#title' => 'Credit limit',
    '#default_value' => isset($row->credit_limit) ? avbase_money_format('%01.2f', $row->credit_limit) : '',
    '#maxlength' => 19,
    '#element_validate' => array('element_validate_number'),
  );
  $form['info'] += $info;
}

/**
 * New customer - generate contact fields.
 * @param $form
 */
function avcustomers_customer_form_contact_fields(&$form, $row) {
  $form['contact'] = array(
    '#type' => 'fieldset',
    '#title' => 'Contact details',
  );
  $contact['email'] = array(
    '#type' => 'textfield',
    '#title' => 'E-mail address',
    '#default_value' => isset($row->email) ? check_plain($row->email) : '',
    '#maxlength' => 255,
  );
  $contact['contact_number'] = array(
    '#type' => 'textfield',
    '#title' => 'Contact number',
    '#default_value' => isset($row->data['contact_number']) ? check_plain($row->data['contact_number']) : '',
    '#maxlength' => 32,
  );
  $contact['website'] = array(
    '#type' => 'textfield',
    '#title' => 'Website',
    '#default_value' => isset($row->data['website']) ? check_plain($row->data['website']) : '',
    '#maxlength' => 255,
  );
  $form['contact'] += $contact;
}

/**
 * New customer - generate address fields.
 * @param $form
 */
function avcustomers_customer_form_address_fields(&$form, $row) {
  $form['address'] = array(
    '#type' => 'fieldset',
    '#title' => 'Address',
  );
  $address['address'] = array(
    '#type' => 'textarea',
    '#title' => 'Address',
    '#default_value' => isset($row->address) ? check_plain($row->address) : '',
    '#maxlength' => 255,
  );
  $address['city'] = array(
    '#type' => 'textfield',
    '#title' => 'City / Town',
    '#default_value' => isset($row->city) ? check_plain($row->city) : '',
    '#maxlength' => 64,
  );
  $address['province'] = array(
    '#type' => 'textfield',
    '#title' => 'Province',
    '#default_value' => isset($row->province) ? check_plain($row->province) : '',
    '#maxlength' => 64,
  );
  $address['zip_code'] = array(
    '#type' => 'textfield',
    '#title' => 'ZIP code',
    '#default_value' => isset($row->zip_code) ? check_plain($row->zip_code) : '',
    '#maxlength' => 16,
  );
  $address['same_addr'] = array(
    '#type' => 'checkbox',
    '#title' => 'Billing address same as shipping address',
    '#default_value' => isset($row->same_addr) ? (empty($row->same_addr) ? 0 : 1) : '',
  );
  $form['address'] += $address;
}

/**
 * New customer - generate payment fields.
 * @param $form
 */
function avcustomers_customer_form_payment_fields(&$form, $row) {
  $form['payment'] = array(
    '#type' => 'fieldset',
    '#title' => 'Payment',
  );

  $payment['payment_method_id'] = array(
    '#type' => 'select',
    '#title' => 'Preferred payment method',
    '#options' => avbase_get_category_options_array('payment_method'),
  );
  $form['payment'] += $payment;
}

/**
 * Menu callback: confirm customer delete.
 * @param $form
 * @param $form_state
 * @param object $row
 *
 * @return array
 */
function avcustomers_customer_delete_confirm_form($form, &$form_state, $row) {
  $prompt = 'Are you sure you want to delete this customer?';
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
 * Handler for customer delete.
 *
 * @see avcustomers_customer_delete_confirm_form()
 */
function avcustomers_customer_delete_confirm_form_submit($form, &$form_state) {
  if (isset($form_state['row']->id)) {
    if (avcustomers_customer_delete($form_state['row']->id)) {
      drupal_set_message('Record deleted.');
    }
  }
  $form_state['redirect'] = '';
}
