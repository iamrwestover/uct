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
 *
 * @return array
 */
function avproduct_query_load($param_where, $param_args = array()) {
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
    avvendor.display_name AS vendor_name,
    avcat_uom.title AS uom_title
    FROM {avtbl_products} avprod
    LEFT JOIN {avtbl_vendors} avvendor ON avvendor.id = avprod.vendor_id
    LEFT JOIN {avtbl_categories} avcat_uom ON avcat_uom.id = avprod.uom_id";
  if (!empty($param_where)) {
    $query .= " WHERE " . implode(' AND ', $param_where);
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
    $db_or = db_or();
    $db_or->condition('avprod.title', '%' . db_like($string) . '%', 'LIKE');
    $db_or->condition('avprod.code', '%' . db_like($string) . '%', 'LIKE');

    $query = db_select('avtbl_products', 'avprod')->fields('avprod', array('id', 'title', 'code', 'uom_id', 'cost', 'price', 'data'));
    $query->condition($db_or);
    $query->range(0, 10);
    $result = $query->execute();

    foreach ($result as $row) {
      $code = check_plain($row->code);
      $title = check_plain($row->title);
      $row->data = unserialize($row->data);
      //$uoms = empty($data['uoms']) ? array() : $data['uoms'];
      $row_html = $title . '<div class="uk-text-muted uk-text-break">' . $code .  '</div>' ;
      $row_html .= '<div style="display: none;" id="av-prod-id">' . $row->id . '</div>';
      $row_html .= '<div style="display: none;" id="av-prod-json">' . drupal_json_encode($row) . '</div>';
      $matches[$row->title] = $row_html;
    }
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
    'code',
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
    'initial_qty_date',
    'data' => array(
      'uoms' => array('type' => 'array'),
    ),
  );

  if ($db_op == 'update') {
    unset($fields['created']);
    unset($fields['qty']);
    unset($fields['initial_qty']);
    unset($fields['initial_qty_date']);
  }
  return $fields;
}
