<?php

/**
 * Load purchase order details.
 * @param $id
 */
function avexpenses_po_load($id) {
  $data = &drupal_static(__FUNCTION__, array());
  if (isset($data[$id])) {
    return $data[$id];
  }

  $rs = db_select('avtbl_po', 'avpo')
    ->fields('avpo')
    ->condition('avpo.id', $id)
    ->execute();
  $row = $rs->fetchObject();

  $rs = db_select('avtbl_po_items', 'avpo_items')
    ->fields('avpo_items')
    ->condition('avpo_items.po_id', $row->id)
    ->execute();
  foreach ($rs as $item) {
    $row->items[$item->id] = $item;
  }

  $data[$id] = $row;
  return $row;
}

/**
 * Loads purchase order details and sends it to specified email.
 * If email is not specified, po email will be used.
 * @param $po_id
 * @param string $email
 *
 * @return bool
 */
function avexpenses_po_send_to_email($po_id, $email = '') {
  if (empty($po_id)) {
    return FALSE;
  }

  $po = avexpenses_po_load($po_id);
  $email = $po->email;
  if ($error = user_validate_mail($email)) {
    drupal_set_message($error, 'error');
    return FALSE;
  }

  $params = array(
    'site_name' => variable_get('site_name', 'UCT'),
    'vendor_name' => 'Mark',
    'message_to_vendor' => check_plain($po->message),
  );
  if (drupal_mail('avexpenses', 'po', $email, language_default(), $params)) {
    drupal_set_message(t('Purchase order sent successfully to %mail.', array('%mail' => $email)));
    return TRUE;
  }
  return FALSE;
}
