<?php
module_load_include('inc', 'avtrans', 'inc/po.handlers');
module_load_include('inc', 'avtrans', 'inc/item_list.form');

/**
 * Load po form.
 * @param object $row
 *    PO row object.
 *
 * @return string
 */
function avtrans_po_form_load($row) {
  return drupal_get_form('avtrans_po_form', $row);
}

/**
 * PO form.
 *
 * @param $form
 * @param $form_state
 * @param $row
 * @return array
 */
function avtrans_po_form($form, &$form_state, $row) {
  $form['id'] = array('#type' => 'value', '#value' => isset($row->id) ? $row->id : NULL);
  switch (arg(4)) {
    case 'view':
      $page_title = t('Purchase order details');
      break;
    case 'edit':
      $page_title = t('Edit purchase order');
      break;
    default:
      $page_title = t('New purchase order');
      break;
  }
  drupal_set_title($page_title);

  // Set crud settings.
  avtrans_po_form_settings($form, $row);

  // Item List form fields.
  avtrans_item_list_form_fields($form, $form_state, $row);

  $form['header_total'] = array(
    '#markup' => '<div><div class="uk-text-bold uk-text-uppercase">AMOUNT</div><div><h1 class="product-form-grand-total">0.00</h1></div></div>',
  );
  $form['footer_total'] = array(
    '#markup' => '<div><div class="uk-text-bold uk-text-uppercase">AMOUNT</div><div><h1 class="product-form-grand-total">0.00</h1></div></div>',
  );

  // Set common fields when purchasing items.
  avtrans_purchase_fields($form, $form_state, $row);

  $form['item_list_date']['#title'] = t('P.O. Date');
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

  $form['buttons']['#theme'] = 'avbase_crud_button_group';
  $form['buttons']['submit_and_send'] = array(
    '#id' => 'submit_and_send',
    '#type' => 'submit',
    '#value' => t('Save and send'),
  );
  $form['buttons']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  $form['buttons']['cancel'] = array(
    '#markup' => l('Cancel', 'av/transactions'),
  );

  // Convert form to non-edit mode if we are on View page.
  avbase_allow_form_view_mode($form);
  return $form;
}

/**
 * Set crud form settings.
 * @param $form
 */
function avtrans_po_form_settings(&$form, $row) {
  $js_settings = array('avtrans' => array('transaction' => 'purchase'));
  $form['#attached']['js'][] = array(
    'data' => $js_settings,
    'type' => 'setting',
  );

  // Get form storage.
  $form_state['storage'] = isset($form_state['storage']) ? $form_state['storage'] : array();
  $_storage = &$form_state['storage'];
  $_storage['transaction'] = $js_settings['avtrans']['transaction'];

  // Table name.
  $table_name = 'avtbl_po';

  // Set flag if this is view mode.
  $form['#av_view_mode'] = (arg(4) == 'view');

  // Set generic submit handler.
  $form['#submit'] = array('avbase_crud_form_submit', 'avtrans_po_form_submit');

  $form['table_name'] = array('#type' => 'value', '#value' => $table_name);
  $form['entity_name'] = array('#type' => 'value', '#value' => 'po');
  $form['#success_message'] = t('Purchase order details saved.');
  $form['#theme'] = 'avtrans_item_list_form';

  //$po_js_settings = array('avTransactionPODetails' => $row);
  //$form['#attached']['js'][] = array(
  //  'data' => $po_js_settings,
  //  'type' => 'setting',
  //);
  //$form['#attached']['css'] = array(
  //  //AVBASE_LIBRARIES_PATH . '/select2/select2.min.css',
  //);

  // Set autocomplete off.
  $form['#attributes']['autocomplete'] = 'off';

  // Set field names to save.
  $save_field_ids = array(
    'created',
    'client_id',
    'message',
    'email',
    //'term_id' => array('type' => 'int'),
    'discount_type' => array('type' => 'int'),
    'discount_value' => array('type' => 'float'),
  );
  if (empty($form['id']['#value'])) {
    $save_field_ids[] = 'transaction_date';
  }

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
