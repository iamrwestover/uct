<?php

/**
 * Perform actions depending on the kind of change that will be made
 * on the database for POs.
 * @param $op
 * @param $row
 * @param $form_state
 * @return object|NULL
 */
function avtrans_avbase_crud_dbchange_po($op, $row, $form_state) {
  switch ($op) {
    case 'pre-save':
      if (empty($row['id'])) {
        $row['transaction_date'] = time();
      }
      return $row;
      break;

    case 'after-save':
      // Get item rows.
      $item_rows = isset($form_state['storage']['item_rows']) ? $form_state['storage']['item_rows'] : array();
      $table_name = 'avtbl_po_items';
      $table_field_names = array('po_id', 'product_id', 'uom_id', 'qty_per_uom', 'qty', 'cost', 'discount');
      sort($table_field_names);

      $insert_rows = array();
      $insert_args = array();
      $update_rows = array();
      $update_args = array();
      $item_ids = array();
      foreach ($item_rows as $prod_row) {
        if (empty($prod_row['product_id'])) {
          continue;
        }
        $prod_row['po_id'] = $row['id'];

        $item_id = empty($prod_row['id']) ? 0 : $prod_row['id'];
        if (!empty($item_id)) {
          $item_ids[] = $item_id;
        }

        ksort($prod_row);
        $insert_arr = $update_arr = array();
        foreach ($prod_row as $k => $v) {
          if (!in_array($k, $table_field_names)) {
            continue;
          }
          $arg_id = ':arg_' . count($insert_args);
          if (empty($item_id)) {
            $insert_arr[] = $arg_id;
            $insert_args[$arg_id] = $v;
          }
          else {
            $update_rows[$k][$item_id] = $v;
          }
        }

        if (!empty($insert_arr)) {
          $insert_rows[] = "(" . implode(',', $insert_arr) . ")";
        }
      }
      //$query->execute();

      if (!empty($insert_rows)) {
        $query = "INSERT INTO {{$table_name}} (" . implode(', ', $table_field_names) . ") VALUES " . implode(', ', $insert_rows);
        db_query($query, $insert_args);
      }
      if (!empty($update_rows)) {
        $set = array();
        foreach ($update_rows as $field_name => $id_and_val) {
          $when = array();
          foreach ($id_and_val as $k => $v) {
            $arg_id = ':arg_' . count($update_args);
            $when[] = "WHEN $k THEN $arg_id";
            $update_args[$arg_id] = $v;
          }
          if (!empty($when)) {
            $set[] = "$field_name = CASE id " . implode(' ', $when) . ' END';
          }
        }

        if (!empty($set)) {
          $query = "UPDATE {{$table_name}} SET " . implode(', ', $set) . " WHERE id IN (:item_ids)";
          $update_args[':item_ids'] = $item_ids;
          db_query($query, $update_args);
        }
      }
      break;
  }
  return NULL;
}

/**
 * Extra submit handler for purchase order form.
 * @param $form
 * @param $form_state
 */
function avtrans_po_form_submit($form, &$form_state) {
  $triggering_element = isset($form_state['triggering_element']) ? $form_state['triggering_element'] : array();
  $po_id = empty($form_state['values']['id']) ? NULL : $form_state['values']['id'];
  switch ($triggering_element['#id']) {
    case 'submit_and_send':
      //$mail_to = empty($form_state['values']['email']) ? '' : $form_state['values']['email'];
      avtrans_po_send_to_email($po_id);
      break;
  }
  //$form_state['redirect'] = 'av/po/' . $po_id . '/view';
}
