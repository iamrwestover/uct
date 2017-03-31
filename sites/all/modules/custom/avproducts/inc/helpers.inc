<?php

/**
 * Load product details by product_id.
 *
 * @param int $product_id
 * @return object
 */
function avproduct_load($product_id) {
  if (empty($product_id) || $product_id == 'add') {
    return FALSE;
  }

  $data = &drupal_static('avbase', array());
  $products = &$data['products'];
  if (isset($products[$product_id])) {
    return $products[$product_id];
  }

  $where = array('avprod.id = :product_id');
  $args = array(':product_id' => $product_id);
  $rows = avproduct_query_load($where, $args);
  $row = empty($rows[$product_id]) ? FALSE : $rows[$product_id];

  // Update static variable.
  $products[$product_id] = $row;
  return $row;
}

/**
 * Load products by ids.
 * @param array $ids
 *
 * @return array
 */
function avproduct_load_ids($ids) {
  if (empty($ids)) {
    return FALSE;
  }

  $data = &drupal_static('avbase', array());
  $products = &$data['products'];

  $return = array();
  $query_ids = array();
  foreach ($ids as $id) {
    if (!empty($id)) {
      if (isset($products[$id])) {
        $return[$id] = $products[$id];
      }
      else {
        $query_ids[] = $id;
      }
    }
  }

  if (!empty($query_ids)) {
    $where = array('avprod.id IN(:product_ids)');
    $args = array(':product_ids' => $query_ids);
    $rows = avproduct_query_load($where, $args);
    if (!empty($rows)) {
      $return += $rows;
    }
  }

  return $return;
}

/**
 * Advanced product load.
 * @param array $param_where
 * @param array $param_args
 * @param $limit_string
 *
 * @return array
 */
function avproduct_query_load($param_where, $param_args = array(), $limit_string = '') {
  if (empty($param_where)) {
    return array();
  }

  // Load products static variable.
  $avbase = &drupal_static('avbase', array());
  $avbase['products'] = empty($avbase['products']) ? array() : $avbase['products'];
  $products = &$avbase['products'];

  // Set / load static variable for this query.
  $data = &drupal_static(__FUNCTION__, array());
  $_data_id = $param_where;
  sort($_data_id);
  $data_id = md5(json_encode($_data_id) . json_encode($param_args));

  if (isset($data[$data_id])) {
    return $data[$data_id];
  }

  $query = "SELECT
    avprod.*,
    avclients.display_name AS vendor_name,
    avcat_uom.title AS uom_title
    FROM {avtbl_products} avprod
    LEFT JOIN {avtbl_clients} avclients ON avclients.id = avprod.vendor_id
    LEFT JOIN {avtbl_categories} avcat_uom ON avcat_uom.id = avprod.uom_id";
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

    $discount = array();
    $sales_discount = array();
    for ($x = 1; $x <= 4; $x++) {
      $_discount_name = "discount$x";
      if (isset($row->$_discount_name)) {
        $discount[] = round($row->$_discount_name, 2);
        $sales_discount[] = round($row->$_discount_name * AVTXN_DISCOUNT_MULTIPLIER, 2);
      }
    }
    $discount = array_filter($discount);
    $sales_discount = array_filter($sales_discount);
    $row->discount_text = implode('/', $discount);
    $row->sales_discount_text = implode('/', $sales_discount);

    // Capitalize product name.
    $row->title = strtoupper($row->title);
    $rows[$row->id] = $row;
  }

  // Update static variables.
  $data[$data_id] = $rows;
  $products += $rows;

  return $rows;
}

/**
 * Delete product.
 * @param $id
 *
 * @return bool
 */
function avproducts_product_delete($id) {
  if (!empty($id) && is_numeric($id)) {
    if (db_query("DELETE FROM {avtbl_products} WHERE id = :product_id", array(':product_id' => $id))) {
      return TRUE;
    }
  }
}

function element_validate_av_group_uom($element, &$form_state, $form) {
  $values = $form_state['values'];
  $base_uom_id = $values['uom_id'];
  if (empty($base_uom_id)) {
    form_set_error('uoms', 'Select a Base UOM first before adding new ones.');
    return;
  }

  $element_value = drupal_array_get_nested_value($values, $element['#parents']);
  if ($element_value == $base_uom_id) {
    form_error($element, 'UOM should be different from Base UOM.');
    return;
  }

  $dup_error_msg = 'Duplicate UOMs selected.';
  $errors = form_get_errors();
  //if (in_array($dup_error_msg, $errors))
  $uoms = $values['uoms'];
  foreach ($uoms as $k => $uom) {
    if ($k == $element['#parents'][1]) {
      continue;
    }

    if ($uom['uom_id'] == $element_value) {
      form_error($element, $dup_error_msg);
      return;
    }
  }
}

/**
 * Menu callback - retrieve a JSON object containing autocomplete suggestions for existing products.
 * @param string $string
 */
function avproducts_autocomplete($string = '') {
  $matches = array();
  $string = trim($string);
  if ($string) {
    $where = array();
    $args = array();
    $where_or = array(
      //'avprod.title LIKE :string',
      //'code = :exact_string',
    );
    $string_arr = explode(' ', $string);
    $like_where = array();
    foreach ($string_arr as $k => $s) {
      $like_where[] = 'title LIKE :string_' . $k;
      $args[':string_' . $k] = '%' . db_like($s) . '%';
    }
    $where_or[] = '(' . implode(' AND ', $like_where) . ')';
    $where_or_str = '(' . implode(' OR ', $where_or) . ')';
    $where[] = $where_or_str;
    //$args[':exact_string'] = $string;

    $where[] = 'status = :active_status';
    $args[':active_status'] = AVPRODUCTS_STATUS_ACTIVE;

    // Build query.
    $query = "SELECT * FROM avtbl_products WHERE " . implode(' AND ', $where) . " ORDER BY title LIMIT 8";
    $rs = db_query($query, $args);
    $ordered_rows = avbase_order_db_rows_by_best_match($rs, 'title', $string);

    foreach ($ordered_rows as $row) {
      $code = '';
      $title = check_plain($row->title);
      $row_html = $title . '<div class="uk-text-muted uk-text-break uk-text-small">' . $code .  '</div>' ;
      $row_html .= '<div style="display: none;" id="av-prod-id">' . $row->id . '</div>';
      $row_html .= '<div style="display: none;" id="av-prod-json">' . drupal_json_encode($row) . '</div>';
      $matches[$row->title] = $row_html;
    }

    //$string = explode(' ', $string);
    //foreach ($string as $k => $s) {
    //  $string[$k] = "-$s";
    //}
    //$string = implode(' ', $string);
  //  $where_or = array(
  //    '(MATCH (title) AGAINST (:string IN BOOLEAN MODE))',
  //    //'(code = :string)',
  //  );
  //  $where_or_str = '(' . implode(' OR ', $where_or) . ')';
  //  $where = array($where_or_str);
  //  $args = array(':string' => $string);
  //  //$args[':like_string'] = '%' . db_like($string) . '%';
  //  $where[] = 'status = :active_status';
  //  $args[':active_status'] = AVPRODUCTS_STATUS_ACTIVE;
  //
  //  // Build query.
  //  $query = "SELECT * FROM avtbl_products WHERE " . implode(' AND ', $where) . " LIMIT 8";
  //  $rs = db_query($query, $args);
  //  foreach ($rs as $row) {
  //    $code = check_plain($row->code);
  //    $title = check_plain($row->title);
  //    $row_html = $title . '<div class="uk-text-muted uk-text-break uk-text-small">' . $code .  '</div>' ;
  //    $row_html .= '<div style="display: none;" id="av-prod-id">' . $row->id . '</div>';
  //    $row_html .= '<div style="display: none;" id="av-prod-json">' . drupal_json_encode($row) . '</div>';
  //    $matches[$row->title] = $row_html;
  //  }
  }

  drupal_json_output($matches);
}

/**
 * Menu callback - retrieve a JSON object containing UOMs of specified product id.
 * @param $product_id
 * @param string $string
 */
function avproducts_product_uom_dropdown($product_id, $string = '') {
  $matches = array();
  $string = trim($string);
  //if ($string) {
  //  $query = db_select('avtbl_categories', 'avc')->fields('avc', array('title'));
  //  //$query->condition('avc.title', '%' . db_like($string) . '%', 'LIKE');
  //  $query->condition('avc.group_id', 'uom');
  //  $query->range(0, 10);
  //  $result = $query->execute();
  //
  //  foreach ($result as $row) {
  //    $matches[$row->title] = check_plain($row->title);
  //  }
  //}
$matches[$product_id] = $product_id.'ss';
  drupal_json_output($matches);
}

/**
 * Return definition of product table fields.
 * @param $db_op
 * @return array
 */
function avproducts_table_fields_to_save($db_op = 'insert') {
  $fields = array(
    'created',
    //'code',
    'title',
    'category_id' => array('type' => 'int'),
    'uom_id' => array('type' => 'int'),
    'rop' => array('type' => 'int'),
    'eoq' => array('type' => 'int'),
    'cost' => array('type' => 'float'),
    'price' => array('type' => 'float'),
    'expiry_date' => array('type' => 'int'),
    'lot_no',
    'vendor_id' => array('type' => 'int'),
    'account_type_id' => array('type' => 'int'),
    'shelf_id' => array('type' => 'int'),
    'principal_id' => array('type' => 'int'),
    'qty' => array('type' => 'int'),
    'initial_qty' => array('type' => 'int'),
    'initial_value' => array('type' => 'float'),
    'initial_qty_date',
    'data' => array(
      'uoms' => array('type' => 'array'),
    ),
    'uid',
    'changed',
  );

  if ($db_op == 'update') {
    unset($fields['created']);
    unset($fields['qty']);
    unset($fields['initial_qty']);
    unset($fields['initial_value']);
    unset($fields['initial_qty_date']);
  }
  return $fields;
}

/**
 * Product status details.
 * @param null $status
 *
 * @return null
 */
function avproducts_product_status_details($status = NULL) {
  $ret = &drupal_static(__FUNCTION__, array());
  if (empty($ret)) {
    $ret[AVCLIENT_STATUS_ACTIVE] = array(
      'status' => AVCLIENT_STATUS_ACTIVE,
      'title' => 'Active',
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

/**
 * Change product status.
 * @param $id
 * @param $status
 *
 * @return bool
 */
function avproducts_product_change_status($id, $status) {
  if (empty($id) || !is_numeric($id)) {
    return FALSE;
  }
  if (!array_key_exists($status, avproducts_product_status_details())) {
    return FALSE;
  }

  if (db_query("UPDATE {avtbl_products} SET status = :status WHERE id = :product_id", array(':product_id' => $id, ':status' => $status))) {
    avbase_custom_watchdog("product__status_change", "ID: {$id}", array('#status' => $status));
    return TRUE;
  }
  return FALSE;
}
