<?php

/**
 * Load delivery details.
 * @param $id
 *
 * @return object
 */
function avtrans_sinv_load($id) {
  if (empty($id) || $id == 'new') {
    return FALSE;
  }

  $id = (int) $id;
  if (empty($id)) {
    return NULL;
  }

  $data = &drupal_static('avbase', array());
  $sinvs = &$data['sinvs'];
  if (isset($sinvs[$id])) {
    return $sinvs[$id];
  }

  $conditions = array();
  $conditions[] = array('avsinv.id', $id);
  $rows = avtrans_sinv_query_load($conditions);
  $row = empty($rows[$id]) ? FALSE : $rows[$id];

  // Update static variable.
  $sinvs[$id] = $row;
  return $row;
}

/**
 * Load delivery details by ids.
 * @param array $ids
 *
 * @return array
 */
function avtrans_sinv_load_ids($ids) {
  if (empty($ids)) {
    return FALSE;
  }

  $data = &drupal_static('avbase', array());
  $sinvs = &$data['sinvs'];

  $return = array();

  $query_ids = array();
  foreach ($ids as $id) {
    if (!empty($id)) {
      if (isset($sinvs[$id])) {
        $return[$id] = $sinvs[$id];
      }
      else {
        $query_ids[] = $id;
      }
    }
  }

  if (!empty($query_ids)) {
    $conditions = array();
    $conditions[] = array('avsinv.id', $query_ids, 'IN');
    $rows = avtrans_sinv_query_load($conditions);
    if (!empty($rows)) {
      $return += $rows;
    }
  }

  return $return;
}

/**
 * Advanced delivery load.
 * @param array $conditions
 *
 * @return array
 */
function avtrans_sinv_query_load($conditions = array()) {
  if (empty($conditions)) {
    return array();
  }

  // Load products static variable.
  $avbase = &drupal_static('avbase', array());
  $avbase['sinvs'] = empty($avbase['sinvs']) ? array() : $avbase['sinvs'];
  $sinvs = &$avbase['sinvs'];

  // Set / load static variable for this query.
  $data = &drupal_static(__FUNCTION__, array());
  $_data_id = $conditions;
  sort($_data_id);
  $data_id = md5(json_encode($_data_id));
  if (isset($data[$data_id])) {
    return $data[$data_id];
  }

  $query = db_select('avtbl_sinv', 'avsinv');
  $query->leftJoin('avtbl_customers', 'avcustomers', 'avcustomers.id = avsinv.client_id');
  $query->leftJoin('avtbl_categories', 'avcat', "avcat.id = avsinv.term_id AND avcat.group_id = 'payment_term'");
  $query->leftJoin('avtbl_categories', 'avcat_agent', "avcat_agent.id = avsinv.agent_id AND avcat_agent.group_id = 'agent_cust'");
  $query->addField('avcustomers', 'display_name', 'client_name');
  $query->addField('avcat', 'title', 'payment_term_title');
  $query->addField('avcat_agent', 'title', 'agent_name');
  $query->fields('avsinv');
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
      $query = db_select('avtbl_sinv_items', 'avsinv_items');
      $query->leftJoin('avtbl_products', 'avprod', 'avprod.id = avsinv_items.product_id');
      $query->leftJoin('avtbl_categories', 'avcat', "avcat.id = avsinv_items.uom_id AND avcat.group_id = 'uom'");
      $query->addField('avprod', 'title', 'product_title');
      $query->addField('avcat', 'title', 'uom_title');
      $query->fields('avsinv_items')
        ->condition('avsinv_items.sinv_id', $row->id)
        ->execute();
      $items_rs = $query->execute();
      foreach ($items_rs as $item) {
        $item->total = $item->cost * $item->qty;
        $discount = empty($item->discount) ? array() : explode('/', $item->discount);
        foreach ($discount as $v) {
          if (is_numeric($v)) {
            $v = (float) $v;
            $item->total = $item->total - (($item->total * $v) / 100);
          }
        }
        $item->total *= $item->reference_type == 'ret' ? -1 : 1;
        $item->qty_per_uom = empty($item->qty_per_uom) ? 1 : $item->qty_per_uom;
        $item->incoming_base_qty = $item->qty * $item->qty_per_uom;
        $row->items[$item->id] = $item;
        $subtotal += $item->total;
      }
    }
    //$row->discount_value_text = $row->discount_type == AVBASE_DISCOUNT_PERCENT ? (round($row->discount_value, 2) . '%') : number_format($row->discount_value, 2);
    $row->subtotal = $subtotal;
    //$row->discounted_value = $row->discount_type == AVBASE_DISCOUNT_PERCENT ? (($row->discount_value / 100) * $subtotal) : $row->discount_value;
    $row->grand_total = $subtotal/* - $row->discounted_value*/;


    // Get related payments.
    $query = "SELECT SUM(paid_amount) FROM {avtbl_spay_items} WHERE reference_id = :invoice_id";
    $total_paid_for_invoice = db_query($query, array(':invoice_id' => $row->id))->fetchField();
    $total_paid_for_invoice = empty($total_paid_for_invoice) ? 0 : $total_paid_for_invoice;
    $row->total_paid = $total_paid_for_invoice;
    //if ($row->status && (round($total_paid_for_invoice, 2) == round($row->grand_total, 2))) {
    //  // Close invoice.
    //  if (db_query("UPDATE {avtbl_sinv} SET status = 0 WHERE id = :invoice_id", array(':invoice_id' => $row->id))) {
    //    avbase_custom_watchdog("sinv__close", "ID: {$row->id}", array('#row' => $row, '#manner' => 'auto'));
    //    $row->status = 0;
    //  }
    //}

    $rows[$row->id] = $row;
  }


  // Update static variables.
  $data[$data_id] = $rows;
  $sinvs += $rows;
  return $rows;
}
