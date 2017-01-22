<?php

/**
 * New Vendor page.
 * @param object $row
 *    Vendor row object.
 *
 * @return string
 */
function avvendors_vendor_form_load($row, $view_mode = FALSE) {
  return drupal_get_form('avvendors_vendor_form', $row, $view_mode);
}

function avvendors_vendor_form($form, &$form_state, $row, $view_mode = FALSE) {
  $form['id'] = array('#type' => 'value', '#value' => isset($row->id) ? $row->id : NULL);
  $page_title = empty($form['id']['#value']) ? 'New vendor' : 'Edit vendor';
  if (!$view_mode) {
    drupal_set_title($page_title);
  }

  // Set crud settings.
  avvendors_vendor_form_settings($form, $view_mode);

  // Info group.
  avvendors_vendor_form_info_fields($form, $row, $view_mode);

  // Contact group.
  avvendors_vendor_form_contact_fields($form, $row, $view_mode);

  // Address group.
  avvendors_vendor_form_address_fields($form, $row, $view_mode);

  // Transaction group.
  avvendors_vendor_form_transaction_fields($form, $form_state, $row, $view_mode);

  // Miscellaneous.
  $form['misc'] = array(
    '#type' => 'fieldset',
    '#title' => 'Miscellaneous',
  );
  $form['misc']['notes'] = array(
    '#type' => 'textarea',
    '#title' => 'Miscellaneous',
    '#default_value' => isset($row->data['notes']) ? $row->data['notes'] : '',
    '#maxlength' => 255,
  );

  $form['buttons']['#theme'] = 'avbase_crud_button_group';
  $form['buttons']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  $form['buttons']['cancel'] = array(
    '#markup' => l('Cancel', 'av/vendors'),
  );

  // Convert form to non-edit mode if we are on View page.
  avbase_allow_form_view_mode($form, $view_mode);
  return $form;
}

/**
 * Set crud form settings.
 * @param $form
 */
function avvendors_vendor_form_settings(&$form, $view_mode = FALSE) {
  $table_name = 'avtbl_vendors';

  // Set generic submit handler.
  $form['#submit'] = array('avbase_crud_form_submit');
  $form['table_name'] = array('#type' => 'value', '#value' => $table_name);
  $form['entity_name'] = array('#type' => 'value', '#value' => 'vendor');

  $form['#attached']['js'] = array(
    AVVENDORS_MODULE_PATH . '/js/vendor_form.js',
  );

  // Set autocomplete off.
  $form['#attributes']['autocomplete'] = 'off';

  // Set field names to save.
  $db_op = empty($form['id']['#value']) ? 'insert' : 'update';
  $save_field_ids = avvendors_table_fields_to_save($db_op);
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
 * New vendor - generate info fields.
 * @param $form
 */
function avvendors_vendor_form_info_fields(&$form, $row, $view_mode = FALSE) {
  $form['info'] = array(
    '#type' => 'fieldset',
    '#title' => 'Info',
  );
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
  $info['category_id'] = array(
    '#type' => 'select',
    '#title' => 'Category',
    '#options' => avbase_get_category_options_array('vendor_cat'),
    '#empty_value' => 0,
    '#empty_option' => '- Select -',
    '#default_value' => isset($row->category_id) ? $row->category_id : 0,
  );
  $info['display_name'] = array(
    '#type' => 'textfield',
    '#title' => 'Display name',
    '#title_display' => $view_mode ? 'none' : 'before',
    '#default_value' => isset($row->display_name) ? $row->display_name : '',
    '#maxlength' => 255,
    '#required' => TRUE,
    '#element_validate' => array('avbase_element_validate_display_name'),
  );
  $info['agent_name'] = array(
    '#id' => 'agent-name',
    '#type' => 'textfield',
    '#title' => t('Agent'),
    '#maxlength' => 255,
    '#autocomplete_path' => 'av/categories/agent_vend/autocomplete',
    '#weight' => 1.2,
    '#element_validate' => array('avbase_element_validate_category_exists'),
    '#element_validate_conditions' => array('group_id' => 'agent_vend'),
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
  $info['opening_balance'] = array(
    '#type' => 'textfield',
    '#title' => 'Opening balance',
    '#default_value' => isset($row->opening_balance) ? round($row->opening_balance, 6) : '',
    '#maxlength' => 19,
    '#element_validate' => array('element_validate_number', 'avbase_element_validate_money_length'),
  );
  if (!empty($row->id)) {
    $info['opening_balance']['#disabled'] = TRUE;
  }
  $form['info'] += $info;
}

/**
 * New vendor - generate contact fields.
 * @param $form
 */
function avvendors_vendor_form_contact_fields(&$form, $row, $view_mode = FALSE) {
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
    '#default_value' => isset($row->data['contact_number']) ? $row->data['contact_number'] : '',
    '#maxlength' => 32,
  );
  $contact['website'] = array(
    '#type' => 'textfield',
    '#title' => 'Website',
    '#default_value' => isset($row->data['website']) ? $row->data['website'] : '',
    '#maxlength' => 255,
  );
  $form['contact'] += $contact;
}

/**
 * New vendor - generate address fields.
 * @param $form
 */
function avvendors_vendor_form_address_fields(&$form, $row, $view_mode = FALSE) {
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
  $form['address'] += $address;
}

/**
 * New vendor - generate transaction fields.
 * @param $form
 */
function avvendors_vendor_form_transaction_fields(&$form, &$form_state, $row, $view_mode = FALSE) {
  if (empty($row->id) || empty($view_mode)) {
    return;
  }

  $form_state['values']['transaction_list_filter']['filter_query']['where'][] = 'avtrans.client_id = :client_id';
  $form_state['values']['transaction_list_filter']['filter_query']['args'][':client_id'] = $row->id;
  $form['transactions'] = avtrans_transaction_list_form($form, $form_state);
  $form['transactions']['transaction_list_filter']['search_text']['#access'] = FALSE;
  //$form['transactions']['transaction_list_filter']['search_text']['#attributes']['placeholder'] = 'ss';
  $form['transactions']['transaction_list_filter']['#access'] = arg(2) != '';

  //return;
  //
  //$transactions = array();
  //
  //$conditions = array(
  //  array('avpo.client_id', $row->id)
  //);
  //$orders = avtrans_po_query_load($conditions, 0, 8);
  //$conditions = array(
  //  array('avgr.client_id', $row->id)
  //);
  //$received = avtrans_gr_query_load($conditions, 0, 8);
  ////dpm($received);
  //
  //$table_rows = array();
  //foreach ($orders as $k => $order) {
  //  $table_cols = array();
  //  $table_cols['type'] = array(
  //    '#markup' => 'Purchase order',
  //    '#column_attributes' => array('class' => array('uk-width-2-5')),
  //  );
  //  $table_cols['transaction_num'] = array(
  //    '#markup' => $order->id,
  //    '#column_attributes' => array('class' => array('uk-width-2-5')),
  //  );
  //  $table_cols['transaction_date'] = array(
  //    '#markup' => format_date($order->transaction_date, 'custom', 'Y M d, h:i a'),
  //    '#column_attributes' => array('class' => array('uk-width-1-5')),
  //  );
  //
  //  $table_rows[$k] = $table_cols;
  //  $table_rows[$k]['#row_attributes'] = array('class' => array('uk-table-middle'));
  //}
  //if (!empty($table_rows)) {
  //  $table = array(
  //    '#theme' => 'avbase_form_table',
  //    '#header' => array(
  //      array('data' => t('Type')),
  //      array('data' => t('Transaction #')),
  //      array('data' => t('Date')),
  //      '&nbsp;',
  //    ),
  //  );
  //  $table['rows'] = $table_rows;
  //  $table['#attributes'] = array('class' => array('uk-table-striped', 'uk-table-condensed'));
  //
  //  //$fields['product_cost_changes'] += $pcfields;
  //  //$fields['intro'] = array('#weight' => -5, '#markup' => '<p>Some product costs have been changed. Do you want to update?</p>');
  //  $transactions['table'] = $table;
  //}
  //else {
  //  $transactions = array('#markup' => '<div class="uk-text-muted uk-text-center">No transactions yet</div>');
  //}
  //
  //$form['transactions'] = array();
  //$form['transactions'] += $transactions;
}


/**
 * Menu callback: confirm vendor delete.
 * @param $form
 * @param $form_state
 * @param object $row
 *
 * @return array
 */
function avvendors_vendor_delete_confirm_form($form, &$form_state, $row) {
  $prompt = 'Are you sure you want to delete this vendor?';
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
 * Handler for vendor delete.
 *
 * @see avvendors_vendor_delete_confirm_form()
 */
function avvendors_vendor_delete_confirm_form_submit($form, &$form_state) {
  //if (isset($form_state['row']->id)) {
  //  if (avvendors_vendor_delete($form_state['row']->id)) {
  //    drupal_set_message('Record deleted.');
  //  }
  //}
  drupal_set_message(t('Record delete is disabled'), 'error');
  $form_state['redirect'] = '';
}
