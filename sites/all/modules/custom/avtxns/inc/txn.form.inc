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
  if (!empty($row->id) && $row->transaction_type != $transaction_type) {
    drupal_not_found();
    drupal_exit();
  }

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
  $form['buttons'] = avtxns_transaction_buttons($form_state, $transaction_type, $row, $view_mode);

  //if ($transaction_type == AVTXNS_TXN_TYPE_INVOICE) {
  //  // Add extra button to load customer returns.
  //  $form['buttons']['check_returns'] = $form['pending_orders']['buttons']['check_po'];
  //  $form['buttons']['check_returns']['#avposition'] = 'opposite';
  //  $form['buttons']['check_returns']['#value'] = t('Add returns');
  //  //$form['buttons']['check_returns']['#weight'] = -1;
  //  //$form['buttons']['check_returns']['#icon_key'] = 'money';
  //  $form['buttons']['check_returns']['#id'] = 'check-ret-btn';
  //  $form['buttons']['check_returns']['#reference_type'] = AVTXNS_TXN_TYPE_SALES_RETURN;
  //  $form['buttons']['check_returns']['#access'] = TRUE;
  //  $form['buttons']['check_returns']['#disabled'] = TRUE;
  //
  //  //$form['buttons']['check_returns']['#attributes']['class']['margin'] = 'uk-margin-large-right';
  //  unset($form['buttons']['check_returns']['#attributes']['class']['color']);
  //  unset($form['buttons']['check_returns']['#attributes']['class']['hide']);
  //  //unset($form['buttons']['check_returns']['#ajax']);
  //
  //  // Count pending returns.
  //  $client_name = empty($row->client_name) ? NULL : $row->client_name;
  //  $client_name = empty($client_name) ? (empty($form_state['values']['client_name']) ? NULL : $form_state['values']['client_name']) : $client_name;
  //  if (!empty($client_name)) {
  //    $query = "SELECT COUNT(avtxn.id) FROM {avtbl_transactions} avtxn
  //      INNER JOIN {avtbl_clients} avclient ON avclient.id = avtxn.client_id
  //      WHERE avclient.display_name = :client_name AND avtxn.status = 1 AND avtxn.transaction_type = :ttype";
  //    $count = db_query($query, array(':client_name' => $client_name, ':ttype' => AVTXNS_TXN_TYPE_SALES_RETURN))->fetchField();
  //    if (!empty($count)) {
  //      //$form['buttons']['check_returns']['#attributes']['class']['color'] = 'uk-button-success';
  //      $form['buttons']['check_returns']['#value'] .= " - <em>$count pending</em>";
  //      $form['buttons']['check_returns']['#disabled'] = FALSE;
  //    }
  //  }
  //}

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
    'due_date' => array('type' => 'int'),
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
 * @param array $form_state
 * @param $transaction_type
 * @param $row
 * @param $view_mode
 *
 * @return array
 */
function avtxns_transaction_buttons($form_state, $transaction_type, $row, $view_mode) {
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

  if ($transaction_type == AVTXNS_TXN_TYPE_INVOICE) {
    // Add extra button to load customer returns.
    $buttons['check_returns'] = array(
      '#id' => 'check-ret-btn',
      '#type' => 'submit',
      '#value' => t('Add returns'),
      '#attributes' => array(
        'class' => array('av-ajax-trigger'),
      ),
      '#submit' => array('avtxns_pending_orders_form_submit'),
      '#limit_validation_errors' => array(array('client_name'), array('client_id'), array('transaction_type')),
      '#ajax' => array(
        'callback' => 'avtxns_txn_form_ajax',
        'wrapper' => 'avtxns-txn-form-wrapper',
        'effect' => 'none',
        'event' => 'click',
      ),
      '#reference_type' => AVTXNS_TXN_TYPE_SALES_RETURN,
      '#avposition' => 'opposite',
      //'#disabled' => TRUE,
    );

    // Count pending returns.
    $client_name = empty($row->client_name) ? NULL : $row->client_name;
    $client_name = empty($client_name) ? (empty($form_state['values']['client_name']) ? NULL : $form_state['values']['client_name']) : $client_name;
    if (!empty($client_name)) {
      $query = "SELECT COUNT(avtxn.id) FROM {avtbl_transactions} avtxn
        INNER JOIN {avtbl_clients} avclient ON avclient.id = avtxn.client_id
        WHERE avclient.display_name = :client_name AND avtxn.status = 1 AND avtxn.transaction_type = :ttype";
      $count = db_query($query, array(':client_name' => $client_name, ':ttype' => AVTXNS_TXN_TYPE_SALES_RETURN))->fetchField();
      if (!empty($count)) {
        //$buttons['check_returns']['#attributes']['class']['color'] = 'uk-button-success';
        $buttons['check_returns']['#button_label'] = "Add returns - <em>$count pending</em>";
        //$buttons['check_returns']['#disabled'] = FALSE;
      }
    }
  }
  return $buttons;
}
