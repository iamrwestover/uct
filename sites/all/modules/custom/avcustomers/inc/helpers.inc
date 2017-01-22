<?php

/**
 * Load customer details by id.
 *
 * @param $customer_id
 * @return object
*/
function avcustomer_load($customer_id) {
  if (empty($customer_id) || $customer_id == 'add') {
    return FALSE;
  }

  $data = &drupal_static('avbase', array());
  $_customers = &$data['customers'];
  if (isset($_customers[$customer_id])) {
    return $_customers[$customer_id];
  }

  $where = array('avcustomers.id = :customer_id');
  $args = array(':customer_id' => $customer_id);
  $rows = avcustomer_query_load($where, $args);
  $row = empty($rows[$customer_id]) ? FALSE : $rows[$customer_id];

  // Update static variable.
  $_customers[$customer_id] = $row;
  return $row;
}

/**
 * Load customers by ids.
 * @param array $ids
 *
 * @return array
 */
function avcustomer_load_ids($ids) {
  if (empty($ids)) {
    return FALSE;
  }

  $data = &drupal_static('avbase', array());
  $_customers = &$data['customers'];

  $return = array();
  $query_ids = array();
  foreach ($ids as $id) {
    if (!empty($id)) {
      if (isset($_customers[$id])) {
        $return[$id] = $_customers[$id];
      }
      else {
        $query_ids[] = $id;
      }
    }
  }

  if (!empty($query_ids)) {
    $where = array('avcustomers.id IN(:customer_ids)');
    $args = array(':customer_ids' => $query_ids);
    $rows = avcustomer_query_load($where, $args);
    if (!empty($rows)) {
      $return += $rows;
    }
  }

  return $return;
}

/**
 * Advanced customer load.
 * @param array $param_where
 * @param array $param_args
 * @param $limit_string
 *
 * @return array
 */
function avcustomer_query_load($param_where, $param_args = array(), $limit_string = '') {
  if (empty($param_where)) {
    return array();
  }

  // Load customers static variable.
  $avbase = &drupal_static('avbase', array());
  $avbase['customers'] = empty($avbase['customers']) ? array() : $avbase['customers'];
  $_customers = &$avbase['customers'];

  // Set / load static variable for this query.
  $data = &drupal_static(__FUNCTION__, array());
  $_data_id = $param_where;
  sort($_data_id);
  $data_id = md5(json_encode($_data_id) . json_encode($param_args));

  if (isset($data[$data_id])) {
    return $data[$data_id];
  }

  $query = "SELECT avcustomers.*, avcat.title as agent_name
    FROM {avtbl_customers} avcustomers
    LEFT JOIN {avtbl_categories} avcat ON avcat.id = avcustomers.agent_id AND avcat.group_id = 'agent_cust'";
  if (!empty($param_where)) {
    $query .= " WHERE " . implode(' AND ', $param_where);
  }
  $query .= " ORDER BY avcustomers.created DESC";
  if (!empty($limit_string)) {
    $query .= " LIMIT " . $limit_string;
  }

  $rs = db_query($query, $param_args);
  $rows = array();
  foreach ($rs as $row) {
    if (isset($row->data)) {
      $row->data = unserialize($row->data);
    }
    $rows[$row->id] = $row;
  }

  // Update static variables.
  $data[$data_id] = $rows;
  $_customers += $rows;

  return $rows;
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

/**
 * Menu callback - retrieve a JSON object containing autocomplete suggestions for existing customers.
 * @param string $string
 */
function avcustomers_autocomplete($string = '') {
  $matches = array();
  $string = trim($string);
  if ($string) {
    //$db_or = db_or();
    //$db_or->condition('avcustomers.display_name', '%' . db_like($string) . '%', 'LIKE');
    //$db_or->condition('avcustomers.company', '%' . db_like($string) . '%', 'LIKE');
    //$db_or->condition('avcustomers.first_name', '%' . db_like($string) . '%', 'LIKE');
    //$db_or->condition('avcustomers.last_name', '%' . db_like($string) . '%', 'LIKE');
    //
    //$query = db_select('avtbl_customers', 'avcust')->fields('avcust', array('id', 'display_name', 'company', 'email', 'term_id', 'discount_type', 'discount_value'));
    //$query->condition($db_or);
    //$query->range(0, 15);
    //$result = $query->execute();
    $where_or = array(
      'avcustomers.display_name LIKE :string',
      'avcustomers.company LIKE :string',
      'avcustomers.first_name LIKE :string',
      'avcustomers.last_name LIKE :string',
    );
    $where_or_str = '(' . implode(' OR ', $where_or) . ')';
    $where = array($where_or_str);
    $args = array(':string' => '%' . db_like($string) . '%');
    $where[] = 'avcustomers.status = :active_status';
    $args[':active_status'] = AVCUSTOMER_STATUS_ACTIVE;
    $rows = avcustomer_query_load($where, $args, '15');
    foreach ($rows as $row) {
      $row_html = check_plain($row->display_name) . '<span class="uk-text-muted"> / ' . check_plain($row->company) .  '</span>';
      $row_html .= '<div style="display: none;" id="av-row-id">' . $row->id . '</div>';
      $row_html .= '<div style="display: none;" id="av-row-json">' . drupal_json_encode($row) . '</div>';
      $matches[$row->display_name] = $row_html;
    }
  }

  drupal_json_output($matches);
}

/**
 * Return definition of customer table fields.
 * @param $db_op
 * @return array
 */
function avcustomers_table_fields_to_save($db_op = 'insert') {
  $fields = array(
    'created',
    'first_name',
    'last_name',
    'middle_name',
    'company',
    'display_name',
    //'code',
    'agent_id' => array('type' => 'int'),
    'email',
    'address',
    'city',
    'province',
    'zip_code',
    //'same_addr',
    'payment_method_id' => array('type' => 'int'),
    'term_id' => array('type' => 'int'),
    'discount_type' => array('type' => 'int'),
    'discount_value' => array('type' => 'float'),
    'category_id' => array('type' => 'int'),
    'credit_limit' => array('type' => 'float'),
    'opening_balance' => array('type' => 'float'),
    'data' => array(
      'phone1',
      'phone2',
      'phone3',
    ),
  );

  if ($db_op == 'update') {
    unset($fields['created']);
    unset($fields['opening_balance']);
  }
  return $fields;
}
