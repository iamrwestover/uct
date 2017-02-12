<?php

/**
 * Clients form.
 *
 * @param $form
 * @param $form_state
 * @param int $client_type
 * @param $row
 * @param bool $view_mode
 *
 * @return array
 */
function avclients_client_form($form, &$form_state, $client_type, $row, $view_mode = FALSE) {
  if (!empty($row->id) && $row->client_type != $client_type) {
    drupal_not_found();
    drupal_exit();
  }

  $form['id'] = array('#type' => 'value', '#value' => isset($row->id) ? $row->id : NULL);

  $ctd = avclients_client_types($client_type);

  // Set crud settings.
  avclients_client_form_settings($form, $ctd, $row, $view_mode);

  // Info group.
  avclients_client_form_info_fields($form, $ctd, $row, $view_mode);

  // Contact group.
  avclients_client_form_contact_fields($form, $ctd, $row, $view_mode);

  // Address group.
  avclients_client_form_address_fields($form, $ctd, $row, $view_mode);

  // Payment group.
  avclients_client_form_payment_fields($form, $ctd, $row, $view_mode);

  // Transaction group.
  avclients_vendor_form_transaction_fields($form, $ctd, $form_state, $row, $view_mode);

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
    '#icon_key' => 'save',
  );
  $form['buttons']['cancel'] = array(
    '#markup' => l('Cancel', $ctd['base_path'] , array('attributes' => array('class' => array('uk-button')))),
  );
  if (!empty($row->id) && $row->client_type == AVCLIENT_TYPE_CUSTOMER) {
    if ($row->status != AVCLIENT_STATUS_SUSPENDED) {
      $form['buttons']['suspend'] = array(
        '#markup' => l('Suspend', $ctd['base_path'] . '/' . $row->id . '/2/change-status', array(
          'attributes' => array('class' => 'uk-button'),
          'query' => array('destination' => $ctd['base_path']),
        )),
        '#avposition' => 'opposite',
      );
    }
    else {
      $form_prefix = empty($form['#prefix']) ? '' : $form['#prefix'];
      $form_prefix = '<div class="uk-alert uk-alert-warning"><span class="uk-badge uk-badge-warning" style="position: absolute;top:10px;left:10px;">Suspended Account</span>' . $form_prefix;
      $form['#prefix'] = $form_prefix;
      $form_suffix = empty($form['#suffix']) ? '' : $form['#suffix'];
      $form_suffix .= '</div>';
      $form['#suffix'] = $form_suffix;
      //drupal_set_message('This account is currently suspended.', 'error', FALSE);
      $form['buttons']['activate'] = array(
        '#markup' => l('Activate', $ctd['base_path'] . '/' . $row->id . '/1/change-status', array(
          'attributes' => array('class' => 'uk-button uk-button-primary'),
          'query' => array('destination' => $ctd['base_path']),
        )),
        '#avposition' => 'opposite',
      );
    }
  }

  // Convert form to non-edit mode if we are on View page.
  avbase_allow_form_view_mode($form, $view_mode);
  return $form;
}

/**
 * Set crud form settings.
 * @param $form
 * @param array $ctd
 * @param object $row
 * @param bool $view_mode
 */
function avclients_client_form_settings(&$form, $ctd, $row, $view_mode = FALSE) {
  $table_name = 'avtbl_clients';

  if (arg(2) != '') {
    $page_title = empty($row->id) ? t('New @type', array('@type' => $ctd['name'])) : $row->display_name;
    drupal_set_title($page_title);
  }

  // Set generic submit handler.
  $form['#submit'] = array('avbase_crud_form_submit');
  $form['table_name'] = array('#type' => 'value', '#value' => $table_name);
  $form['client_type'] = array('#type' => 'value', '#value' => $ctd['type']);
  $form['entity_name'] = array('#type' => 'value', '#value' => 'client');

  //$form['#attached']['js'] = array(
  //  AVCLIENTS_MODULE_PATH . '/js/client_form.js',
  //);

  // Set autocomplete off.
  $form['#attributes']['autocomplete'] = 'off';

  // Set field names to save.
  $db_op = empty($form['id']['#value']) ? 'insert' : 'update';
  $save_field_ids = avclients_table_fields_to_save($db_op);
  $form['save_field_ids'] = array(
    '#type' => 'value',
    '#value' => $save_field_ids,
  );
  // Set which fields belong to data field.
  $data_ids = empty($save_field_ids['data']) ? array() : $save_field_ids['data'];
  $form['data_ids'] = array(
    '#type' => 'value',
    '#value' => $data_ids,
  );
}

/**
 * New client - generate info fields.
 * @param $form
 */
function avclients_client_form_info_fields(&$form, $ctd, $row, $view_mode = FALSE) {
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
    '#default_value' => isset($row->first_name) ? $row->first_name : '',
    '#maxlength' => 32,
  );
  $info['middle_name'] = array(
    '#type' => 'textfield',
    '#title' => 'Middle name',
    '#default_value' => isset($row->middle_name) ? $row->middle_name : '',
    '#maxlength' => 32,
  );
  $info['last_name'] = array(
    '#type' => 'textfield',
    '#title' => 'Last name',
    '#default_value' => isset($row->last_name) ? $row->last_name : '',
    '#maxlength' => 32,
  );
  $info['company'] = array(
    '#type' => 'textfield',
    '#title' => 'Company',
    '#default_value' => isset($row->company) ? $row->company : '',
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
    '#options' => avbase_get_category_options_array("category" . $ctd['type']),
    '#empty_value' => 0,
    '#empty_option' => '- Select -',
    '#default_value' => isset($row->category_id) ? $row->category_id : 0,
  );
  $area_options = avbase_get_category_options_array('area');
  asort($area_options);
  $info['area_id'] = array(
    '#type' => 'select',
    '#title' => 'Area',
    '#options' => $area_options,
    '#empty_value' => 0,
    '#empty_option' => '- Select -',
    '#default_value' => isset($row->area_id) ? $row->area_id : 0,
    '#access' => $ctd['type'] == AVCLIENT_TYPE_CUSTOMER,
  );
  //$info['code'] = array(
  //  '#type' => 'textfield',
  //  '#title' => 'Client code',
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
    '#title_display' => $view_mode ? 'none' : 'before',
    '#default_value' => isset($row->display_name) ? $row->display_name : '',
    '#maxlength' => 255,
    '#required' => TRUE,
    '#element_validate' => array('avbase_element_validate_client_name'),
  );
  $info['agent_name'] = array(
    '#id' => 'agent-name',
    '#type' => 'textfield',
    '#title' => t('Agent'),
    '#maxlength' => 255,
    '#autocomplete_path' => "av/categories/agent{$ctd['type']}/autocomplete",
    '#element_validate' => array('avbase_element_validate_category_exists'),
    '#element_validate_conditions' => array('group_id' => "agent{$ctd['type']}"),
    '#element_save_id' => 'agent_id',
    '#default_value' => empty($row->agent_name) ? '' : $row->agent_name,
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
    '#element_validate' => array('element_validate_number', 'avbase_element_validate_money_length'),
    '#special_element_type' => 'number',
    '#attributes' => array(
      //'class' => array(''),
      'step' => 0.01,
    ),
  );
  $form['info'] += $info;
}

/**
 * New client - generate contact fields.
 * @param $form
 */
function avclients_client_form_contact_fields(&$form, $ctd, $row, $view_mode = FALSE) {
  $form['contact'] = array(
    '#type' => 'fieldset',
    '#title' => 'Contact details',
  );
  $contact['email'] = array(
    '#type' => 'textfield',
    '#title' => 'E-mail address',
    '#default_value' => isset($row->email) ? $row->email : '',
    '#maxlength' => 255,
    '#element_validate' => array('avbase_element_validate_email'),
  );
  $contact['contact_number'] = array(
    '#type' => 'textfield',
    '#title' => 'Contact number',
    '#default_value' => isset($row->contact_number) ? $row->contact_number : '',
    '#maxlength' => 32,
  );
  //$contact['phone2'] = array(
  //  '#type' => 'textfield',
  //  '#title' => 'Phone 2',
  //  '#default_value' => isset($row->data['phone2']) ? $row->data['phone2'] : '',
  //  '#maxlength' => 32,
  //);
  //$contact['phone3'] = array(
  //  '#type' => 'textfield',
  //  '#title' => 'Phone 3',
  //  '#default_value' => isset($row->data['phone3']) ? $row->data['phone3'] : '',
  //  '#maxlength' => 32,
  //);
  $contact['website'] = array(
    '#type' => 'textfield',
    '#title' => 'Website',
    '#default_value' => isset($row->website) ? $row->website : '',
    '#maxlength' => 255,
    '#access' => $ctd['type'] == AVCLIENT_TYPE_VENDOR,
  );
  $form['contact'] += $contact;
}

/**
 * New client - generate address fields.
 * @param $form
 */
function avclients_client_form_address_fields(&$form, $ctd, $row, $view_mode = FALSE) {
  $form['address'] = array(
    '#type' => 'fieldset',
    '#title' => 'Address',
  );
  $address['address'] = array(
    '#type' => 'textarea',
    '#title' => 'Address',
    '#default_value' => isset($row->address) ? $row->address : '',
    '#maxlength' => 255,
  );
  $address['city'] = array(
    '#type' => 'textfield',
    '#title' => 'City / Town',
    '#default_value' => isset($row->city) ? $row->city : '',
    '#maxlength' => 64,
  );
  $address['province'] = array(
    '#type' => 'textfield',
    '#title' => 'Province',
    '#default_value' => isset($row->province) ? $row->province : '',
    '#maxlength' => 64,
  );
  $address['zip_code'] = array(
    '#type' => 'textfield',
    '#title' => 'ZIP code',
    '#default_value' => isset($row->zip_code) ? $row->zip_code : '',
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
 * New client - generate payment fields.
 * @param $form
 */
function avclients_client_form_payment_fields(&$form, $ctd, $row, $view_mode = FALSE) {
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
    '#access' => $ctd['type'] == AVCLIENT_TYPE_CUSTOMER,
  );
  $payment['credit_limit'] = array(
    '#type' => 'textfield',
    '#title' => 'Credit limit',
    '#default_value' => isset($row->credit_limit) ? ($view_mode ? number_format($row->credit_limit, 2) : avbase_number_format($row->credit_limit)) : '',
    '#maxlength' => 19,
    '#element_validate' => array('element_validate_number', 'avbase_element_validate_money_length'),
    '#access' => $ctd['type'] == AVCLIENT_TYPE_VENDOR,
  );
  $payment['opening_balance'] = array(
    '#type' => 'textfield',
    '#title' => 'Opening balance',
    '#default_value' => isset($row->opening_balance) ? ($view_mode ? number_format($row->opening_balance, 2) : avbase_number_format($row->opening_balance)) : '',
    '#maxlength' => 19,
    '#element_validate' => array('element_validate_number', 'avbase_element_validate_money_length'),
  );
  if (!empty($row->id)) {
    $payment['opening_balance']['#disabled'] = TRUE;
  }
  $form['payment'] += $payment;
}

/**
 * Menu callback: confirm client delete.
 * @param $form
 * @param $form_state
 * @param object $row
 *
 * @return array
 */
function avclients_client_delete_confirm_form($form, &$form_state, $row) {
  $prompt = 'Are you sure you want to delete this client?';
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
 * Handler for client delete.
 *
 * @see avclients_client_delete_confirm_form()
 */
function avclients_client_delete_confirm_form_submit($form, &$form_state) {
  //if (isset($form_state['row']->id)) {
  //  if (avclients_client_delete($form_state['row']->id)) {
  //    drupal_set_message('Record deleted.');
  //  }
  //}
  drupal_set_message(t('Record delete is disabled'), 'error');
  $form_state['redirect'] = '';
}

/**
 * Menu callback: confirm client status change.
 * @param $form
 * @param $form_state
 * @param object $row
 * @param $status
 *
 * @return array
 */
function avclients_client_change_status_confirm_form($form, &$form_state, $row, $status) {
  switch ($status) {
    case AVCLIENT_STATUS_DISABLED:
      $prompt = 'Are you sure you want to disable this client\'s account?';
      break;
    case AVCLIENT_STATUS_SUSPENDED:
      $prompt = 'Are you sure you want to suspend this client\'s account?';
      break;
    case AVCLIENT_STATUS_ACTIVE:
      $prompt = 'Are you sure you want to activate this client\'s account?';
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
 * Handler for client status change.
 *
 * @see avclients_client_change_status_confirm_form()
 */
function avclients_client_change_status_confirm_form_submit($form, &$form_state) {
  if (isset($form_state['row']->id)) {
    $status = isset($form_state['status']) ? $form_state['status'] : '';
    if (avclients_client_change_status($form_state['row']->id, $status)) {

      switch ($status) {
        case AVCLIENT_STATUS_DISABLED:
          //drupal_set_message('Client\'s account has been disabled.');
          break;
        case AVCLIENT_STATUS_SUSPENDED:
          //drupal_set_message('Client\'s account has been suspended.');
          break;
        case AVCLIENT_STATUS_ACTIVE:
          drupal_set_message('Client\'s account has been activated.');
          break;
        default:
          break;
      }
    }
  }
  $form_state['redirect'] = '';
}

/**
 *
 * @param $form
 */
function avclients_vendor_form_transaction_fields(&$form, &$form_state, $ctd, $row, $view_mode = FALSE) {
  if (empty($row->id) || empty($view_mode)) {
    return;
  }

  $options = array(
    '#hide_filter_form' => TRUE,
    '#filter_query' => array(
      'where' => array('avtxn.client_id = :client_id'),
      'args' => array(':client_id' => $row->id),
    ),
  );
  $form['transactions'] = avtxns_txn_list_form($form, $form_state, $options);
}