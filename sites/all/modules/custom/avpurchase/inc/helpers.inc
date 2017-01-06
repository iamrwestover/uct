<?php

/**
 * Load purchase order details.
 * @param $id
 */
function avpurchase_po_load($id) {
  $id = (int) $id;
  if (empty($id)) {
    return NULL;
  }

  $data = &drupal_static(__FUNCTION__, array());
  if (isset($data[$id])) {
    return $data[$id];
  }

  $query = db_select('avtbl_po', 'avpo');
  $query->leftJoin('avtbl_vendors', 'avvendors', 'avvendors.id = avpo.vendor_id');
  $query->leftJoin('avtbl_categories', 'avcat', "avcat.id = avpo.term_id AND avcat.group_id = 'payment_term'");
  $query->addField('avvendors', 'display_name', 'vendor_name');
  $query->addField('avcat', 'title', 'payment_term_title');
  $query->fields('avpo')
    //->fields('avvendors', array('display_name'))
    ->condition('avpo.id', $id)
    ->execute();
  $row = $query->execute()->fetchObject();

  $subtotal = 0;
  if (!empty($row->id)) {
    $query = db_select('avtbl_po_items', 'avpo_items');
    $query->leftJoin('avtbl_products', 'avprod', 'avprod.id = avpo_items.product_id');
    $query->leftJoin('avtbl_categories', 'avcat', "avcat.id = avpo_items.uom_id AND avcat.group_id = 'uom'");
    $query->addField('avprod', 'title', 'product_title');
    $query->addField('avcat', 'title', 'uom_title');
    $query->fields('avpo_items')
      ->condition('avpo_items.po_id', $row->id)
      ->execute();
    $rs = $query->execute();
    foreach ($rs as $item) {
      $item->total = $item->cost * $item->qty;
      $row->items[$item->id] = $item;
      $subtotal += $item->total;
    }
  }

  $row->discount_value_text = $row->discount_type == AVBASE_DISCOUNT_PERCENT ? (round($row->discount_value, 2) . '%') : number_format($row->discount_value, 2);
  $row->subtotal = $subtotal;
  $row->discounted_value = $row->discount_type == AVBASE_DISCOUNT_PERCENT ? (($row->discount_value / 100) * $subtotal) : $row->discount_value;
  $row->grand_total = $subtotal - $row->discounted_value;

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
function avpurchase_po_send_to_email($po_id, $email = '') {
  if (empty($po_id)) {
    return FALSE;
  }

  $po = avpurchase_po_load($po_id);
  $email = $po->email;
  if ($error = user_validate_mail($email)) {
    drupal_set_message($error, 'error');
    return FALSE;
  }

  $po->discount_value = round($po->discount_value, 2);
  $params = array(
    'po' => $po,
    'po_num' => $po->id,
    'po_date' => format_date($po->created, 'custom', 'F d, Y'),
    'site_name' => variable_get('site_name', 'UCT'),
    'shipping_address' => variable_get('av_company_address', '-'),
    'vendor_name' => check_plain($po->vendor_name),
    'payment_term_title' => check_plain($po->payment_term_title),
  );

  $params['po_table'] = theme('avpurchase_po_mail_table', $params);
  $message = drupal_mail('avpurchase', 'po', $email, language_default(), $params);
  if (!empty($message['result'])) {
    drupal_set_message(t('Purchase order sent successfully to %mail.', array('%mail' => $email)));
    return TRUE;
  }
  return FALSE;
}
