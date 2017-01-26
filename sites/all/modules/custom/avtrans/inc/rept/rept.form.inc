<?php
module_load_include('inc', 'avtrans', 'inc/item_list.form');
module_load_include('inc', 'avtrans', 'inc/trans.cost_changes.form');
//module_load_include('inc', 'avtrans', 'inc/trans.client_changes.form');
module_load_include('inc', 'avtrans', 'inc/trans.pending_orders.form');
module_load_include('inc', 'avtrans', 'inc/rept/rept.handlers');
module_load_include('inc', 'avtrans', 'inc/gr/gr.handlers');

/**
 * rept form.
 *
 * @param $form
 * @param $form_state
 * @param $row
 * @return array
 */
function avtrans_rept_form($form, &$form_state, $row, $view_mode = FALSE) {
  $form['#id'] = 'avtrans-rept-form';
  //$form['#attached']['js'][] = AVTRANS_MODULE_PATH . '/js/avtrans.form.js';
  $form['#prefix'] = '<div id="avtrans-transaction-form-wrapper"><!-- Do not remove me. -->';
  $form['#suffix'] = '</div>';

  // Set crud settings.
  avtrans_rept_form_settings($form, $form_state, $row, $view_mode);

  // Add generic rept form fields.
  avtrans_rept_form_default($form, $form_state, $row, $view_mode);

  // Add fields for Cost Changes popup.
  //avtrans_cost_changes_form_fields($form, $form_state, $row, $view_mode);

  // Add fields for Customer SOs popup.
  //avtrans_pending_orders_form_fields($form, $form_state, $row, $view_mode);

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
function avtrans_rept_form_default(&$form, &$form_state, $row, $view_mode = FALSE) {
  $form['id'] = array('#type' => 'value', '#value' => isset($row->id) ? $row->id : NULL);
  $page_title = empty($row->id) ? t('New Purchase Return') : t('Purchase Return');
  drupal_set_title($page_title);

  $form['#theme'] = 'avtrans_item_list_form';

  // Goods Received fields.
  avtrans_rept_form_fields($form, $form_state, $row, $view_mode);

  // Item List form fields.
  avtrans_item_list_form_fields($form, $form_state, $row, $view_mode);

  // Set common fields when purchasing items.
  avtrans_purchase_fields($form, $form_state, $row, $view_mode);

  // Buttons.
  avtrans_rept_form_buttons($form, $form_state, $row, $view_mode);

  $form['#hide_discount_fields'] = TRUE;
  $form['discount_type']['#access'] = FALSE;
  $form['discount_value']['#access'] = FALSE;
}

/**
 * Set crud form settings.
 * @param $form
 * @param $form_state
 * @param $row
 */
function avtrans_rept_form_settings(&$form, &$form_state, $row, $view_mode = FALSE) {
  $js_settings = array('avtrans' => array('transaction' => 'purchase'));
  $form['#attached']['js'][] = array(
    'data' => $js_settings,
    'type' => 'setting',
  );

  // Get form storage.
  $form_state['storage'] = isset($form_state['storage']) ? $form_state['storage'] : array();
  $_storage = &$form_state['storage'];
  $_storage['transaction'] = $js_settings['avtrans']['transaction'];
  $_storage['transaction_type'] = 'rept';

  // Set generic submit handler.
  //$form['#submit'] = array('avbase_crud_form_submit');

  $table_name = 'avtbl_rept';
  $form['table_name'] = array('#type' => 'value', '#value' => $table_name);
  $form['entity_name'] = array('#type' => 'value', '#value' => 'rept');
  $form['#success_message'] = t('Purchase return details saved.');

  // Set autocomplete off.
  $form['#attributes']['autocomplete'] = 'off';

  // Set field names to save.
  $save_field_ids = array(
    'created',
    'client_id',
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
 * Generic DR form fields.
 * @param $form
 * @param $form_state
 * @param $row
 */
function avtrans_rept_form_fields(&$form, &$form_state, $row, $view_mode = FALSE) {
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
}

/**
 * Buttons for Goods Received form.
 * @param $form
 * @param $form_state
 * @param $row
 */
function avtrans_rept_form_buttons(&$form, $form_state, $row, $view_mode = FALSE) {
  $form['buttons']['#theme'] = 'avbase_crud_button_group';
  $form['buttons']['submit'] = array(
    '#id' => 'rept-submit-btn',
    '#type' => 'submit',
    '#value' => t('Save return details'),
    '#attributes' => array(
      'class' => array('av-ajax-trigger', 'uk-button-primary'),
    ),
    '#ajax' => array(
      'callback' => 'avtrans_transaction_form_ajax',
      //'wrapper' => 'avtrans-gr-cost-changed-wrapper',
      'wrapper' => 'avtrans-transaction-form-wrapper',
      'effect' => 'none',
      'event' => 'click',
    ),
  );
  if (empty($view_mode)) {
    $form['buttons']['cancel'] = array(
      '#markup' => l('Cancel', 'av/transactions', array('attributes' => array('class' => array('uk-button')))),
    );
  }
  else {
    $form['buttons']['print'] = array(
      '#id' => 'av-print-btn',
      '#type' => 'button',
      '#value' => t('Print...'),
      '#visible_in_view_mode' => TRUE,
      '#attributes' => array('class' => array('uk-button-primary')),
    );
  }
}