<?php

/**
 * Load customer details.
 *
 * @param $customer_id
 * @return mixed
 */
function avcustomer_load($customer_id) {
  $data = &drupal_static(__FUNCTION__, array());
  if (isset($data[$customer_id])) {
    return $data[$customer_id];
  }

  $query = "SELECT avcust.*, avcat.title as agent_name
  FROM {avtbl_customers} avcust
  LEFT JOIN {avtbl_categories} avcat ON avcat.id = avcust.agent_id AND avcat.group_id = 'agent_cust'
  WHERE avcust.id = :customer_id";
  $args = array(':customer_id' => $customer_id);
  $row = db_query($query, $args)->fetchObject();
  if (isset($row->data)) {
    $row->data = unserialize($row->data);
  }

  $data[$customer_id] = $row;
  return $row;
}

/**
 * Delete customer.
 * @param $id
 *
 * @return bool
 */
function avcustomers_customer_delete($id) {
  if (!empty($id) && is_numeric($id)) {
    if (db_query("DELETE FROM {avtbl_customers} WHERE id = :customer_id", array(':customer_id' => $id))) {
      return TRUE;
    }
  }
}
