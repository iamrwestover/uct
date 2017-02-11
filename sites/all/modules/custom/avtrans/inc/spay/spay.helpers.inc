<?php

/**
 * Load delivery details.
 * @param $id
 *
 * @return object
 */
function avtrans_spay_load($id) {
  if (empty($id) || $id == 'new') {
    return FALSE;
  }

  $id = (int) $id;
  if (empty($id)) {
    return NULL;
  }

  $data = &drupal_static('avbase', array());
  $spays = &$data['spays'];
  if (isset($spays[$id])) {
    return $spays[$id];
  }

  $conditions = array();
  $conditions[] = array('avspay.id', $id);
  $rows = avtrans_spay_query_load($conditions);
  $row = empty($rows[$id]) ? FALSE : $rows[$id];

  // Update static variable.
  $spays[$id] = $row;
  return $row;
}

/**
 * Load delivery details by ids.
 * @param array $ids
 *
 * @return array
 */
function avtrans_spay_load_ids($ids) {
  if (empty($ids)) {
    return FALSE;
  }

  $data = &drupal_static('avbase', array());
  $spays = &$data['spays'];

  $return = array();

  $query_ids = array();
  foreach ($ids as $id) {
    if (!empty($id)) {
      if (isset($spays[$id])) {
        $return[$id] = $spays[$id];
      }
      else {
        $query_ids[] = $id;
      }
    }
  }

  if (!empty($query_ids)) {
    $conditions = array();
    $conditions[] = array('avspay.id', $query_ids, 'IN');
    $rows = avtrans_spay_query_load($conditions);
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
function avtrans_spay_query_load($conditions = array()) {
  if (empty($conditions)) {
    return array();
  }

  // Load products static variable.
  $avbase = &drupal_static('avbase', array());
  $avbase['spays'] = empty($avbase['spays']) ? array() : $avbase['spays'];
  $spays = &$avbase['spays'];

  // Set / load static variable for this query.
  $data = &drupal_static(__FUNCTION__, array());
  $_data_id = $conditions;
  sort($_data_id);
  $data_id = md5(json_encode($_data_id));
  if (isset($data[$data_id])) {
    return $data[$data_id];
  }

  $query = db_select('avtbl_spay', 'avspay');
  $query->leftJoin('avtbl_customers', 'avcustomers', 'avcustomers.id = avspay.client_id');
  $query->leftJoin('avtbl_categories', 'avcat', "avcat.id = avspay.term_id AND avcat.group_id = 'payment_term'");
  $query->leftJoin('avtbl_categories', 'avcat_agent', "avcat_agent.id = avspay.agent_id AND avcat_agent.group_id = 'agent_cust'");
  $query->addField('avcustomers', 'display_name', 'client_name');
  $query->addField('avcat', 'title', 'payment_term_title');
  $query->addField('avcat_agent', 'title', 'agent_name');
  $query->fields('avspay');
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
      $query = "SELECT avspay_items.*
        FROM {avtbl_spay_items} avspay_items
        WHERE avspay_items.spay_id = :spay_id";
      $items_rs = db_query($query, array(':spay_id' => $row->id));
      //$query = db_select('avtbl_spay_items', 'avspay_items');
      ////$query->leftJoin('avtbl_products', 'avprod', 'avprod.id = avspay_items.product_id');
      ////$query->leftJoin('avtbl_categories', 'avcat', "avcat.id = avspay_items.uom_id AND avcat.group_id = 'uom'");
      ////$query->addField('avprod', 'title', 'product_title');
      ////$query->addField('avcat', 'title', 'uom_title');
      //$query->fields('avspay_items')
      //  ->condition('avspay_items.spay_id', $row->id)
      //  ->execute();
      //$rs = $query->execute();
      $spay_grand_total = 0;
      foreach ($items_rs as $item) {
        if (!empty($item->reference_id)) {
          $query = db_select('avtbl_sinv_items', 'avsinv_items');
          $query->leftJoin('avtbl_sinv', 'avsinv', 'avsinv.id = avsinv_items.sinv_id');
          $query->leftJoin('avtbl_products', 'avprod', 'avprod.id = avsinv_items.product_id');
          $query->leftJoin('avtbl_categories', 'avcat', "avcat.id = avsinv_items.uom_id AND avcat.group_id = 'uom'");
          $query->addField('avsinv', 'transaction_date');
          $query->addField('avsinv', 'status');
          $query->addField('avprod', 'title', 'product_title');
          $query->addField('avcat', 'title', 'uom_title');
          $query->fields('avsinv_items')
            ->condition('avsinv_items.sinv_id', $item->reference_id)
            ->execute();
          $sinv_rs = $query->execute();
          $subtotal = 0;
          foreach ($sinv_rs as $sinv_item) {
            $sinv_item->total = $sinv_item->cost * $sinv_item->qty;
            $discount = empty($sinv_item->discount) ? array() : explode('/', $sinv_item->discount);
            foreach ($discount as $v) {
              if (is_numeric($v)) {
                $v = (float) $v;
                $sinv_item->total = $sinv_item->total - (($sinv_item->total * $v) / 100);
              }
            }
            $sinv_item->total *= $sinv_item->reference_type == 'ret' ? -1 : 1;
            $sinv_item->qty_per_uom = empty($sinv_item->qty_per_uom) ? 1 : $sinv_item->qty_per_uom;
            $sinv_item->incoming_base_qty = $sinv_item->qty * $sinv_item->qty_per_uom;
            //$row->items[$sinv_item->id] = $sinv_item;
            $subtotal += $sinv_item->total;
            $item->transaction_date_formatted = format_date($sinv_item->transaction_date, 'custom', 'Y M d, h:i a');
            $item->status = $sinv_item->status;
          }
          $item->grand_total = $subtotal;

          $query = "SELECT SUM(paid_amount) FROM {avtbl_spay_items} WHERE reference_id = :invoice_id";
          $total_paid_for_invoice = db_query($query, array(':invoice_id' => $item->reference_id))->fetchField();
          $total_paid_for_invoice = empty($total_paid_for_invoice) ? 0 : $total_paid_for_invoice;
          $item->balance = avbase_number_format($subtotal - $total_paid_for_invoice + $item->paid_amount, 2);
          $item->previous_payment = avbase_number_format($total_paid_for_invoice - $item->paid_amount, 2);
          $spay_grand_total += $item->paid_amount;

          // Close invoice if necessary.
          if (round($total_paid_for_invoice, 2) == round($item->grand_total, 2)) {
            if (db_query("UPDATE {avtbl_sinv} SET status = :status WHERE id = :invoice_id", array(':invoice_id' => $item->reference_id, ':status' => (empty($item->status) ? 1 : 0)))) {
              avbase_custom_watchdog("sinv__close", "ID: {$item->reference_id}", array('#row' => $item, '#manner' => 'payment-auto'));
            }
          }
        }

        $row->items[$item->id] = $item;
        //$subtotal += $item->total;
      }
      $row->grand_total = $spay_grand_total;
    }
    //$row->discount_value_text = $row->discount_type == AVBASE_DISCOUNT_PERCENT ? (round($row->discount_value, 2) . '%') : number_format($row->discount_value, 2);
    //$row->subtotal = $subtotal;
    //$row->discounted_value = $row->discount_type == AVBASE_DISCOUNT_PERCENT ? (($row->discount_value / 100) * $subtotal) : $row->discount_value;
    //$row->grand_total = $subtotal/* - $row->discounted_value*/;

    $rows[$row->id] = $row;
  }

  // Update static variables.
  $data[$data_id] = $rows;
  $spays += $rows;
  return $rows;
}
