<?php

/**
 * New Customer page.
 * @return string
 */
function avscustomers_add() {
  return drupal_get_form('avscustomers_add_form');
}

function avscustomers_add_form($form, &$form_state) {
  $form['customer_info'] = array('#type' => 'vertical_tabs');
  // Info group.
  $form['info'] = array(
    '#type' => 'fieldset',
    '#title' => 'Info',
    '#group' => 'customer_info',
  );
  $info['title'] = array(
    '#type' => 'textfield',
    '#title' => 'Title',
    '#maxlength' => 32,
    '#group' => 'customer_info',
  );
  $info['first_name'] = array(
    '#type' => 'textfield',
    '#title' => 'First name',
    '#maxlength' => 32,
    '#group' => 'customer_info',
  );
  $info['middle_name'] = array(
    '#type' => 'textfield',
    '#title' => 'Middle name',
    '#maxlength' => 32,
    '#group' => 'customer_info',
  );
  $info['last_name'] = array(
    '#type' => 'textfield',
    '#title' => 'Last name',
    '#maxlength' => 32,
    '#group' => 'customer_info',
  );
  $info['company'] = array(
    '#type' => 'textfield',
    '#title' => 'Company',
    '#maxlength' => 255,
    '#group' => 'customer_info',
  );
  $info['display_name'] = array(
    '#type' => 'textfield',
    '#title' => 'Display name as',
    '#maxlength' => 255,
    '#required' => TRUE,
    '#group' => 'customer_info',
  );
  $form['info'] += $info;

  // Contact group.
  $form['contact'] = array(
    '#type' => 'fieldset',
    '#title' => 'Contact details',
    '#group' => 'customer_info',
  );
  $contact['email'] = array(
    '#type' => 'textfield',
    '#title' => 'E-mail address',
    '#maxlength' => 255,
    '#group' => 'customer_info',
  );
  $contact['contact_number'] = array(
    '#type' => 'textfield',
    '#title' => 'Contact number',
    '#maxlength' => 32,
    '#group' => 'customer_info',
  );
  $contact['website'] = array(
    '#type' => 'textfield',
    '#title' => 'Website',
    '#maxlength' => 255,
    '#group' => 'customer_info',
  );
  $form['contact'] += $contact;

  // Address group.
  $form['address'] = array(
    '#type' => 'fieldset',
    '#title' => 'Address',
    '#group' => 'customer_info',
  );
  $address['address'] = array(
    '#type' => 'textarea',
    '#title' => 'Address',
    '#maxlength' => 255,
    '#group' => 'customer_info',
  );
  $address['city'] = array(
    '#type' => 'textfield',
    '#title' => 'City / Town',
    '#maxlength' => 64,
    '#group' => 'customer_info',
  );
  $address['province'] = array(
    '#type' => 'textfield',
    '#title' => 'Province',
    '#maxlength' => 64,
    '#group' => 'customer_info',
  );
  $address['zip_code'] = array(
    '#type' => 'textfield',
    '#title' => 'ZIP code',
    '#maxlength' => 16,
    '#group' => 'customer_info',
  );
  $form['address'] += $address;

  // Payment group.
  $form['payment'] = array(
    '#type' => 'fieldset',
    '#title' => 'Payment',
    '#group' => 'customer_info',
  );
  $payment['payment_method_id'] = array(
    '#type' => 'select',
    '#title' => 'Preferred payment method',
    '#options' => array(1, 2, 3),
    '#group' => 'customer_info',
  );
  $form['payment'] += $payment;

  $form['buttons']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}
