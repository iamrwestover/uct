<?php
module_load_include('inc', 'avtrans', 'inc/item_list.form');
module_load_include('inc', 'avtrans', 'inc/trans.cost_changes.form');
//module_load_include('inc', 'avtrans', 'inc/trans.client_changes.form');
module_load_include('inc', 'avtrans', 'inc/trans.pending_orders.form');
module_load_include('inc', 'avtrans', 'inc/gr/gr.handlers');

/**
 * Load "Receive items" form.
 * @param object $row
 *    RI row object.
 *
 * @return string
 */
function avtrans_gr_form_load($row, $view_mode = FALSE) {
  return drupal_get_form('avtrans_gr_form', $row, $view_mode);
}

/**
 * "Receive items" form.
 *
 * @param $form
 * @param $form_state
 * @param $row
 * @return array
 */
function avtrans_gr_form($form, &$form_state, $row, $view_mode = FALSE) {
  $form['#id'] = 'avtrans-gr-form';
  //$form['#attached']['js'][] = AVTRANS_MODULE_PATH . '/js/avtrans.form.js';
  $form['#prefix'] = '<div id="avtrans-transaction-form-wrapper"><!-- Do not remove me. -->';
  $form['#suffix'] = '</div>';

  // Set crud settings.
  avtrans_gr_form_settings($form, $form_state, $row, $view_mode);

  // Add generic Goods Received form fields.
  avtrans_gr_form_default($form, $form_state, $row, $view_mode);

  // Add fields for Cost Changes popup.
  avtrans_cost_changes_form_fields($form, $form_state, $row, $view_mode);

  // Add fields for Client Changes popup.
  //avtrans_client_changes_form_fields($form, $form_state, $row);

  // Add fields for Vendor POs popup.
  avtrans_pending_orders_form_fields($form, $form_state, $row, $view_mode);

  // Convert form to non-edit mode if we are on View page.
  avbase_allow_form_view_mode($form, $view_mode);
  return $form;
}

/**
 * Default Goods Received form fields.
 * @param $form
 * @param $form_state
 * @param $row
 *
 * @return array
 */
function avtrans_gr_form_default(&$form, &$form_state, $row, $view_mode = FALSE) {
  $form['id'] = array('#type' => 'value', '#value' => isset($row->id) ? $row->id : NULL);
  $page_title = empty($row->id) ? t('Receive items') : t('Received items');
  drupal_set_title($page_title);


  $form['#theme'] = 'avtrans_item_list_form';

  // Goods Received fields.
  avtrans_gr_form_fields($form, $form_state, $row, $view_mode);

  // Item List form fields.
  avtrans_item_list_form_fields($form, $form_state, $row, $view_mode);

  // Set common fields when purchasing items.
  avtrans_purchase_fields($form, $form_state, $row, $view_mode);

  // Buttons.
  avtrans_gr_form_buttons($form, $form_state, $row, $view_mode);
}

/**
 * Set crud form settings.
 * @param $form
 * @param $form_state
 * @param $row
 */
function avtrans_gr_form_settings(&$form, &$form_state, $row, $view_mode = FALSE) {
  $js_settings = array('avtrans' => array('transaction' => 'purchase'));
  $form['#attached']['js'][] = array(
    'data' => $js_settings,
    'type' => 'setting',
  );

  // Get form storage.
  $form_state['storage'] = isset($form_state['storage']) ? $form_state['storage'] : array();
  $_storage = &$form_state['storage'];
  $_storage['transaction'] = $js_settings['avtrans']['transaction'];
  $_storage['transaction_type'] = 'gr';

  // Set generic submit handler.
  //$form['#submit'] = array('avtrans_gr_form_submit', 'avtrans_redirect_transaction_form');

  $table_name = 'avtbl_gr';
  $form['table_name'] = array('#type' => 'value', '#value' => $table_name);
  $form['entity_name'] = array('#type' => 'value', '#value' => 'gr');
  $form['#success_message'] = t('Item receipt details saved.');

  // Set autocomplete off.
  $form['#attributes']['autocomplete'] = 'off';

  // Set field names to save.
  $save_field_ids = array(
    'created',
    'client_id',
    'reference_id',
    'memo',
    'term_id' => array('type' => 'int'),
    'agent_id' => array('type' => 'int'),
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

/**
 * Generic GR form fields.
 * @param $form
 * @param $form_state
 * @param $row
 */
function avtrans_gr_form_fields(&$form, &$form_state, $row, $view_mode = FALSE) {
  //$term_options = avbase_get_category_options_array('payment_term');
  //asort($term_options);
  //$form['term_id'] = array(
  //  '#id' => 'term-id',
  //  '#type' => 'select',
  //  '#title' => 'Payment term',
  //  '#options' => $term_options,
  //  '#default_value' => isset($row->term_id) ? $row->term_id : 0,
  //  '#empty_value' => 0,
  //  '#empty_option' => '',
  //  '#avbase_payment_terms_js' => array('preventChildrenHide' => TRUE),
  //);
  $form['reference_id'] = array(
    '#type' => 'textfield',
    '#title' => 'Reference ID',
    '#default_value' => isset($row->reference_id) ? $row->reference_id : '',
    '#maxlength' => 32,
    '#required' => TRUE,
  );
}

