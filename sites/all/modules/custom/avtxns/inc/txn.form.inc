<?php
module_load_include('inc', 'avtxns', 'inc/txn.handlers');
module_load_include('inc', 'avtxns', 'inc/item_list.form');
module_load_include('inc', 'avtxns', 'inc/txn_cost_changes.form');
module_load_include('inc', 'avtxns', 'inc/txn_pending_orders.form');

/**
 * PO form.
 *
 * @param array $form
 * @param array $form_state
 * @param string $transaction_type
 * @param object $row
 * @param bool $view_mode
 * @return array
 */
function avtxns_txn_form($form, &$form_state, $transaction_type, $row, $view_mode = FALSE) {
  if (empty($row->status) && empty($view_mode) && !empty($row->id)) {
    drupal_set_message('This transaction cannot be modified anymore.', 'warning');
    $view_mode = TRUE;
  }

  // Get transaction type details.
  $ttd = avtxns_txn_types($transaction_type);

  // Wrapper.
  $form['#prefix'] = '<div id="avtxns-txn-form-wrapper"><!-- Do not remove me. -->';
  $form['#suffix'] = '</div>';

  // Set crud settings.
  avtxns_txn_form_settings($form, $form_state, $transaction_type, $row, $view_mode);

  // Item List form fields.
  avtxns_item_list_form_fields($form, $form_state, $transaction_type, $row, $view_mode);

  if (!empty($ttd['cost_change'])) {
    // Add fields for Cost Changes popup.
    avtxns_cost_changes_form_fields($form, $form_state, $transaction_type, $row, $view_mode);
  }
  if (isset($ttd['pending_txns_reference_type'])) {
    // Add fields for Pending orders popup.
    avtxns_pending_orders_form_fields($form, $form_state, $transaction_type, $row, $view_mode);
  }

  // Buttons.
  $form['buttons'] = avtxns_transaction_buttons($transaction_type, $view_mode);

  // Convert form to non-edit mode if we are on View page.
  avbase_allow_form_view_mode($form, $view_mode);
  return $form;
}

/**
 * Set crud form settings.
 * @param array $form
 * @param array $form_state
 * @param string $transaction_type
 * @param object $row
 * @param bool $view_mode
 */
function avtxns_txn_form_settings(&$form, &$form_state, $transaction_type, $row, $view_mode = FALSE) {
  // Get transaction type details.
  $ttd = avtxns_txn_types($transaction_type);

  // Set page title.
  $page_title = empty($row->id) ? t('New @type', array('@type' => $ttd['name'])) : t('@type', array('@type' => $ttd['name']));
  drupal_set_title($page_title);

  // Settings.
  $form['table_name'] = array('#type' => 'value', '#value' => 'avtbl_transactions');
  $form['entity_name'] = array('#type' => 'value', '#value' => 'txn');
  $form['transaction_type'] = array('#type' => 'value', '#value' => $transaction_type);
  $form['id'] = array('#type' => 'value', '#value' => isset($row->id) ? $row->id : NULL);
  $form['#success_message'] = t('@type details saved.', array('@type' => $ttd['name']));
  $form['#theme'] = 'avtxns_item_list_form';

  // Set generic submit handler.
  if (!isset($ttd['pending_txns_reference_type'])) {
    $form['#submit'] = array('avbase_crud_form_submit', 'avtxns_txn_form_submit', 'avtxns_redirect_transaction_form');
  }
  else {
    $form['#submit'] = array('avtxns_txn_form_submit2');
  }

  // Js settings.
  $js_settings = array('avtxns' => array('transaction' => $ttd['transaction']));
  $form['#attached']['js'][] = array(
    'data' => $js_settings,
    'type' => 'setting',
  );

  // Get form storage.
  $form_state['storage'] = isset($form_state['storage']) ? $form_state['storage'] : array();
  $_storage = &$form_state['storage'];
  //$_storage['transaction'] = $js_settings['avtxns']['transaction'];


  
  // Set autocomplete off.
  $form['#attributes']['autocomplete'] = 'off';

  // Set field names to save.
  $save_field_ids = array(
    'transaction_type',
    'transaction_date',
    'client_id',
    'term_id' => array('type' => 'int'),
    'agent_id' => array('type' => 'int'),
    'reference_id',
    'return_type' => array('type' => 'int'),
    'due_date',
    'memo',
    'email',
    'message',
    'created',
    'changed',
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
 * Transaction buttons.
 * @param $transaction_type
 * @param $view_mode
 *
 * @return array
 */
function avtxns_transaction_buttons($transaction_type, $view_mode) {
  // Get transaction type details.
  $ttd = avtxns_txn_types($transaction_type);

  $buttons = array('#theme' => 'avbase_crud_button_group');
  $buttons['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#icon_key' => 'save',
  );
  if ($transaction_type == AVTXNS_TXN_TYPE_PURCHASE_ORDER) {
    $buttons['submit_and_send'] = array(
      '#id' => 'submit_and_send',
      '#type' => 'submit',
      '#value' => t('Save and send'),
      '#icon_key' => 'mail-forward',
    );
  }

  $buttons['submit_and_print'] = array(
    '#id' => 'submit_and_print',
    '#type' => 'submit',
    '#value' => t('Save and print'),
    '#icon_key' => 'print',
  );
  if (empty($view_mode)) {
    $buttons['cancel'] = array(
      '#markup' => l('Cancel', 'av/txns', array('attributes' => array('class' => array('uk-button')))),
    );
  }
  else {
    $buttons['print'] = array(
      '#id' => 'av-print-btn',
      '#type' => 'button',
      '#value' => t('Print...'),
      '#visible_in_view_mode' => TRUE,
      '#attributes' => array('class' => array('suk-button-primary')),
      '#icon_key' => 'print',
    );
  }

  if (isset($ttd['pending_txns_reference_type'])) {
    $buttons['submit']['#id'] = 'detect-cost-change-btn';
    $buttons['submit']['#attributes']['class'][] = 'av-ajax-trigger';
    $buttons['submit']['#ajax'] = array(
      'callback' => 'avtxns_txn_form_ajax',
      'wrapper' => 'avtxns-txn-form-wrapper',
      'effect' => 'none',
      'event' => 'click',
    );
    $buttons['submit_and_print']['#attributes'] = $buttons['submit']['#attributes'];
    $buttons['submit_and_print']['#ajax'] = $buttons['submit']['#ajax'];
  }
  return $buttons;
}
