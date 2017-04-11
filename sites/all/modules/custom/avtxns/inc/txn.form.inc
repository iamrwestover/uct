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

  if (empty($view_mode) && !empty($row->id) && !in_array($row->status, array(AVTXNS_TXN_STATUS_PENDING, AVTXNS_TXN_STATUS_OPEN))) {
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
  if (isset($ttd['pending_txns_reference_type']) && empty($row->id)) {
    // Add fields for Pending orders popup.
    avtxns_pending_orders_form_fields($form, $form_state, $transaction_type, $row, $view_mode);
  }

  //if ($view_mode && $transaction_type == AVTXNS_TXN_TYPE_INVOICE && !empty($row->client_id)) {
  //  $term_days = isset($row->term_data['days']) ? (int) $row->term_data['days'] : NULL;
  //  $form['special_discount'] = avtxns_special_discount_form($term_days);
  //  $form['balance_ages'] = avreports_client_balance_aging_form($row->client_id);
  //}


  // Buttons.
  $form['buttons'] = avtxns_transaction_buttons($form_state, $transaction_type, $row, $view_mode);

  // Show created info.
  if ($view_mode && !empty($row->created)) {
    global $user;
    $form['record_created'] = array('#markup' => '<div class="uk-text-center">' . t('Created by @name - @date', array('@name' => $user->name, '@date' => format_date($row->created, 'custom', 'l, F d, Y h:i A'))) . '</div>');
  }
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
  //      INNER JOIN {avtbl_clients} avclients ON avclients.id = avtxn.client_id
  //      WHERE avclients.display_name = :client_name AND avtxn.status = 1 AND avtxn.transaction_type = :ttype";
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

  if ($view_mode && !empty($row->id)) {
    $window_title = strtoupper($row->transaction_type) . '# ' . $row->id;
    if (isset($row->client_name)) {
      $window_title .= ' .::. ' . strtoupper($row->client_name);
    }
    if (isset($row->grand_total)) {
      $window_title .= ' - ' . number_format($row->grand_total, 2);
    }
    avbase_set_window_title($window_title);
  }

  // Transaction status.
  if (isset($row->status) && $row->status != AVTXNS_TXN_STATUS_OPEN) {
    $tsd = avtxns_txn_status_details($row->status);
    $badge_color = 'uk-badge-success';
    $icon_html = '';
    $form_color_class = '';
    switch ($row->status) {
      case AVTXNS_TXN_STATUS_PENDING:
        $badge_color = 'uk-badge-warning';
        break;
      case AVTXNS_TXN_STATUS_VOID:
        $badge_color = 'uk-badge-void';
        $form_color_class = 'uk-alert uk-alert-mute';
        break;
    }
    $form['#prefix'] .= '<span class="uk-text-large not-printable uk-badge ' . $badge_color . '" style="position:absolute; top:0;">&nbsp;' . $icon_html . strtoupper($tsd['title']) . '&nbsp;</span><span class="not-printable">&nbsp;</span>';
    if ($row->status != AVTXNS_TXN_STATUS_CLOSED) {
      $form['#attributes']['class'][] = 'not-printable';
    }
    $form['#attributes']['class'][] = $form_color_class;
    //$page_title .= ' - ' . strtoupper($tsd['title']);
  }

  // Settings.
  $form['table_name'] = array('#type' => 'value', '#value' => 'avtbl_transactions');
  $form['entity_name'] = array('#type' => 'value', '#value' => 'txn');
  $form['transaction_type'] = array('#type' => 'value', '#value' => $transaction_type);
  $form['id'] = array('#type' => 'value', '#value' => (isset($row->id) ? $row->id : NULL));
  $form['last_changed'] = array('#type' => 'hidden', '#default_value' => (isset($row->changed) ? $row->changed : NULL));
  $form['transaction_status'] = array('#type' => 'value', '#value' => (isset($row->status) ? $row->status : NULL));
  $form['#success_message'] = t('@type details saved.', array('@type' => $ttd['name']));
  $form['#theme'] = 'avtxns_item_list_form';

  // Set generic submit handler.
  if (!isset($ttd['pending_txns_reference_type'])) {
    $form['#submit'] = array('avbase_crud_form_submit', 'avtxns_txn_form_submit', 'avtxns_redirect_transaction_form');
  }
  else {
    $form['#submit'] = array('avtxns_txn_form_submit2');
  }
  // Generic form validator.
  $form['#validate'][] = 'avbase_crud_form_validate';

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
    'pmt_method_id' => array('type' => 'int'),
    'memo',
    'email',
    'message',
    'created',
    'changed',
    'uid',
    'amount_to_credit' => array('type' => 'float'),
    //'available_credit' => array('type' => 'float'),
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
  if (empty($view_mode)) {
    $buttons['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Save'),
      '#icon_key' => 'save',
      //'#attributes' => array(
      //  'data-uk-tooltip' => "{delay:'500', cls: 'tt-keyboard-shortcut'}",
      //  'title' => "Ctrl Alt S",
      //),
    );
    if ($transaction_type == AVTXNS_TXN_TYPE_PURCHASE_ORDER) {
      $buttons['submit_and_send'] = array(
        '#id' => 'submit_and_send',
        '#type' => 'submit',
        '#value' => t('Save and send'),
        '#icon_key' => 'send',
      );
    }

    $buttons['submit_and_print'] = array(
      '#id' => 'submit_and_print',
      '#type' => 'submit',
      '#value' => t('Save and print'),
      '#icon_key' => 'print',
    );

    $buttons['cancel'] = array(
      '#markup' => l('Cancel', 'av/transactions', array('attributes' => array('class' => array('uk-button')))),
    );
  }
  else {
    if (in_array($row->status, array(AVTXNS_TXN_STATUS_CLOSED, AVTXNS_TXN_STATUS_OPEN))) {
      $buttons['print'] = array(
        '#id' => 'av-print-btn',
        '#type' => 'button',
        '#value' => t('Print...'),
        '#visible_in_view_mode' => TRUE,
        '#attributes' => array('class' => array('suk-button-primary')),
        '#icon_key' => 'print',
      );
    }

    // Add status change buttons.
    if (empty($ttd['cant_change_status'])) {
      if (isset($row->status) && in_array($row->status, array(AVTXNS_TXN_STATUS_OPEN, AVTXNS_TXN_STATUS_PENDING)) && avtxns_txn_can_status_change($row, AVTXNS_TXN_STATUS_VOID)) {
        $buttons['void_btn'] = array(
          '#markup' => l('Void transaction', 'av/transactions/' . $row->id . '/change-status/' . AVTXNS_TXN_STATUS_VOID, array(
            'attributes' => array('class' => 'uk-button uk-button-danger uk-margin-small-right'),
            'query' => array('destination' => $ttd['base_path'] . '/' . $row->id . '/view'),
            'icon_key' => 'minus-circle',
            'icon_classes' => array('uk-margin-small-right'),
          )),
          '#visible_in_view_mode' => TRUE,
          '#avposition' => 'opposite',
          //'#access' => isset($row->status) && in_array($row->status, array(AVTXNS_TXN_STATUS_OPEN, AVTXNS_TXN_STATUS_PENDING)) && avtxns_txn_can_status_change($row, AVTXNS_TXN_STATUS_VOID),
        );
      }
      if (isset($row->status) && in_array($row->status, array(AVTXNS_TXN_STATUS_OPEN)) && $row->transaction_type == AVTXNS_TXN_TYPE_DELIVERY && avtxns_txn_can_status_change($row, AVTXNS_TXN_STATUS_OPEN)) {
        $buttons['pending_btn'] = array(
          '#markup' => l('For approval', 'av/transactions/' . $row->id . '/change-status/' . AVTXNS_TXN_STATUS_PENDING, array(
            'attributes' => array('class' => 'uk-button uk-margin-small-right'),
            'query' => array('destination' => $ttd['base_path'] . '/' . $row->id . '/view'),
            'icon_key' => 'user',
            'icon_classes' => array('uk-margin-small-right'),
          )),
          '#visible_in_view_mode' => TRUE,
          '#avposition' => 'opposite',
          //'#access' => isset($row->status) && in_array($row->status, array(AVTXNS_TXN_STATUS_OPEN)) && $row->transaction_type == AVTXNS_TXN_TYPE_DELIVERY && avtxns_txn_can_status_change($row, AVTXNS_TXN_STATUS_OPEN),
        );
      }
      if (isset($row->status) && in_array($row->status, array(AVTXNS_TXN_STATUS_PENDING)) && $row->transaction_type == AVTXNS_TXN_TYPE_DELIVERY && avtxns_txn_can_status_change($row, AVTXNS_TXN_STATUS_OPEN)) {
        $buttons['open_btn'] = array(
          '#markup' => l('Approve', 'av/transactions/' . $row->id . '/change-status/' . AVTXNS_TXN_STATUS_OPEN, array(
            'attributes' => array('class' => 'uk-button uk-button-success uk-margin-small-right'),
            'query' => array('destination' => $ttd['base_path'] . '/' . $row->id . '/view'),
            'icon_key' => 'thumbs-o-up',
            'icon_classes' => array('uk-margin-small-right'),
          )),
          '#visible_in_view_mode' => TRUE,
          '#avposition' => 'opposite',
          //'#access' => isset($row->status) && in_array($row->status, array(AVTXNS_TXN_STATUS_PENDING)) && $row->transaction_type == AVTXNS_TXN_TYPE_DELIVERY && avtxns_txn_can_status_change($row, AVTXNS_TXN_STATUS_OPEN),
        );
      }
    }
  }

  if (isset($buttons['submit']) && isset($ttd['pending_txns_reference_type'])) {
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

  //if (!empty($ttd['bill'])) {
  //  // Add extra button to load customer returns.
  //  $buttons['check_returns'] = array(
  //    '#id' => 'check-ret-btn',
  //    '#type' => 'submit',
  //    '#value' => t('Add credits'),
  //    '#attributes' => array(
  //      'class' => array('av-ajax-trigger'),
  //    ),
  //    '#submit' => array('avtxns_pending_orders_form_submit'),
  //    '#limit_validation_errors' => array(array('client_name'), array('client_id'), array('transaction_type')),
  //    '#ajax' => array(
  //      'callback' => 'avtxns_txn_form_ajax',
  //      'wrapper' => 'avtxns-txn-form-wrapper',
  //      'effect' => 'none',
  //      'event' => 'click',
  //    ),
  //    '#reference_type' => $ttd['pending_return_type'],
  //    '#avposition' => 'opposite',
  //    //'#disabled' => TRUE,
  //  );
  //
  //  // Count pending returns.
  //  $client_name = empty($row->client_name) ? NULL : $row->client_name;
  //  $client_name = empty($client_name) ? (empty($form_state['values']['client_name']) ? NULL : $form_state['values']['client_name']) : $client_name;
  //  if (empty($view_mode) && !empty($client_name)) {
  //    $query = "SELECT COUNT(avtxn.id) FROM {avtbl_transactions} avtxn
  //      INNER JOIN {avtbl_clients} avclients ON avclients.id = avtxn.client_id
  //      WHERE avclients.display_name = :client_name AND avtxn.status = 1 AND avtxn.transaction_type = :ttype";
  //    $count = db_query($query, array(':client_name' => $client_name, ':ttype' => $ttd['pending_return_type']))->fetchField();
  //    if (!empty($count)) {
  //      //$buttons['check_returns']['#attributes']['class']['color'] = 'uk-button-success';
  //      $buttons['check_returns']['#button_label'] = "Add returns - <em>$count pending</em>";
  //      //$buttons['check_returns']['#disabled'] = FALSE;
  //    }
  //  }
  //}
  return $buttons;
}

/**
 * Menu callback: confirm transaction status change.
 * @param $form
 * @param $form_state
 * @param object $row
 * @param $status
 *
 * @return array
 */
function avtxns_txn_change_status_confirm_form($form, &$form_state, $row, $status) {
  switch ($status) {
    case AVTXNS_TXN_STATUS_PENDING:
      $prompt = 'Are you sure you want to send this transaction for admin approval?';
      break;
    case AVTXNS_TXN_STATUS_OPEN:
      $prompt = 'Are you sure you want to open this transaction?';
      break;
    case AVTXNS_TXN_STATUS_VOID:
      $prompt = 'Are you sure you want to void this transaction?';
      break;
    default:
      $prompt = 'Are you sure?';
      break;
  }

  $display_name = 'Transaction # ' . $row->id;
  $client_name = check_plain($row->client_name);
  $ttd = avtxns_txn_types($row->transaction_type);
  $description = <<<HTML
  <span class="uk-text-bold">$display_name</span><br />
  {$ttd['name']}<br />
  $client_name
HTML;

  if ($status == AVTXNS_TXN_STATUS_VOID) {
    $description .= '<br /><br /><em class="uk-text-large uk-margin-top">This action cannot be undone.</em>';
  }
  $form_state['id'] = $row->id;
  $form_state['status'] = $status;
  return confirm_form(array(), $prompt, '', $description);
}

/**
 * Handler for transaction status change.
 *
 * @see avtxns_txn_change_status_confirm_form()
 */
function avtxns_txn_change_status_confirm_form_submit($form, &$form_state) {
  $status = isset($form_state['status']) ? $form_state['status'] : '';
  $id = empty($form_state['id']) ? NULL : $form_state['id'];
  if (avtxns_txn_change_status($id, $status)) {
    drupal_set_message('Transaction status changed.');
  }

  //
  //if (isset($form_state['row']->id)) {
  //
  //  if (avclients_client_change_status($form_state['row']->id, $status)) {
  //
  //    switch ($status) {
  //      case AVCLIENT_STATUS_DISABLED:
  //        //drupal_set_message('Client\'s account has been disabled.');
  //        break;
  //      case AVCLIENT_STATUS_SUSPENDED:
  //        //drupal_set_message('Client\'s account has been suspended.');
  //        break;
  //      case AVCLIENT_STATUS_ACTIVE:
  //        drupal_set_message('Client\'s account has been activated.');
  //        break;
  //      default:
  //        break;
  //    }
  //  }
  //}
  //$form_state['redirect'] = '';
}
