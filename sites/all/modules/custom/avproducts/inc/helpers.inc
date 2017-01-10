<?php

/**
 * Load product details.
 *
 * @param int|array $param_where
 *    Accepts an integer value and will be treated as product id.
 * @param $param_args
 * @return mixed
 */
function avproduct_load($param_where, $param_args = array()) {
  //var_dump($param_where);
  if (empty($param_where) || $param_where == 'add') {
    return FALSE;
  }

  $data = &drupal_static(__FUNCTION__, array());
  if (is_numeric($param_where)) {
    $param_args[':product_id'] = $param_where;
    $param_where = array('avprod.id = :product_id');
  }
  //dpm($param_where);
  //sort($param_where);
  //dpm($param_where);

  $data_id = md5(json_encode($param_where) . json_encode($param_args));
  if (isset($data[$data_id])) {
    return $data[$data_id];
  }

  $query = "SELECT avprod.*, avvendor.display_name AS vendor_name
    FROM {avtbl_products} avprod
    LEFT JOIN {avtbl_vendors} avvendor ON avvendor.id = avprod.vendor_id";
  if (!empty($param_where)) {
    $query .= " WHERE " . implode(' AND ', $param_where);
  }
  $row = db_query($query, $param_args)->fetchObject();
  if (isset($row->data)) {
    $row->data = unserialize($row->data);
  }

  $data[$data_id] = $row;
  return $row;
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
    $query->range(0, 15);
    $result = $query->execute();

    foreach ($result as $row) {
      $code = check_plain($row->code);
      $title = check_plain($row->title);
      $row->data = unserialize($row->data);
      //$uoms = empty($data['uoms']) ? array() : $data['uoms'];
      $row_html = $title . '<div class="uk-text-muted uk-text-break">' . $code .  '</div>' ;
      $row_html .= '<div style="display: none;" id="av-prod-id">' . $row->id . '</div>';
      $row_html .= '<div style="display: none;" id="av-prod-json">' . drupal_json_encode($row) . '</div>';
      $matches[$title] = $row_html;
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
