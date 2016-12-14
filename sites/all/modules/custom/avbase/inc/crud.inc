<?php

/**
 * Generic submit handler to crud forms.
 * @param $form
 * @param $form_state
 *
 * @throws
 */
function avbase_crud_form_submit($form, &$form_state) {
  //dpm($form_state);return;
  $transaction = db_transaction();
  try {
    $values = $form_state['values'];
    $record_id = isset($values['id']) ? $values['id'] : NULL;
    $save_field_ids = isset($values['save_field_ids']) ? $values['save_field_ids'] : array();
    $hide_crud_messages = !empty($form['#hide_crud_messages']);

    // Get $form_state values and store them on $row array.
    $row = array();
    foreach ($save_field_ids as $k => $field) {
      $field_type = 'string';
      if (is_array($field)) {
        $field_id = $k;
        $field_type = isset($field['type']) ? $field['type'] : $field_type;
      }
      else {
        $field_id = $field;
      }

      if (!isset($values[$field_id])) {
        continue;
      }
      settype($values[$field_id], $field_type);
      $row[$field_id] = $field_type == 'string' ? trim($values[$field_id]) : $values[$field_id];
    }

    // Get values for "data" field.
    $data_ids = isset($values['data_ids']) ? $values['data_ids'] : array();
    if (!empty($data_ids)) {
      $row['data'] = array();
      foreach ($data_ids as $k => $data) {
        $data_type = 'string';
        if (is_array($data)) {
          $data_id = $k;
          $data_type = isset($data['type']) ? $data['type'] : $data_type;
        }
        else {
          $data_id = $data;
        }
        if (!isset($values[$data_id])) {
          continue;
        }
        settype($values[$data_id], $data_type);
        $row['data'][$data_id] = $data_type == 'string' ? trim($values[$data_id]) : $values[$data_id];
      }
      $row['data'] = serialize($row['data']);
    }

    // Do save.
    $table_name = isset($values['table_name']) ? $values['table_name'] : NULL;
    $entity_name = isset($values['entity_name']) ? $values['entity_name'] : '';
    if (!empty($row) && !empty($table_name)) {
      // Let other modules perform pre-save processes.
      if (!empty($entity_name)) {
        if ($returned_row = module_invoke_all('avbase_crud_dbchange_' . $entity_name, 'pre-save', $row, $form_state)) {
          $row = $returned_row;
        }
      }

      // Do save.
      if (empty($record_id) || !is_numeric($record_id)) {
        // Insert new record.
        // Save timestamp when record is created.
        if (array_search('created', $save_field_ids) !== FALSE) {
          $row['created'] = time();
        }
        if ($row['id'] = db_insert($table_name)->fields($row)->execute()) {
          if (!$hide_crud_messages) {
            drupal_set_message('New record saved.');
          }
        }
      } else {
        // Update record.
        if (db_update($table_name)->fields($row)->condition('id', $record_id)->execute()) {
          if (!$hide_crud_messages) {
            drupal_set_message('Changes saved.');
          }
        }
        $row['id'] = $record_id;
      }

      // Let other modules perform after-save processes.
      if (!empty($entity_name)) {
        module_invoke_all('avbase_crud_dbchange_' . $entity_name, 'after-save', $row, $form_state);
      }
    }
    else {
      drupal_set_message('Nothing saved.', 'warning');
    }
  }
  catch (Exception $e) {
    $transaction->rollback();
    watchdog_exception('crud_save', $e);
    throw $e;
  }
}
