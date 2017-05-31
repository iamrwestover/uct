<?php

/**
 * Load client details by id.
 *
 * @param $client_id
 * @return object
*/
function avclient_load($client_id) {
  if (empty($client_id) || $client_id == 'add') {
    return FALSE;
  }

  $data = &drupal_static('avbase', array());
  $_clients = &$data['clients'];
  if (isset($_clients[$client_id])) {
    return $_clients[$client_id];
  }

  $where = array('avclients.id = :client_id');
  $args = array(':client_id' => $client_id);
  $rows = avclient_query_load($where, $args);
  $row = empty($rows[$client_id]) ? FALSE : $rows[$client_id];

  // Update static variable.
  $_clients[$client_id] = $row;
  return $row;
}

/**
 * Load clients by ids.
 * @param array $ids
 *
 * @return array
 */
function avclient_load_ids($ids) {
  if (empty($ids)) {
    return FALSE;
  }

  $data = &drupal_static('avbase', array());
  $_clients = &$data['clients'];

  $return = array();
  $query_ids = array();
  foreach ($ids as $id) {
    if (!empty($id)) {
      if (isset($_clients[$id])) {
        $return[$id] = $_clients[$id];
      }
      else {
        $query_ids[] = $id;
      }
    }
  }

  if (!empty($query_ids)) {
    $where = array('avclients.id IN(:client_ids)');
    $args = array(':client_ids' => $query_ids);
    $rows = avclient_query_load($where, $args);
    if (!empty($rows)) {
      $return += $rows;
    }
  }

  return $return;
}

/**
 * Advanced client load.
 * @param array $param_where
 * @param array $param_args
 * @param $limit_string
 *
 * @return array
 */
function avclient_query_load($param_where, $param_args = array(), $limit_string = '', $order_by = NULL) {
  if (empty($param_where)) {
    return array();
  }

  // Load clients static variable.
  $avbase = &drupal_static('avbase', array());
  $avbase['clients'] = empty($avbase['clients']) ? array() : $avbase['clients'];
  $_clients = &$avbase['clients'];

  // Set / load static variable for this query.
  $data = &drupal_static(__FUNCTION__, array());
  $_data_id = $param_where;
  sort($_data_id);
  $data_id = md5(json_encode($_data_id) . json_encode($param_args));

  if (isset($data[$data_id])) {
    return $data[$data_id];
  }

  $query = "SELECT
    avclients.*, avcat.title as agent_name,
    avcat_terms.title as term_name,
    avcat_terms.data as term_data
    FROM {avtbl_clients} avclients
    LEFT JOIN {avtbl_categories} avcat ON avcat.id = avclients.agent_id
    LEFT JOIN {avtbl_categories} avcat_terms ON avcat_terms.id = avclients.term_id";
  if (!empty($param_where)) {
    $query .= " WHERE " . implode(' AND ', $param_where);
  }
  $order_by = is_null($order_by) ? array('avclients.created DESC') : $order_by;
  if (!empty($order_by)) {
    $query .= " ORDER BY " . implode(', ', $order_by);
  }
  if (!empty($limit_string)) {
    $query .= " LIMIT " . $limit_string;
  }

  $rs = db_query($query, $param_args);
  $rows = array();
  foreach ($rs as $row) {
    if (isset($row->data)) {
      $row->data = unserialize($row->data);
    }
    if (isset($row->term_data)) {
      $row->term_data = unserialize($row->term_data);
    }
    $row->address_string = avclients_address_string_format($row);
    $rows[$row->id] = $row;
  }

  // Update static variables.
  $data[$data_id] = $rows;
  $_clients += $rows;

  return $rows;
}

/**
 * Delete client.
 * @param $id
 *
 * @return bool
 */
function avclients_client_delete($id) {
  if (!empty($id) && is_numeric($id)) {
    if (db_query("DELETE FROM {avtbl_clients} WHERE id = :client_id", array(':client_id' => $id))) {
      return TRUE;
    }
  }
  return FALSE;
}

/**
 * Change client status.
 * @param $id
 * @param $status
 * @param $where
 * @param $more_args
 *
 * @return bool
 */
function avclients_client_change_status($id, $status, $where = array(), $more_args = array()) {
  if (empty($id) || !is_numeric($id)) {
    return FALSE;
  }
  if (!array_key_exists($status, avclients_statuses())) {
    return FALSE;
  }

  $query = "UPDATE {avtbl_clients} SET status = :status WHERE id = :client_id";
  $args = array(':client_id' => $id, ':status' => $status);
  if (!empty($where)) {
    $query .= ' AND ' . implode(' AND ', $where);
  }
  $args = array_merge($args, $more_args);
  if (db_query($query, $args)) {
    avbase_custom_watchdog("client__status_change", "ID: {$id}", array('#status' => $status));
    return TRUE;
  }
  return FALSE;
}

/**
 * Return list of client statuses.
 */
function avclients_statuses() {
  return array(
    AVCLIENT_STATUS_DISABLED => 'Disabled',
    AVCLIENT_STATUS_ACTIVE => 'Active',
    AVCLIENT_STATUS_SUSPENDED => 'Suspended',
  );
}

/**
 * Menu callback - retrieve a JSON object containing autocomplete suggestions for existing clients.
 * @param $client_type_string
 * @param string $string
 */
function avclients_autocomplete($client_type_string, $status_string, $string = '') {
  $matches = array();
  $string = trim($string);
  if ($string) {
    $where = array();
    $args = array();
    $where_or = array();
    $string_arr = explode(' ', $string);
    $like_where = array();
    foreach ($string_arr as $k => $s) {
      $like_where[] = 'avclients.display_name LIKE :string_' . $k;
      $args[':string_' . $k] = '%' . db_like($s) . '%';
    }
    $where_or[] = '(' . implode(' AND ', $like_where) . ')';
    $where_or_str = '(' . implode(' OR ', $where_or) . ')';
    $where[] = $where_or_str;

    $client_types = explode('-', $client_type_string);
    $client_types = array_filter($client_types);
    if (!empty($client_types)) {
      $where[] = 'avclients.client_type IN(:client_types)';
      $args[':client_types'] = $client_types;
    }
    $statuses = explode('-', $status_string);
    $statuses = array_filter($statuses);
    if (!empty($statuses)) {
      $where[] = "avclients.status IN(:statuses)";
      $args[':statuses'] = $statuses;
    }

    // Build query.
    //$query = "SELECT
    //  avclients.*, avcat.title as agent_name,
    //  avcat_terms.title as term_name,
    //  avcat_terms.data as term_data
    //  FROM {avtbl_clients} avclients
    //  LEFT JOIN {avtbl_categories} avcat ON avcat.id = avclients.agent_id
    //  LEFT JOIN {avtbl_categories} avcat_terms ON avcat_terms.id = avclients.term_id
    //  WHERE " . implode(' AND ', $where) . " ORDER BY display_name LIMIT 8";
    //$rs = db_query($query, $args);
    $order_by = array();
    $order_by [] = 'CASE WHEN display_name LIKE :like_entire_string THEN 1 WHEN display_name = :entire_string THEN 2 ELSE 3 END';
    $args[':like_entire_string'] = '%' . db_like($string) . '%';
    $args[':entire_string'] = $string;
    $order_by[] = 'display_name';
    $rs = avclient_query_load($where, $args, 8, $order_by);
    $ordered_rows = avbase_order_db_rows_by_best_match($rs, 'display_name', $string);
    foreach ($ordered_rows as $row) {
      $row->address_string = avclients_address_string_format($row);
      $status_html = '';
      $status_icon_html = '';
      if ($row->status != AVCLIENT_STATUS_ACTIVE) {
        $csd = avclients_client_status_details($row->status);
        $status_icon_html .= '<em><i class="uk-icon-exclamation-circle"></i></em> ';
        $status_html .= ' <em>' . strtoupper($csd['title']) . '</em>';
      }
      $row_html = $status_icon_html . check_plain($row->display_name) . $status_html . '<div class="uk-text-muted uk-text-small">' . $row->address_string .  '</div>';
      $row_html .= '<div style="display: none;" id="av-row-id">' . $row->id . '</div>';
      $row_html .= '<div style="display: none;" id="av-row-json">' . drupal_json_encode($row) . '</div>';
      $matches[$row->display_name] = $row_html;
    }


    //$where_or = array(
    //  '(MATCH (display_name) AGAINST (:string IN BOOLEAN MODE))',
    //  //'(display_name LIKE :like_string)',
    //);
    //$where_or_str = '(' . implode(' OR ', $where_or) . ')';
    //$where = array($where_or_str);
    //$args = array(':string' => $string);
    ////$args[':like_string'] = '%' . db_like($string) . '%';
    //// Filter client status.
    //$statuses = explode('-', $status_string);
    //$statuses = array_filter($statuses);
    //if (!empty($statuses)) {
    //  $where[] = "client_type IN(:statuses)";
    //  $args['statuses'] = $statuses;
    //}
    //// Filter client type.
    //$client_types = explode('-', $client_type_string);
    //$client_types = array_filter($client_types);
    //if (!empty($client_types)) {
    //  $where[] = "client_type IN(:client_types)";
    //  $args['client_types'] = $client_types;
    //}
    //// Build query.
    //$query = "SELECT * FROM avtbl_clients WHERE " . implode(' AND ', $where) . " LIMIT 8";
    //$rs = db_query($query, $args);
    //foreach ($rs as $row) {
    //  $row->address_string = avclients_address_string_format($row);
    //  $row_html = check_plain($row->display_name) . '<div class="uk-text-muted uk-text-small">' . $row->address_string .  '</div>';
    //  $row_html .= '<div style="display: none;" id="av-row-id">' . $row->id . '</div>';
    //  $row_html .= '<div style="display: none;" id="av-row-json">' . drupal_json_encode($row) . '</div>';
    //  $matches[$row->display_name] = $row_html;
    //}
  }

  drupal_json_output($matches);
}

/**
 * Return definition of client table fields.
 * @param $db_op
 * @return array
 */
function avclients_table_fields_to_save($db_op = 'insert') {
  $fields = array(
    'client_type',
    'created',
    'changed',
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
    'area_id' => array('type' => 'int'),
    'discount_type' => array('type' => 'int'),
    'discount_value' => array('type' => 'float'),
    'category_id' => array('type' => 'int'),
    'credit_limit' => array('type' => 'float'),
    'opening_balance' => array('type' => 'float'),
    'contact_number',
    'website',
    'uid',
    'notes',
  );

  if ($db_op == 'update') {
    unset($fields['created']);
    unset($fields['opening_balance']);
  }
  return $fields;
}

/**
 * Return formatted address string.
 * @param $client
 * @return string
 */
function avclients_address_string_format($client) {
  $city_province = array(
    'city' => empty($client->city) ? NULL : check_plain($client->city),
    'province' => empty($client->province) ? NULL : check_plain($client->province),
    'zip_code' => empty($client->zip_code) ? NULL : check_plain($client->zip_code),
  );
  $city_province = array_filter($city_province);
  $address_details = array(
    'address' => empty($client->address) ? NULL : check_plain($client->address),
    'city_province' => implode(', ', $city_province),
  );
  $address_details = array_filter($address_details);
  $address_string = implode("\n", $address_details);
  return $address_string;
}

/**
 * Client status details.
 * @param null $status
 *
 * @return null
 */
function avclients_client_status_details($status = NULL) {
  $ret = &drupal_static(__FUNCTION__, array());
  if (empty($ret)) {
    $ret[AVCLIENT_STATUS_ACTIVE] = array(
      'status' => AVCLIENT_STATUS_ACTIVE,
      'title' => 'Active',
    );
    $ret[AVCLIENT_STATUS_SUSPENDED] = array(
      'status' => AVCLIENT_STATUS_SUSPENDED,
      'title' => 'Suspended',
      'color_class' => 'uk-alert-danger',
      'badge_class' => 'uk-badge-danger',
    );
    $ret[AVCLIENT_STATUS_DISABLED] = array(
      'status' => AVCLIENT_STATUS_DISABLED,
      'title' => 'Disabled',
      'color_class' => 'uk-alert-mute',
      'badge_class' => 'uk-badge-void',
    );
  }

  if (isset($ret[$status])) {
    return $ret[$status];
  }
  elseif (is_null($status)) {
    return $ret;
  }
  return NULL;
}
