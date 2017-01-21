<?php

/**
 * Client Changes form fields.
 * @param $form
 * @param $form_state
 * @param $row
 *
 * @return mixed
 */
function avtrans_client_changes_form_fields(&$form, &$form_state, $row) {
  // Get form storage.
  $form_state['storage'] = isset($form_state['storage']) ? $form_state['storage'] : array();
  $_storage = &$form_state['storage'];
  $transaction = empty($_storage['transaction']) ? '' : $_storage['transaction'];

  $detected_client_changes = empty($form_state['values']['detected_client_changes']) ? array() : $form_state['values']['detected_client_changes'];
  $fields = array();
  $fields['client_changes'] = array('#tree' => TRUE);
  $table_rows = array();
  foreach ($detected_client_changes as $k => $v) {
    $table_cols = array();
    $table_cols['field_title'] = array(
      '#title' => 'Field title',
      '#title_display' => 'none',
      '#type' => 'item',
      '#markup' => check_plain($v['field_title']),
      '#column_attributes' => array('class' => array('uk-width-1-5')),
    );

    $table_cols['current_value'] = array(
      '#title' => 'Current value',
      '#title_display' => 'none',
      '#type' => 'item',
      '#markup' => check_plain($v['current_value']),
      '#column_attributes' => array('class' => array('uk-width-1-5')),
    );

    $table_cols['new_value'] = array(
      '#title' => 'New value',
      '#title_display' => 'none',
      '#type' => 'item',
      '#markup' => check_plain($v['new_value_text']),
      '#column_attributes' => array('class' => array('uk-width-1-5')),
    );

    $table_cols['values'] = array(
      'action' => array(
        '#type' => 'radios',
        '#title' => t('Action'),
        '#title_display' => 'none',
        '#options' => array(
          'ignore' => t('Leave unchanged'),
          'update' => t('Update'),
        ),
        '#default_value' => 'ignore',
        '#attributes' => array('class' => array('uk-button-group')),
      ),
      'new_value' => array('#type' => 'value', '#value' => $v['new_value']),
      'field_id' => array('#type' => 'value', '#value' => $v['field_id']),
      '#column_attributes' => array('class' => array('uk-width-2-5')),
    );

    $table_rows[$k] = $table_cols;
    $table_rows[$k]['#row_attributes'] = array('class' => array('uk-table-middle'));
  }

  if (!empty($table_rows)) {
    $table = array(
      '#theme' => 'avbase_form_table',
      '#header' => array(
        'title' => array('data' => '&nbsp;'),
        'current_value' => array('data' => t('Current value')),
        'new_value' => array('data' => t('New value')),
        'actions' => '&nbsp;',
      ),
      //'product_rows' => array('#tree' => TRUE),
    );

    $table['rows'] = $table_rows;
    $table['#attributes'] = array('class' => array('uk-table-striped', 'uk-table-condensed'));

    //$fields['client_changes'] += $pcfields;
    //$fields['intro'] = array('#weight' => -5, '#markup' => '<p>Some product costs have been changed. Do you want to update?</p>');
    $fields['client_changes'] += $table;
  }

  // Buttons
  $fields['buttons']['submit'] = array(
    '#id' => 'client-changed-submit-btn',
    '#type' => 'submit',
    '#value' => t('Continue'),
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
  $fields['buttons']['cancel'] = array(
    '#id' => 'client-changed-cancel-btn',
    '#type' => 'submit',
    '#value' => t('Cancel'),
    '#attributes' => array(
      'class' => array('uk-modal-close'),
    ),
  );

  $client_changes_form = $fields;
  $client_changes_form['#type'] = 'container';
  $client_changes_form['#theme'] = 'avbase_modal_form';
  $client_title = $transaction == 'sales' ? 'Customer' : 'Vendor';
  $client_changes_form['#modal_options'] = array(
    'modal_id' => 'client-changed',
    'modal_title' => t('@client_title details have been changed. Do you want to update our records?', array('@client_title' => $client_title)),
    'modal_classes' => array('uk-modal-dialog-large'),
  );
  $form['client_changes'] = $client_changes_form;

  // Attachments.
  $form['#attached']['js'][] = AVTRANS_MODULE_PATH . '/js/trans.client_changes.form.js';
}

/**
 * Update products record with changes to cost and price.
 * @param $form_state
 * @param $row
 */
function avtrans_client_changes_db_update($form_state, $row) {
  // Get form storage.
  $form_state['storage'] = isset($form_state['storage']) ? $form_state['storage'] : array();
  $_storage = &$form_state['storage'];

  // Get transaction type.
  $transaction = empty($_storage['transaction']) ? '' : $_storage['transaction'];
  $transaction_type = empty($_storage['transaction_type']) ? 'gr' : $_storage['transaction_type'];

  // Get form values.
  $values = empty($form_state['values']) ? array() : $form_state['values'];
  $client_changes = empty($values['client_changes']) ? array() : $values['client_changes'];
  if (!empty($client_changes)) {
    foreach ($client_changes['rows'] as $client_change) {
      $changes = empty($client_change['values']) ? array() : $client_change['values'];
      if (empty($changes)) {
        continue;
      }

      $update_arr = array();
      $update_args = array();
      $field_id = !isset($changes['field_id']) ? '' : $changes['field_id'];
      $field_value = !isset($changes['field_value']) ? '' : $changes['field_value'];
      switch ($changes['action']) {
        case 'update':
          $update_arr[$field_id] = "$field_id = :$field_id";
          $update_args[":$field_id"] = $field_value;
          break;
        default:
          continue;
          break;
      }
    }

    $table_name = $transaction == 'sales' ? 'avtbl_customers' : 'avtbl_vendors';
    if (!empty($update_arr)) {
      $update_args[':client_id'] = $row['client_id'];
      $query = "UPDATE {{$table_name}} SET " . implode(', ', $update_arr) . " END WHERE id = (:client_id)";
      if (db_query($query, $update_args)) {
        avbase_custom_watchdog("{$transaction_type}__client_update", "ID: {$row['id']}", array('#row' => $row, '#client_changes' => $client_changes, '#update_arr' => $update_arr, '#update_args' => $update_args));
      }
    }
  }
}
