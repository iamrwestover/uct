<?php
module_load_include('inc', 'avtxns', 'inc/txn.handlers');
module_load_include('inc', 'avtxns', 'inc/item_list.form');

/**
 * PO form.
 *
 * @param array $form
 * @param array $form_state
 * @param string $txn_type
 * @param object $row
 * @param bool $view_mode
 * @return array
 */
function avtxns_txn_form($form, &$form_state, $txn_type, $row, $view_mode = FALSE) {
  $form['id'] = array('#type' => 'value', '#value' => isset($row->id) ? $row->id : NULL);
  $page_title = empty($row->id) ? t('New purchase order') : t('Purchase order');
  drupal_set_title($page_title);

  // Set crud settings.
  avtxns_txn_form_settings($form, $form_state, $txn_type, $row, $view_mode);

  // Item List form fields.
  avtxns_item_list_form_fields($form, $form_state, $txn_type, $row, $view_mode);

  $form['header_total'] = array(
    '#markup' => '<div><div class="uk-text-bold uk-text-uppercase">AMOUNT</div><div><h1 class="product-form-grand-total">0.00</h1></div></div>',
  );
  $form['footer_total'] = array(
    '#markup' => '<div><div class="uk-text-bold uk-text-uppercase">AMOUNT</div><div><h1 class="product-form-grand-total">0.00</h1></div></div>',
  );

  // Set common fields when purchasing items.
  avtxns_purchase_fields($form, $form_state, $txn_type, $row, $view_mode);

  //$form['transaction_date']['#title'] = t('P.O. Date');
  $form['email'] = array(
    '#id' => 'vendor-email',
    '#type' => 'textfield',
    '#title' => 'E-mail address',
    '#default_value' => isset($row->email) ? check_plain($row->email) : '',
    '#maxlength' => 255,
    '#element_validate' => array('avbase_element_validate_email'),
  );
  $form['message'] = array(
    '#type' => 'textarea',
    '#title' => 'Message to vendor',
    '#default_value' => isset($row->message) ? check_plain($row->message) : '',
    '#maxlength' => 65000,
  );
  $form['address'] = array(
    '#type' => 'item',
    '#title' => 'Shipping address',
    '#markup' => nl2br(check_plain(variable_get('av_company_address', ''))),
    //'#maxlength' => 65000,
  );

  $form['buttons'] = avtxns_transaction_buttons($txn_type, $view_mode);

  // Convert form to non-edit mode if we are on View page.
  avbase_allow_form_view_mode($form, $view_mode);
  return $form;
}

/**
 * Set crud form settings.
 * @param array $form
 * @param array $form_state
 * @param string $txn_type
 * @param object $row
 * @param bool $view_mode
 */
function avtxns_txn_form_settings(&$form, &$form_state, $txn_type, $row, $view_mode = FALSE) {
  $js_settings = array('avtxns' => array('transaction' => 'purchase'));
  $form['#attached']['js'][] = array(
    'data' => $js_settings,
    'type' => 'setting',
  );

  // Get form storage.
  $form_state['storage'] = isset($form_state['storage']) ? $form_state['storage'] : array();
  $_storage = &$form_state['storage'];
  $_storage['transaction'] = $js_settings['avtxns']['transaction'];

  // Table name.
  $table_name = 'avtbl_txns';

  // Set generic submit handler.
  $form['#submit'] = array('avbase_crud_form_submit', 'avtxns_txn_form_submit', 'avtxns_redirect_transaction_form');

  $form['table_name'] = array('#type' => 'value', '#value' => $table_name);
  $form['entity_name'] = array('#type' => 'value', '#value' => 'txn');
  $form['#success_message'] = t('Purchase order details saved.');
  $form['#theme'] = 'avtxns_item_list_form';
  
  // Set autocomplete off.
  $form['#attributes']['autocomplete'] = 'off';

  // Set field names to save.
  $save_field_ids = array(
    'created',
    'client_id',
    'message',
    'email',
    'memo',
    'agent_id' => array('type' => 'int'),
    //'term_id' => array('type' => 'int'),
    //'discount_type' => array('type' => 'int'),
    //'discount_value' => array('type' => 'float'),
  );

  $form['save_field_ids'] = array(
    '#type' => 'value',
    '#value' => $save_field_ids,
  );

  // Set which fields belong to data field.
  $data_ids = array(
    //'uoms' => array('type' => 'array'),
  );
  $form['data_ids'] = array(
    '#type' => 'value',
    '#value' => $data_ids,
  );
}
