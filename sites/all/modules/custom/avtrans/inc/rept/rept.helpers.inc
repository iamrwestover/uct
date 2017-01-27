<?php

/**
 * Load delivery details.
 * @param $id
 *
 * @return object
 */
function avtrans_rept_load($id) {
  if (empty($id) || $id == 'new') {
    return FALSE;
  }

  $id = (int) $id;
  if (empty($id)) {
    return NULL;
  }

  $data = &drupal_static('avbase', array());
  $repts = &$data['repts'];
  if (isset($repts[$id])) {
    return $repts[$id];
  }

  $conditions = array();
  $conditions[] = array('avrept.id', $id);
  $rows = avtrans_rept_query_load($conditions);
  $row = empty($rows[$id]) ? FALSE : $rows[$id];

  // Update static variable.
  $repts[$id] = $row;
  return $row;
}

/**
 * Load delivery details by ids.
 * @param array $ids
 *
 * @return array
 */
function avtrans_rept_load_ids($ids) {
  if (empty($ids)) {
    return FALSE;
  }

  $data = &drupal_static('avbase', array());
  $repts = &$data['repts'];

  $return = array();

  $query_ids = array();
  foreach ($ids as $id) {
    if (!empty($id)) {
      if (isset($repts[$id])) {
        $return[$id] = $repts[$id];
      }
      else {
        $query_ids[] = $id;
      }
    }
  }

  if (!empty($query_ids)) {
    $conditions = array();
    $conditions[] = array('avrept.id', $query_ids, 'IN');
    $rows = avtrans_rept_query_load($conditions);
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
function avtrans_rept_query_load($conditions = array()) {
  if (empty($conditions)) {
    return array();
  }

  // Load products static variable.
  $avbase = &drupal_static('avbase', array());
  $avbase['repts'] = empty($avbase['repts']) ? array() : $avbase['repts'];
  $repts = &$avbase['repts'];

  // Set / load static variable for this query.
  $data = &drupal_static(__FUNCTION__, array());
  $_data_id = $conditions;
  sort($_data_id);
  $data_id = md5(json_encode($_data_id));
  if (isset($data[$data_id])) {
    return $data[$data_id];
  }

  $query = db_select('avtbl_rept', 'avrept');
  $query->leftJoin('avtbl_vendors', 'avvendors', 'avvendors.id = avrept.client_id');
  $query->leftJoin('avtbl_categories', 'avcat', "avcat.id = avrept.term_id AND avcat.group_id = 'payment_term'");
  $query->leftJoin('avtbl_categories', 'avcat_agent', "avcat_agent.id = avrept.agent_id AND avcat_agent.group_id = 'agent_vend'");
  $query->addField('avvendors', 'display_name', 'client_name');
  $query->addField('avcat', 'title', 'payment_term_title');
  $query->addField('avcat_agent', 'title', 'agent_name');
  $query->fields('avrept');
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
      $query = db_select('avtbl_rept_items', 'avrept_items');
      $query->leftJoin('avtbl_products', 'avprod', 'avprod.id = avrept_items.product_id');
      $query->leftJoin('avtbl_categories', 'avcat', "avcat.id = avrept_items.uom_id AND avcat.group_id = 'uom'");
      $query->addField('avprod', 'title', 'product_title');
      $query->addField('avcat', 'title', 'uom_title');
      $query->fields('avrept_items')
        ->condition('avrept_items.rept_id', $row->id)
        ->execute();
      $rs = $query->execute();
      foreach ($rs as $item) {
        $item->total = $item->cost * $item->qty;
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

    $rows[$row->id] = $row;
  }


  // Update static variables.
  $data[$data_id] = $rows;
  $repts += $rows;
  return $rows;
}
