<?php

/**
 * Perform actions depending on the kind of change that will be made
 * on the database for Goods received.
 * @param $op
 * @param $row
 * @param $form_state
 * @return object|NULL
 */
function avtrans_avbase_crud_dbchange_spay($op, $row, $form_state) {
  return avtrans_avbase_crud_dbchange_gr($op, $row, $form_state);
}

///**
// * Ajax callback before submitting a Receive Items form.
// */
//function avtrans_delivery_form_ajax($form, $form_state) {
//  if (form_get_errors()) {
//    // Prevent modal from popping out.
//    unset($form['cost_changes']['product_cost_changes']);
//  }
//  return $form;
//}

/**
 * Submit handler for Goods Received form.
 * @param $form
 * @param $form_state
 */
function avtrans_spay_form_submit($form, &$form_state) {
  avtrans_gr_form_submit($form, $form_state);
}
