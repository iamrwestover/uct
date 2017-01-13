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
  switch (arg(3)) {
    case 'view':
      $page_title = $row->display_name;
      break;
    case 'edit':
      $page_title = $row->display_name . ' - ' . t('Edit');
      break;
    default:
      $page_title = t('New customer');
      break;
  }
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
  //$form['misc']['notes'] = array(
  //  '#type' => 'textarea',
  //  '#title' => 'Miscellaneous',
  //  '#default_value' => isset($row->data['notes']) ? check_plain($row->data['notes']) : '',
  //  '#maxlength' => 255,
  //);

  $form['buttons']['#theme'] = 'avbase_crud_button_group';
  $form['buttons']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  $form['buttons']['cancel'] = array(
    '#markup' => l('Cancel', 'av/customers'),
  );
  if (!empty($row->id)) {
    if ($row->status != AVCUSTOMER_STATUS_SUSPENDED) {
      $form['buttons']['suspend'] = array(
        '#markup' => l('Suspend', 'av/customers/' . $row->id . '/2/change-status', array(
          'attributes' => array('class' => 'uk-button'),
          'query' => drupal_get_destination(),
        )),
        '#avposition' => 'opposite',
      );
    }
    else {
      drupal_set_message('This account is currently suspended.', 'error', FALSE);
      $form['buttons']['activate'] = array(
        '#markup' => l('Activate', 'av/customers/' . $row->id . '/1/change-status', array(
          'attributes' => array('class' => 'uk-button uk-button-primary'),
          'query' => drupal_get_destination(),
        )),
        '#avposition' => 'opposite',
      );
    }
  }

  // Convert form to non-edit mode if we are on View page.
  $form['#av_view_mode'] = (arg(3) == 'view');
  avbase_allow_form_view_mode($form);
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
  $form['entity_name'] = array('#type' => 'value', '#value' => 'customer');

  $form['#attached']['js'] = array(
    AVCUSTOMERS_MODULE_PATH . '/js/customer_form.js',
  );

  // Set autocomplete off.
  $form['#attributes']['autocomplete'] = 'off';

  // Set field names to save.
  $save_field_ids = array(
    'created',
    'first_name',
    'last_name',
    'middle_name',
    'company',
    'display_name',
    //'code',
    'agent_id' => array('type' => 'int'),
    'email',
    'address',
    'city',
    'province',
    'zip_code',
    //'same_addr',
    'payment_method_id' => array('type' => 'int'),
    'term_id' => array('type' => 'int'),
    'discount_type' => array('type' => 'int'),
    'discount_value' => array('type' => 'float'),
    'category_id' => array('type' => 'int'),
    'credit_limit' => array('type' => 'float'),
  );
  if (empty($form['id']['#value'])) {
    $save_field_ids['opening_balance'] = array('type' => 'float');
  }
  $form['save_field_ids'] = array(
    '#type' => 'value',
    '#value' => $save_field_ids,
  );

  // Set which fields belong to data field.
  $data_ids = array(
    //'title',
    'phone1',
    'phone2',
    'phone3',
    //'website',
    //'notes',
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
  $info['_created'] = array(
    '#type' => 'textfield',
    '#title' => 'Date added',
    '#default_value' => empty($row->id) ? '' : format_date($row->created, 'custom', 'F d, Y'),
    '#disabled' => TRUE,
    '#access' => !empty($row->id),
  );
  $info['category_id'] = array(
    '#type' => 'select',
    '#title' => 'Category',
    '#options' => avbase_get_category_options_array('customer_cat'),
    '#empty_value' => 0,
    '#empty_option' => '- Select -',
    '#default_value' => isset($row->category_id) ? $row->category_id : 0,
  );
  //$info['code'] = array(
  //  '#type' => 'textfield',
  //  '#title' => 'Customer code',
  //  '#default_value' => isset($row->code) ? check_plain($row->code) : '',
  //  '#maxlength' => 32,
  //  '#element_validate' => array('avbase_element_validate_unique_field'),
  //  '#attributes' => array(
  //    'class' => array('uk-text-uppercase'),
  //  ),
  //);
  $info['display_name'] = array(
    '#type' => 'textfield',
    '#title' => 'Display name',
    '#default_value' => isset($row->display_name) ? check_plain($row->display_name) : '',
    '#maxlength' => 255,
    '#required' => TRUE,
    '#element_validate' => array('avbase_element_validate_display_name'),
  );
  $info['agent_id'] = array(
    '#id' => 'agent-id',
    '#type' => 'textfield',
    '#title' => t('Agent'),
    '#maxlength' => 255,
    '#autocomplete_path' => 'av/categories/agent_cust/autocomplete',
    '#weight' => 1.2,
    //'#required' => TRUE,
    '#element_validate' => array('avbase_element_validate_category_exists'),
    '#element_validate_conditions' => array('group_id' => 'agent_cust'),
    '#default_value' => empty($row->agent_name) ? '' : check_plain($row->agent_name),
  );

  $term_options = avbase_get_category_options_array('payment_term');
  asort($term_options);
  $info['term_id'] = array(
    '#id' => 'term-id',
    '#type' => 'select',
    '#title' => 'Payment term',
    '#options' => $term_options,
    '#default_value' => isset($row->term_id) ? $row->term_id : 0,
    '#empty_value' => 0,
    '#empty_option' => '- Select -',
    '#avbase_payment_terms_js' => array('preventChildrenHide' => FALSE),
  );
  $info['discount_type'] = array(
    '#id' => 'discount-type',
    '#type' => 'select',
    '#title' => 'Discount type',
    '#options' => avbase_get_discount_types(),
    '#default_value' => isset($row->discount_type) ? $row->discount_type : 1,
  );
  $info['discount_value'] = array(
    '#id' => 'discount-value',
    '#type' => 'textfield',
    '#title' => 'Discount value',
    '#default_value' => isset($row->discount_value) ? round($row->discount_value, 2) : '',
    '#maxlength' => 19,
    '#element_validate' => array('element_validate_number'),
    '#special_element_type' => 'number',
    '#attributes' => array(
      //'class' => array(''),
      'step' => 0.01,
    ),
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
    '#element_validate' => array('avbase_element_validate_email'),
  );
  $contact['phone1'] = array(
    '#type' => 'textfield',
    '#title' => 'Phone 1',
    '#default_value' => isset($row->data['phone1']) ? check_plain($row->data['phone1']) : '',
    '#maxlength' => 32,
  );
  $contact['phone2'] = array(
    '#type' => 'textfield',
    '#title' => 'Phone 2',
    '#default_value' => isset($row->data['phone2']) ? check_plain($row->data['phone2']) : '',
    '#maxlength' => 32,
  );
  $contact['phone3'] = array(
    '#type' => 'textfield',
    '#title' => 'Phone 3',
    '#default_value' => isset($row->data['phone3']) ? check_plain($row->data['phone3']) : '',
    '#maxlength' => 32,
  );
  //$contact['website'] = array(
  //  '#type' => 'textfield',
  //  '#title' => 'Website',
  //  '#default_value' => isset($row->data['website']) ? check_plain($row->data['website']) : '',
  //  '#maxlength' => 255,
  //);
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
  //$address['same_addr'] = array(
  //  '#type' => 'checkbox',
  //  '#title' => 'Billing address same as shipping address',
  //  '#default_value' => isset($row->same_addr) ? (empty($row->same_addr) ? 0 : 1) : '',
  //);
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
    '#default_value' => isset($row->payment_method_id) ? $row->payment_method_id : 0,
    '#empty_value' => 0,
    '#empty_option' => '- Select -',
  );
  $payment['credit_limit'] = array(
    '#type' => 'textfield',
    '#title' => 'Credit limit',
    '#default_value' => isset($row->credit_limit) ? round($row->credit_limit, 6) : '',
    '#maxlength' => 19,
    '#element_validate' => array('element_validate_number'),
  );
  $payment['opening_balance'] = array(
    '#type' => 'textfield',
    '#title' => 'Opening balance',
    '#default_value' => isset($row->opening_balance) ? round($row->opening_balance, 6) : '',
    '#maxlength' => 19,
    '#element_validate' => array('element_validate_number'),
  );
  if (!empty($row->id)) {
    $payment['opening_balance']['#disabled'] = TRUE;
  }
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

/**
 * Menu callback: confirm customer status change.
 * @param $form
 * @param $form_state
 * @param object $row
 * @param $status
 *
 * @return array
 */
function avcustomers_customer_change_status_confirm_form($form, &$form_state, $row, $status) {
  switch ($status) {
    case AVCUSTOMER_STATUS_DISABLED:
      $prompt = 'Are you sure you want to disable this customer\'s account?';
      break;
    case AVCUSTOMER_STATUS_SUSPENDED:
      $prompt = 'Are you sure you want to suspend this customer\'s account?';
      break;
    case AVCUSTOMER_STATUS_ACTIVE:
      $prompt = 'Are you sure you want to activate this customer\'s account?';
      break;
    default:
      $prompt = 'Are you sure?';
      break;
  }

  $display_name = check_plain($row->display_name);
  $company = check_plain($row->company);
  $description = <<<HTML
  <h1 class="uk-article-title">$display_name</h1>$company
HTML;
  $form_state['row'] = $row;
  $form_state['status'] = $status;
  return confirm_form(array(), $prompt, '', $description);
}

/**
 * Handler for customer status change.
 *
 * @see avcustomers_customer_change_status_confirm_form()
 */
function avcustomers_customer_change_status_confirm_form_submit($form, &$form_state) {
  if (isset($form_state['row']->id)) {
    $status = isset($form_state['status']) ? $form_state['status'] : '';
    if (avcustomers_customer_change_status($form_state['row']->id, $status)) {

      switch ($status) {
        case AVCUSTOMER_STATUS_DISABLED:
          //drupal_set_message('Customer\'s account has been disabled.');
          break;
        case AVCUSTOMER_STATUS_SUSPENDED:
          //drupal_set_message('Customer\'s account has been suspended.');
          break;
        case AVCUSTOMER_STATUS_ACTIVE:
          drupal_set_message('Customer\'s account has been activated.');
          break;
        default:
          break;
      }
    }
  }
  $form_state['redirect'] = '';
}
