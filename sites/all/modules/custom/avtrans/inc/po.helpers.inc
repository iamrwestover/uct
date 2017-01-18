<?php

/**
 * Load purchase order details.
 * @param $id
 *
 * @return object
 */
function avtrans_po_load($id) {
  $id = (int) $id;
  if (empty($id)) {
    return NULL;
  }

  $data = &drupal_static('avbase', array());
  $purchase_orders = &$data['pos'];
  if (isset($purchase_orders[$id])) {
    return $purchase_orders[$id];
  }

  $conditions = array();
  $conditions[] = array('avpo.id', $id);
  $rows = avtrans_po_query_load($conditions);
  $row = empty($rows[$id]) ? FALSE : $rows[$id];

  // Update static variable.
  $purchase_orders[$id] = $row;
  return $row;
}

/**
 * Load purchase order details by ids.
 * @param array $ids
 *
 * @return array
 */
function avtrans_po_load_ids($ids) {
  if (empty($ids)) {
    return FALSE;
  }

  $data = &drupal_static('avbase', array());
  $purchase_orders = &$data['pos'];

  $return = array();

  $query_ids = array();
  foreach ($ids as $id) {
    if (!empty($id)) {
      if (isset($purchase_orders[$id])) {
        $return[$id] = $purchase_orders[$id];
      }
      else {
        $query_ids[] = $id;
      }
    }
  }

  if (!empty($query_ids)) {
    $conditions = array();
    $conditions[] = array('avpo.id', $query_ids, 'IN');
    $rows = avtrans_po_query_load($conditions);
    if (!empty($rows)) {
      $return += $rows;
    }
  }

  return $return;
}

/**
 * Advanced po load.
 * @param array $conditions
 *
 * @return array
 */
function avtrans_po_query_load($conditions = array(), $offset = NULL, $limit = NULL) {
  if (empty($conditions)) {
    return array();
  }

  // Load products static variable.
  $avbase = &drupal_static('avbase', array());
  $avbase['pos'] = empty($avbase['pos']) ? array() : $avbase['pos'];
  $purchase_orders = &$avbase['pos'];

  // Set/load static variable for this query.
  $data = &drupal_static(__FUNCTION__, array());
  $_data_id = $conditions;
  sort($_data_id);
  $data_id = md5(json_encode($_data_id));
  if (isset($data[$data_id])) {
    return $data[$data_id];
  }

  $query = db_select('avtbl_po', 'avpo');
  $query->leftJoin('avtbl_vendors', 'avvendors', 'avvendors.id = avpo.client_id');
  //$query->leftJoin('avtbl_categories', 'avcat', "avcat.id = avpo.term_id AND avcat.group_id = 'payment_term'");
  $query->addField('avvendors', 'display_name', 'client_name');
  //$query->addField('avcat', 'title', 'payment_term_title');
  $query->fields('avpo');
  //->fields('avvendors', array('display_name'))
  //->condition('avpo.id', $id)
  //->execute();
  foreach ($conditions as $condition) {
    $field_name = $condition[0];
    $field_value = isset($condition[1]) ? $condition[1] : NULL;
    $field_operator = isset($condition[2]) ? $condition[2] : NULL;
    $query->condition($field_name, $field_value, $field_operator);
  }

  if (isset($offset) && isset($limit)) {
    $query->range($offset, $limit);
  }

  $query->orderBy('transaction_date', 'DESC');
  $rs = $query->execute();
  $rows = array();
  foreach ($rs as $row) {
    $subtotal = 0;
    if (!empty($row->id)) {
      $query = db_select('avtbl_po_items', 'avpo_items');
      $query->leftJoin('avtbl_products', 'avprod', 'avprod.id = avpo_items.product_id');
      $query->leftJoin('avtbl_categories', 'avcat', "avcat.id = avpo_items.uom_id AND avcat.group_id = 'uom'");
      $query->leftJoin('avtbl_categories', 'avcat_base', "avcat_base.id = avprod.uom_id AND avcat.group_id = 'uom'");
      $query->addField('avprod', 'title', 'product_title');
      $query->addField('avprod', 'uom_id', 'base_uom_id');
      $query->addField('avcat', 'title', 'uom_title');
      $query->addField('avcat_base', 'title', 'base_uom_title');
      $query->fields('avpo_items')
        ->condition('avpo_items.po_id', $row->id)
        ->execute();
      $rs = $query->execute();
      foreach ($rs as $item) {
        $item->total = $item->cost * $item->qty;
        $item->qty_per_uom = empty($item->qty_per_uom) ? 1 : $item->qty_per_uom;
        $item->incoming_base_qty = $item->qty * $item->qty_per_uom;
        $item->remaining_base_qty = $item->incoming_base_qty;
        $row->items[$item->id] = $item;
        $subtotal += $item->total;
      }
    }
    $row->discount_value_text = $row->discount_type == AVBASE_DISCOUNT_PERCENT ? (round($row->discount_value, 2) . '%') : number_format($row->discount_value, 2);
    $row->subtotal = $subtotal;
    $row->discounted_value = $row->discount_type == AVBASE_DISCOUNT_PERCENT ? (($row->discount_value / 100) * $subtotal) : $row->discount_value;
    $row->grand_total = $subtotal - $row->discounted_value;

    // Attach received items.
    avtrans_po_attach_received_items($row);

    $rows[$row->id] = $row;
  }


  // Update static variables.
  $data[$data_id] = $rows;
  $purchase_orders += $rows;
  return $rows;
}

/**
 * Loads purchase order details and sends it to specified email.
 * If email is not specified, po email will be used.
 * @param $po_id
 * @param string $email
 *
 * @return bool
 */
function avtrans_po_send_to_email($po_id, $email = '') {
  if (empty($po_id)) {
    return FALSE;
  }

  $po = avtrans_po_load($po_id);
  $email = $po->email;
  if ($error = user_validate_mail($email)) {
    drupal_set_message($error, 'error');
    return FALSE;
  }

  $po->discount_value = round($po->discount_value, 2);
  $params = array(
    'po' => $po,
    'po_num' => $po->id,
    'transaction_date' => format_date($po->created, 'custom', 'F d, Y'),
    'site_name' => variable_get('site_name', 'UCT'),
    'shipping_address' => variable_get('av_company_address', '-'),
    'vendor_name' => check_plain($po->client_name),
    //'payment_term_title' => check_plain($po->payment_term_title),
  );

  $params['po_table'] = theme('avtrans_po_mail_table', $params);
  $message = drupal_mail('avtrans', 'po', $email, language_default(), $params);
  if (!empty($message['result'])) {
    drupal_set_message(t('Purchase order sent successfully to %mail.', array('%mail' => $email)));
    avbase_custom_watchdog("crud_mail_po", "Sent Purchase Order #@po_id to @vendor_name",
      array('@po_id' => $po->id, '@vendor_name' => $po->client_name , '#message' => $message)
    );
    return TRUE;
  }
  return FALSE;
}

function avtrans_po_attach_received_items(&$po) {
  if (empty($po->id) || empty($po->items)) {
    return;
  }

  $po_items = &$po->items;
  $reference_ids = array_keys($po_items);
  $query = db_select('avtbl_gr_items', 'avgr_items');
  //$query->leftJoin('avtbl_vendors', 'avvendors', 'avvendors.id = avpo.client_id');
  //$query->leftJoin('avtbl_categories', 'avcat', "avcat.id = avpo.term_id AND avcat.group_id = 'payment_term'");
  //$query->addField('avvendors', 'display_name', 'vendor_name');
  //$query->addField('avcat', 'title', 'payment_term_title');
  $query->fields('avgr_items');
  //->fields('avvendors', array('display_name'))
  $query->condition('avgr_items.reference_id', $reference_ids, 'IN');
  $rs = $query->execute();
  $received_items = array();
  foreach ($rs as $received_item) {
    $reference_id = $received_item->reference_id;
    if (empty($po_items[$reference_id])) {
      continue;
    }
    $gr_item_id = $received_item->id;

    // Compute for total base qty received from this gr_item.
    $received_item->received_base_qty = $received_item->qty * $received_item->qty_per_uom;

    // Compute for total base qty received from this po_item.
    $po_items[$reference_id]->received_base_qty = empty($po_items[$reference_id]->received_base_qty) ? 0 : $po_items[$reference_id]->received_base_qty;
    $po_items[$reference_id]->received_base_qty += $received_item->received_base_qty;

    // Set remaining qty to be received.
    $po_items[$reference_id]->remaining_base_qty -= $received_item->received_base_qty;

    //$grand_received_base_qty += $received_item->received_base_qty;
    $received_items[$reference_id][$gr_item_id] = $received_item;
  }

  //dpm($grand_received_base_qty)l
  //dpm()

  foreach ($received_items as $k => $v) {
    $po_items[$k]->received_items = $v;
  }

  // Compute total remaining base qty for this PO.
  $total_remaining_base_qty = 0;
  foreach ($po_items as $po_item) {
    $total_remaining_base_qty += $po_item->remaining_base_qty;
  }

  if (empty($total_remaining_base_qty)) {
    // Close this PO.
    db_query("UPDATE {avtbl_po} SET status = 0 WHERE id = :id", array(':id' => $po->id));
    $po->status = 0;
  }
}
