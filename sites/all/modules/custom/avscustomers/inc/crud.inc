<?php

/**
 * New Customer page.
 * @return string
 */
function avscustomers_add() {
  return drupal_get_form('avscustomers_add_form');
}

function avscustomers_add_form($form, &$form_state) {

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
  );
  $info['email'] = array(
    '#type' => 'textfield',
    '#title' => 'E-mail address',
    '#maxlength' => 255,
  );
  $info['contact_number'] = array(
    '#type' => 'textfield',
    '#title' => 'Contact number',
    '#maxlength' => 32,
  );
  $info['website'] = array(
    '#type' => 'textfield',
    '#title' => 'Website',
    '#maxlength' => 255,
  );
  $info['address'] = array(
    '#type' => 'textfield',
    '#title' => 'Address',
    '#maxlength' => 255,
  );
  $info['city'] = array(
    '#type' => 'textfield',
    '#title' => 'City / Town',
    '#maxlength' => 64,
  );
  $info['province'] = array(
    '#type' => 'textfield',
    '#title' => 'Province',
    '#maxlength' => 64,
  );
  $info['zip_code'] = array(
    '#type' => 'textfield',
    '#title' => 'ZIP code',
    '#maxlength' => 16,
  );
  $info['payment_method_id'] = array(
    '#type' => 'select',
    '#title' => 'Preferred payment method',
    '#options' => array(1, 2, 3)
  );

  $form = $info;

  $form['buttons']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  return $form;
}
