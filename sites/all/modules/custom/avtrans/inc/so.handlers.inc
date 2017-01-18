<?php

/**
 * Perform actions depending on the kind of change that will be made
 * on the database for Sales Orders.
 * @param $op
 * @param $row
 * @param $form_state
 * @return object|NULL
 */
function avtrans_avbase_crud_dbchange_so($op, $row, $form_state) {
  switch ($op) {
    case 'pre-save':
      $row['transaction_date'] = time();
      return $row;
      break;

    case 'after-save':
      // Get item rows.
      $item_rows = isset($form_state['storage']['item_rows']) ? $form_state['storage']['item_rows'] : array();

      // Save sales order items.
      $query = db_insert('avtbl_so_items')->fields(array('so_id', 'product_id', 'uom_id', 'qty_per_uom', 'qty', 'cost'));
      foreach ($item_rows as $prod_row) {
        if (empty($prod_row['product_id'])) {
          continue;
        }
        $prod_row['so_id'] = $row['id'];
        $query->values($prod_row);
      }
      $query->execute();
      break;
  }
  return NULL;
}

///**
// * Extra submit handler for sales order order form.
// * @param $form
// * @param $form_state
// */
//function avtrans_so_form_submit($form, &$form_state) {
//  $triggering_element = isset($form_state['triggering_element']) ? $form_state['triggering_element'] : array();
//  switch ($triggering_element['#id']) {
//    case 'submit_and_send':
//      $po_id = empty($form_state['values']['id']) ? NULL : $form_state['values']['id'];
//      //$mail_to = empty($form_state['values']['email']) ? '' : $form_state['values']['email'];
//      avtrans_so_send_to_email($po_id);
//      break;
//  }
//}
