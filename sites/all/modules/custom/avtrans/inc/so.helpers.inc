<?php

/**
 * Load sales order details.
 * @param $id
 *
 * @return object
 */
function avtrans_so_load($id) {
  if (empty($id) || $id == 'new') {
    return FALSE;
  }

  $id = (int) $id;
  if (empty($id)) {
    return NULL;
  }

  $data = &drupal_static('avbase', array());
  $sales_orders = &$data['sos'];
  if (isset($sales_orders[$id])) {
    return $sales_orders[$id];
  }

  $conditions = array();
  $conditions[] = array('avso.id', $id);
  $rows = avtrans_so_query_load($conditions);
  $row = empty($rows[$id]) ? FALSE : $rows[$id];

  // Update static variable.
  $sales_orders[$id] = $row;
  return $row;
}

/**
 * Load sales order details by ids.
 * @param array $ids
 *
 * @return array
 */
function avtrans_so_load_ids($ids) {
  if (empty($ids)) {
    return FALSE;
  }

  $data = &drupal_static('avbase', array());
  $sales_orders = &$data['sos'];

  $return = array();

  $query_ids = array();
  foreach ($ids as $id) {
    if (!empty($id)) {
      if (isset($sales_orders[$id])) {
        $return[$id] = $sales_orders[$id];
      }
      else {
        $query_ids[] = $id;
      }
    }
  }

  if (!empty($query_ids)) {
    $conditions = array();
    $conditions[] = array('avso.id', $query_ids, 'IN');
    $rows = avtrans_so_query_load($conditions);
    if (!empty($rows)) {
      $return += $rows;
    }
  }

  return $return;
}

/**
 * Advanced so load.
 * @param array $conditions
 *
 * @return array
 */
function avtrans_so_query_load($conditions = array()) {
  if (empty($conditions)) {
    return array();
  }

  // Load products static variable.
  $avbase = &drupal_static('avbase', array());
  $avbase['sos'] = empty($avbase['sos']) ? array() : $avbase['sos'];
  $sales_orders = &$avbase['sos'];

  // Set / load static variable for this query.
  $data = &drupal_static(__FUNCTION__, array());
  $_data_id = $conditions;
  sort($_data_id);
  $data_id = md5(json_encode($_data_id));
  if (isset($data[$data_id])) {
    return $data[$data_id];
  }

  $query = db_select('avtbl_so', 'avso');
  $query->leftJoin('avtbl_customers', 'avcustomers', 'avcustomers.id = avso.client_id');
  $query->leftJoin('avtbl_categories', 'avcat_agent', "avcat_agent.id = avso.agent_id AND avcat_agent.group_id = 'agent_cust'");
  $query->addField('avcustomers', 'display_name', 'client_name');
  $query->addField('avcat_agent', 'title', 'agent_name');
  $query->fields('avso');
  //->fields('avcustomers', array('display_name'))
  //->condition('avso.id', $id)
  //->execute();
  foreach ($conditions as $condition) {
    $field_name = $condition[0];
    $field_value = isset($condition[1]) ? $condition[1] : NULL;
    $field_operator = isset($condition[2]) ? $condition[2] : NULL;
    $query->condition($field_name, $field_value, $field_operator);
  }

  $query->orderBy('transaction_date', 'DESC');
  $rs = $query->execute();
  $rows = array();
  foreach ($rs as $row) {
    $subtotal = 0;
    if (!empty($row->id)) {
      $query = db_select('avtbl_so_items', 'avso_items');
      $query->leftJoin('avtbl_products', 'avprod', 'avprod.id = avso_items.product_id');
      $query->leftJoin('avtbl_categories', 'avcat', "avcat.id = avso_items.uom_id AND avcat.group_id = 'uom'");
      $query->leftJoin('avtbl_categories', 'avcat_base', "avcat_base.id = avprod.uom_id AND avcat.group_id = 'uom'");
      $query->addField('avprod', 'title', 'product_title');
      $query->addField('avprod', 'uom_id', 'base_uom_id');
      $query->addField('avcat', 'title', 'uom_title');
      $query->addField('avcat_base', 'title', 'base_uom_title');
      $query->fields('avso_items')
        ->condition('avso_items.so_id', $row->id)
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

    // Attach delivered items.
    avtrans_so_attach_delivered_items($row);

    $rows[$row->id] = $row;
  }


  // Update static variables.
  $data[$data_id] = $rows;
  $sales_orders += $rows;
  return $rows;
}

function avtrans_so_attach_delivered_items(&$dr) {
  if (empty($dr->id) || empty($dr->items)) {
    return;
  }

  $dr_items = &$dr->items;
  $reference_ids = array_keys($dr_items);
  $query = db_select('avtbl_dr_items', 'avdr_items');
  //$query->leftJoin('avtbl_vendors', 'avvendors', 'avvendors.id = avpo.client_id');
  //$query->leftJoin('avtbl_categories', 'avcat', "avcat.id = avpo.term_id AND avcat.group_id = 'payment_term'");
  //$query->addField('avvendors', 'display_name', 'vendor_name');
  //$query->addField('avcat', 'title', 'payment_term_title');
  $query->fields('avdr_items');
  //->fields('avvendors', array('display_name'))
  $query->condition('avdr_items.reference_id', $reference_ids, 'IN');
  $rs = $query->execute();
  $delivered_items = array();
  foreach ($rs as $delivered_item) {
    $reference_id = $delivered_item->reference_id;
    if (empty($dr_items[$reference_id])) {
      continue;
    }
    $gr_item_id = $delivered_item->id;

    // Compute for total base qty delivered from this gr_item.
    $delivered_item->delivered_base_qty = $delivered_item->qty * $delivered_item->qty_per_uom;

    // Compute for total base qty delivered from this po_item.
    $dr_items[$reference_id]->delivered_base_qty = empty($dr_items[$reference_id]->delivered_base_qty) ? 0 : $dr_items[$reference_id]->delivered_base_qty;
    $dr_items[$reference_id]->delivered_base_qty += $delivered_item->delivered_base_qty;

    // Set remaining qty to be delivered.
    $dr_items[$reference_id]->remaining_base_qty -= $delivered_item->delivered_base_qty;

    //$grand_delivered_base_qty += $delivered_item->delivered_base_qty;
    $delivered_items[$reference_id][$gr_item_id] = $delivered_item;
  }

  //dpm($grand_delivered_base_qty)l
  //dpm()

  foreach ($delivered_items as $k => $v) {
    $dr_items[$k]->delivered_items = $v;
  }

  // Compute total remaining base qty for this Delivery.
  $total_remaining_base_qty = 0;
  foreach ($dr_items as $dr_item) {
    $total_remaining_base_qty += $dr_item->remaining_base_qty;
  }

  if (empty($total_remaining_base_qty)) {
    // Close this PO.
    db_query("UPDATE {avtbl_so} SET status = 0 WHERE id = :id", array(':id' => $dr->id));
    $dr->status = 0;
  }
}
