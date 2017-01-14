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

  $data = &drupal_static('avbase', array());
  $purchase_orders = &$data['pos'];
  if (isset($purchase_orders[$id])) {
    return $purchase_orders[$id];
  }

  $conditions = array();
  $conditions[] = array('avpo.id', $id);
  $rows = avpurchase_po_query_load($conditions);
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
function avpurchase_po_load_ids($ids) {
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
    $rows = avpurchase_po_query_load($conditions);
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
function avpurchase_po_query_load($conditions = array()) {
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
  $query->leftJoin('avtbl_vendors', 'avvendors', 'avvendors.id = avpo.vendor_id');
  //$query->leftJoin('avtbl_categories', 'avcat', "avcat.id = avpo.term_id AND avcat.group_id = 'payment_term'");
  $query->addField('avvendors', 'display_name', 'vendor_name');
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

  $query->orderBy('po_date', 'DESC');
  $rs = $query->execute();
  $rows = array();
  foreach ($rs as $row) {
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
        $item->qty_per_uom = empty($item->qty_per_uom) ? 1 : $item->qty_per_uom;
        $row->items[$item->id] = $item;
        $subtotal += $item->total;
      }
    }
    $row->discount_value_text = $row->discount_type == AVBASE_DISCOUNT_PERCENT ? (round($row->discount_value, 2) . '%') : number_format($row->discount_value, 2);
    $row->subtotal = $subtotal;
    $row->discounted_value = $row->discount_type == AVBASE_DISCOUNT_PERCENT ? (($row->discount_value / 100) * $subtotal) : $row->discount_value;
    $row->grand_total = $subtotal - $row->discounted_value;

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
    //'payment_term_title' => check_plain($po->payment_term_title),
  );

  $params['po_table'] = theme('avpurchase_po_mail_table', $params);
  $message = drupal_mail('avpurchase', 'po', $email, language_default(), $params);
  if (!empty($message['result'])) {
    drupal_set_message(t('Purchase order sent successfully to %mail.', array('%mail' => $email)));
    avbase_custom_watchdog("crud_mail_po", "Sent Purchase Order #@po_id to @vendor_name",
      array('@po_id' => $po->id, '@vendor_name' => $po->vendor_name , '#message' => $message)
    );
    return TRUE;
  }
  return FALSE;
}

/**
 * Compute new product cost.
 * @param float $current_cost
 * @param $onhand_qty
 * @param float $new_cost
 * @param $incoming_qty
 *
 * @return float
 */
function avpurchase_compute_product_cost($current_cost, $onhand_qty, $new_cost, $incoming_qty) {
  $current_cost = empty($current_cost) ? $new_cost : $current_cost;
  return ((($current_cost * $onhand_qty) + ($new_cost * $incoming_qty)) / ($onhand_qty + $incoming_qty));
}

/**
 * Compute new sales price by using the markup difference of current sales price
 * and current cost and adding it to the new cost.
 * @param float $current_sales_price
 * @param float $current_cost
 * @param float $new_cost
 *
 * @return float
 */
function avpurchase_compute_sales_price($current_sales_price, $current_cost, $new_cost) {
  if ($current_sales_price <= $current_cost) {
    $markup = 0.2;
  }
  else {
    $markup = ($current_sales_price - $current_cost) / $current_cost;
  }

  return array('markup' => $markup, 'price' => ($new_cost + ($new_cost * $markup)));
}

function avpurchase_po_attach_received_items(&$po) {
  if (empty($po->id) || empty($po->items)) {
    return;
  }

  $po_items = &$po->items;
  $po_item_ids = array_keys($po_items);
  $query = db_select('avtbl_gr_items', 'avgr_items');
  //$query->leftJoin('avtbl_vendors', 'avvendors', 'avvendors.id = avpo.vendor_id');
  //$query->leftJoin('avtbl_categories', 'avcat', "avcat.id = avpo.term_id AND avcat.group_id = 'payment_term'");
  //$query->addField('avvendors', 'display_name', 'vendor_name');
  //$query->addField('avcat', 'title', 'payment_term_title');
  $query->fields('avgr_items');
  //->fields('avvendors', array('display_name'))
  $query->condition('avgr_items.po_item_id', $po_item_ids, 'IN');
  $rs = $query->execute();
  $received_items = array();
  foreach ($rs as $received_item) {
    $received_items[$received_item->po_item_id] = $received_item;
  }

  foreach ($received_items as $k => $v) {
    $po_items->$k->received_items = $v;
  }
}