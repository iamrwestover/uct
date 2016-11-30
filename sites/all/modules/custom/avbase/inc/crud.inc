<?php

/**
 * Generic submit handler to crud forms.
 * @param $form
 * @param $form_state
 *
 * @throws
 */
function avbase_crud_form_submit($form, &$form_state) {
  $transaction = db_transaction();
  try {
    $values = $form_state['values'];
    $record_id = isset($values['id']) ? $values['id'] : NULL;
    $save_field_ids = isset($values['save_field_ids']) ? $values['save_field_ids'] : array();

    // Get $form_state values and store them on $row array.
    $row = array();
    foreach ($save_field_ids as $field_id) {
      if (!isset($values[$field_id]) || trim($values[$field_id]) == '') {
        continue;
      }
      $row[$field_id] = $values[$field_id];
    }

    // Get values for "data" field.
    $data_ids = isset($values['data_ids']) ? $values['data_ids'] : array();
    if (!empty($data_ids)) {
      $row['data'] = array();
      foreach ($data_ids as $data_id) {
        $row['data'][$data_id] = isset($values[$data_id]) ? $values[$data_id] : NULL;
      }
      $row['data'] = serialize($row['data']);
    }

    // Do save.
    $table_name = isset($values['table_name']) ? $values['table_name'] : NULL;
    if (!empty($row) && !empty($table_name)) {
      if (empty($record_id) || !is_numeric($record_id)) {
        // Insert new record.
        // Save timestamp when record is created.
        $row['created'] = time();
        if (db_insert($table_name)->fields($row)->execute()) {
          drupal_set_message('New record saved.');
        }
      } else {
        // Update record.
        if (db_update($table_name)->fields($row)->condition('id', $record_id)->execute()) {
          drupal_set_message('Changes saved.');
        }
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
