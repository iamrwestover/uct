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
  return FALSE;
}

/**
 * Change customer status.
 * @param $id
 * @param $status
 *
 * @return bool
 */
function avcustomers_customer_change_status($id, $status) {
  if (empty($id) || !is_numeric($id)) {
    return FALSE;
  }
  if (!array_key_exists($status, avcustomers_statuses())) {
    return FALSE;
  }

  if (db_query("UPDATE {avtbl_customers} SET status = :status WHERE id = :customer_id", array(':customer_id' => $id, ':status' => $status))) {
    return TRUE;
  }
  return FALSE;
}

/**
 * Return list of customer statuses.
 */
function avcustomers_statuses() {
  return array(
    AVCUSTOMER_STATUS_DISABLED => 'Disabled',
    AVCUSTOMER_STATUS_ACTIVE => 'Active',
    AVCUSTOMER_STATUS_SUSPENDED => 'Suspended',
  );
}