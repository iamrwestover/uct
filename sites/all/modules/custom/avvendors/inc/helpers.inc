<?php

/**
 * Load vendor details.
 *
 * @param $vendor_id
 * @return mixed
 */
function avvendor_load($vendor_id) {
  if (empty($vendor_id) || $vendor_id == 'add') {
    return FALSE;
  }

  $data = &drupal_static('avbase', array());
  $_vendors = &$data['vendors'];
  if (isset($_vendors[$vendor_id])) {
    return $_vendors[$vendor_id];
  }

  $where = array('avvendors.id = :vendor_id');
  $args = array(':vendor_id' => $vendor_id);
  $rows = avvendor_query_load($where, $args);
  $row = empty($rows[$vendor_id]) ? FALSE : $rows[$vendor_id];

  // Update static variable.
  $_vendors[$vendor_id] = $row;
  return $row;
}

/**
 * Load vendors by ids.
 * @param array $ids
 *
 * @return array
 */
function avvendor_load_ids($ids) {
  if (empty($ids)) {
    return FALSE;
  }

  $data = &drupal_static('avbase', array());
  $_vendors = &$data['vendors'];

  $return = array();
  $query_ids = array();
  foreach ($ids as $id) {
    if (!empty($id)) {
      if (isset($_vendors[$id])) {
        $return[$id] = $_vendors[$id];
      }
      else {
        $query_ids[] = $id;
      }
    }
  }

  if (!empty($query_ids)) {
    $where = array('avvendors.id IN(:vendor_ids)');
    $args = array(':vendor_ids' => $query_ids);
    $rows = avvendor_query_load($where, $args);
    if (!empty($rows)) {
      $return += $rows;
    }
  }

  return $return;
}

/**
 * Advanced vendor load.
 * @param array $param_where
 * @param array $param_args
 * @param $limit_string
 *
 * @return array
 */
function avvendor_query_load($param_where, $param_args = array(), $limit_string = '') {
  if (empty($param_where)) {
    return array();
  }

  // Load vendors static variable.
  $avbase = &drupal_static('avbase', array());
  $avbase['vendors'] = empty($avbase['vendors']) ? array() : $avbase['vendors'];
  $_vendors = &$avbase['vendors'];

  // Set / load static variable for this query.
  $data = &drupal_static(__FUNCTION__, array());
  $_data_id = $param_where;
  sort($_data_id);
  $data_id = md5(json_encode($_data_id) . json_encode($param_args));

  if (isset($data[$data_id])) {
    return $data[$data_id];
  }

  //$query = "SELECT avv.*, avc.title AS agent_name
  //FROM {avtbl_vendors} avv
  //LEFT JOIN {avtbl_categories} avc ON avc.id = avv.agent_id AND avc.group_id = :group_id
  //WHERE avv.id = :vendor_id";
  //$args = array(':vendor_id' => $vendor_id, ':group_id' => 'agent_vend');
  //$row = db_query($query, $args)->fetchObject();
  //if (isset($row->data)) {
  //  $row->data = unserialize($row->data);
  //}
  //
  //$data[$vendor_id] = $row;
  //
  $query = "SELECT avvendors.*, avcat.title as agent_name
    FROM {avtbl_vendors} avvendors
    LEFT JOIN {avtbl_categories} avcat ON avcat.id = avvendors.agent_id AND avcat.group_id = 'agent_vend'";
  if (!empty($param_where)) {
    $query .= " WHERE " . implode(' AND ', $param_where);
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
    $rows[$row->id] = $row;
  }

  // Update static variables.
  $data[$data_id] = $rows;
  $_vendors += $rows;

  return $rows;
}


/**
 * Delete vendor.
 * @param $id
 *
 * @return bool
 */
function avvendors_vendor_delete($id) {
  if (!empty($id) && is_numeric($id)) {
    if (db_query("DELETE FROM {avtbl_vendors} WHERE id = :vendor_id", array(':vendor_id' => $id))) {
      return TRUE;
    }
  }
}

/**
 * Menu callback - retrieve a JSON object containing autocomplete suggestions for existing vendors.
 * @param string $string
 */
function avvendors_autocomplete($string = '') {
  $matches = array();
  $string = trim($string);
  if ($string) {
    $db_or = db_or();
    $db_or->condition('avv.display_name', '%' . db_like($string) . '%', 'LIKE');
    $db_or->condition('avv.company', '%' . db_like($string) . '%', 'LIKE');
    $db_or->condition('avv.first_name', '%' . db_like($string) . '%', 'LIKE');
    $db_or->condition('avv.last_name', '%' . db_like($string) . '%', 'LIKE');

    $query = db_select('avtbl_vendors', 'avv')->fields('avv', array('id', 'display_name', 'company', 'email', 'term_id', 'discount_type', 'discount_value'));
    $query->condition($db_or);
    $query->range(0, 15);
    $result = $query->execute();

    foreach ($result as $row) {
      $row->display_name = check_plain($row->display_name);
      $row_html = $row->display_name . '<span class="uk-text-muted"> / ' . check_plain($row->company) .  '</span>';
      $row_html .= '<div style="display: none;" id="av-row-id">' . $row->id . '</div>';
      $row_html .= '<div style="display: none;" id="av-row-json">' . drupal_json_encode($row) . '</div>';
      $matches[$row->display_name] = $row_html;
    }
  }

  drupal_json_output($matches);
}

/**
 * Return list of vendor categories.
 */
function avvendors_get_vendor_categories() {
  return array(
    AVVENDORS_CATEGORY_SUPPLIER => 'Supplier',
    AVVENDORS_CATEGORY_EXPENSES => 'Expenses',
  );
}

/**
 * Return definition of vendor table fields.
 * @param $db_op
 * @return array
 */
function avvendors_table_fields_to_save($db_op = 'insert') {
  $fields = array(
    'created',
    'first_name',
    'last_name',
    'middle_name',
    'company',
    'display_name',
    'agent_id' => array('type' => 'int'),
    'email',
    'address',
    'city',
    'province',
    'zip_code',
    'term_id' => array('type' => 'int'),
    'discount_type' => array('type' => 'int'),
    'discount_value' => array('type' => 'float'),
    'category_id' => array('type' => 'int'),
    'opening_balance' => array('type' => 'float'),
    'data' => array(
      'contact_number',
      'website',
    ),
  );

  if ($db_op == 'update') {
    unset($fields['created']);
    unset($fields['opening_balance']);
  }
  return $fields;
}
