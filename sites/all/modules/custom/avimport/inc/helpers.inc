<?php

/**
 * Return valid headers.
 * @param $type
 *
 * @return array
 */
function avimport_get_valid_headers($type) {
  $required = array();
  $optional = array();
  switch ($type) {
    case 'products':
      // Required.
      $required['TITLE'] = 'title';
      $required['INITIAL QTY'] = 'initial_qty';
      $required['BASE UOM'] = 'uom_title';
      $required['ROP'] = 'rop';
      $required['EOQ'] = 'eoq';

      // Optional.
      $optional['CODE'] = 'code';
      $optional['COST'] = 'cost';
      $optional['SALES PRICE'] = 'price';
      $optional['CATEGORY'] = 'category_title';
      $optional['UOM 1'] = 'uom_1_title';
      $optional['UOM 1 QTY'] = 'uom_1_qty';
      $optional['UOM 2'] = 'uom_2_title';
      $optional['UOM 2 QTY'] = 'uom_2_qty';
      $optional['EXPIRY DATE'] = 'expiry_date';
      $optional['LOT NO'] = 'lot_no';
      $optional['SHELF LOCATION'] = 'shelf_title';
      $optional['PRINCIPAL'] = 'principal_title';
      break;
    case 'vendors':
      // Required.
      $required['VENDOR NAME'] = 'display_name';

      // Optional.
      $optional['COMPANY'] = 'company';
      $optional['AGENT NAME'] = 'agent_name';
      $optional['FIRST NAME'] = 'first_name';
      $optional['LAST NAME'] = 'last_name';
      $optional['EMAIL'] = 'email';
      $optional['CONTACT NUMBER'] = 'contact_number';
      $optional['CATEGORY'] = 'category';
      $optional['TERM'] = 'term';
      $optional['ADDRESS'] = 'address';
      $optional['CITY'] = 'city';
      $optional['PROVINCE'] = 'province';
      $optional['ZIP CODE'] = 'zip_code';
      $optional['OPENING BALANCE'] = 'opening_balance';
      break;

    case 'customers':
      // Required.
      $required['CUSTOMER NAME'] = 'display_name';

      // Optional.
      $optional['COMPANY'] = 'company';
      $optional['AGENT NAME'] = 'agent_name';
      $optional['FIRST NAME'] = 'first_name';
      $optional['LAST NAME'] = 'last_name';
      $optional['EMAIL'] = 'email';
      $optional['CONTACT NUMBER'] = 'contact_number';
      $optional['CATEGORY'] = 'category';
      $optional['TERM'] = 'term';
      $optional['AREA'] = 'area';
      $optional['ADDRESS'] = 'address';
      $optional['CITY'] = 'city';
      $optional['PROVINCE'] = 'province';
      $optional['ZIP CODE'] = 'zip_code';
      $optional['PAYMENT METHOD'] = 'payment_method';
      $optional['STATUS'] = 'status';
      $optional['CREDIT LIMIT'] = 'credit_limit';
      $optional['OPENING BALANCE'] = 'opening_balance';
      break;
  }

  $headers = array('required' => $required, 'optional' => $optional);
  return $headers;
}

/**
 * Insert rows into db.
 * @param $results
 * @param $row
 * @param $type
 */
function avimport_db_insert(&$results, $row, $type) {
  $results['insert_errors'] = empty($results['insert_errors']) ? array() : $results['insert_errors'];
  $_insert_errors = &$results['insert_errors'];
  $results['row_num'] = empty($results['row_num']) ? 1 : $results['row_num'];
  $row_num = ++$results['row_num'];

  $valid_headers = avimport_get_valid_headers($type);
  $required_headers = empty($valid_headers['required']) ? array() : $valid_headers['required'];
  $optional_headers = empty($valid_headers['optional']) ? array() : $valid_headers['optional'];
  if (empty($required_headers)) {
    $_insert_errors[$row_num] = t('Error importing data because required headers are not set.');
    return;
  }

  $title_key = array_search('title', $required_headers);
  $title_key = empty($title_key) ? array_search('display_name', $required_headers) : $title_key;
  $title = $row[$title_key];

  foreach ($required_headers as $k => $v) {
    if (!isset($row[$k]) || trim($row[$k]) == '') {
      $_insert_errors[$row_num] = t('Error importing data on row number %row_num (%title) because of invalid %header.', array('%title' => $title, '%row_num' => $row_num, '%header' => $k));
      return;
    }
  }

  $function_name = 'avimport_db_insert_' . $type . '_row';
  if (function_exists($function_name)) {
    $result = $function_name($row);
    if ($result === TRUE) {
      $results['inserted'][] = $row;
    }
    else {

      $_insert_errors[$row_num] = t('Error while importing data on row %row_num (%title) into database. %result_err', array('%title' => $title,'%row_num' => $row_num, '%result_err' => $result));
    }
  }
}

/**
 * Insert an excel row into products table.
 * @param $excel_row
 * @return bool
 */
function avimport_db_insert_products_row($excel_row) {
  // Build insert row.
  $insert_row = avimport_excel_row_to_db_row($excel_row, 'products');

  // Check if product title already exists.
  if (db_query('SELECT id FROM {avtbl_products} WHERE title = :title OR code = :code', array(':title' => $insert_row['title'], ':code' => $insert_row['code']))->fetchField()) {
    return t('Product with same title and/or code already exists.');
  }

  // Add base uom_id.
  $title = empty($excel_row['BASE UOM']) ? '' : trim($excel_row['BASE UOM']);
  if (!empty($title)) {
    $insert_row['uom_id'] = avbase_get_category_id_by_title($title, 'uom', TRUE);
  }
  $title = empty($excel_row['CATEGORY']) ? '' : trim($excel_row['CATEGORY']);
  if (!empty($title)) {
    $insert_row['category_id'] = avbase_get_category_id_by_title($title, 'product_cat', TRUE);
  }
  $title = empty($excel_row['SHELF LOCATION']) ? '' : trim($excel_row['SHELF LOCATION']);
  if (!empty($title)) {
    $insert_row['shelf_id'] = avbase_get_category_id_by_title($title, 'shelf', TRUE);
  }
  $title = empty($excel_row['PRINCIPAL']) ? '' : trim($excel_row['PRINCIPAL']);
  if (!empty($title)) {
    $insert_row['principal_id'] = avbase_get_category_id_by_title($title, 'principal', TRUE);
  }
  $title = empty($excel_row['UOM 1']) ? '' : trim($excel_row['UOM 1']);
  if (!empty($title)) {
    $uom_id = avbase_get_category_id_by_title($title, 'uom', TRUE);
    if (!empty($uom_id)) {
      $insert_row['data']['uoms'][$uom_id] = array(
        'uom_id' => avbase_get_category_id_by_title($title, 'uom', TRUE),
        'qty' => ((int)$excel_row['UOM 1 QTY']) ? (int)$excel_row['UOM 1 QTY'] : 1,
      );
    }
  }
  $title = empty($excel_row['UOM 2']) ? '' : trim($excel_row['UOM 2']);
  if (!empty($title)) {
    $uom_id = avbase_get_category_id_by_title($title, 'uom', TRUE);
    if (!empty($uom_id)) {
      $insert_row['data']['uoms'][$uom_id] = array(
        'uom_id' => avbase_get_category_id_by_title($title, 'uom', TRUE),
        'qty' => ((int)$excel_row['UOM 2 QTY']) ? (int)$excel_row['UOM 2 QTY'] : 1,
      );
    }
  }

  // Add created and initial qty date timestamp.
  $insert_row['created'] = $insert_row['initial_qty_date'] = time();
  if (!empty($insert_row['expiry_date'])) {
    $insert_row['expiry_date'] = strtotime($insert_row['expiry_date']);
  }
  if (!empty($insert_row['data'])) {
    $insert_row['data'] = serialize($insert_row['data']);
  }
  $insert_row['cost'] = (float) str_replace(',', '', $insert_row['cost']);
  $insert_row['price'] = (float) str_replace(',', '', $insert_row['price']);
  $insert_row['rop'] = (int) str_replace(',', '', $insert_row['rop']);
  $insert_row['eoq'] = (int) str_replace(',', '', $insert_row['eoq']);
  $insert_row['initial_qty'] = (int) str_replace(',', '', $insert_row['initial_qty']);
  $insert_row['qty'] = $insert_row['initial_qty'];

  return avimport_db_insert_row($insert_row, 'products');
}

/**
 * Insert an excel row into vendors table.
 * @param $excel_row
 * @return bool
 */
function avimport_db_insert_vendors_row($excel_row) {
  // Build insert row.
  $insert_row = avimport_excel_row_to_db_row($excel_row, 'vendors');

  // Check if display_name already exists.
  if (db_query('SELECT display_name FROM {avtbl_vendors} WHERE display_name = :display_name', array(':display_name' => $insert_row['display_name']))->fetchField()) {
    return t('Vendor with same name already exists.');
  }

  $title = empty($excel_row['AGENT NAME']) ? '' : trim($excel_row['AGENT NAME']);
  if (!empty($title)) {
    $insert_row['agent_id'] = avbase_get_category_id_by_title($title, 'agent_vend', TRUE);
  }
  $title = empty($excel_row['TERM']) ? '' : trim($excel_row['TERM']);
  if (!empty($title)) {
    $insert_row['term_id'] = avbase_get_category_id_by_title($title, 'payment_term', TRUE);
  }
  $title = empty($excel_row['CATEGORY']) ? '' : trim($excel_row['CATEGORY']);
  if (!empty($title)) {
    $insert_row['category_id'] = avbase_get_category_id_by_title($title, 'vendor_cat', TRUE);
  }

  $insert_row['opening_balance'] = (float) str_replace(',', '', $insert_row['opening_balance']);

  $title = empty($excel_row['CONTACT NUMBER']) ? '' : trim($excel_row['CONTACT NUMBER']);
  if (!empty($title)) {
    $insert_row['data']['contact_number'] = $title;
  }
  if (!empty($insert_row['data'])) {
    $insert_row['data'] = serialize($insert_row['data']);
  }


  // Add created and initial qty date timestamp.
  $insert_row['created'] = time();
  return avimport_db_insert_row($insert_row, 'vendors');
}

/**
 * Insert an excel row into customers table.
 * @param $excel_row
 * @return bool
 */
function avimport_db_insert_customers_row($excel_row) {
  // Build insert row.
  $insert_row = avimport_excel_row_to_db_row($excel_row, 'customers');

  // Check if customer title already exists.
  if (db_query('SELECT id FROM {avtbl_customers} WHERE display_name = :display_name', array(':display_name' => $insert_row['display_name']))->fetchField()) {
    return t('Customer with same name already exists.');
  }

  $title = empty($excel_row['AGENT NAME']) ? '' : trim($excel_row['AGENT NAME']);
  if (!empty($title)) {
    $insert_row['agent_id'] = avbase_get_category_id_by_title($title, 'agent_cust', TRUE);
  }
  $title = empty($excel_row['TERM']) ? '' : trim($excel_row['TERM']);
  if (!empty($title)) {
    $insert_row['term_id'] = avbase_get_category_id_by_title($title, 'payment_term', TRUE);
  }
  $title = empty($excel_row['CATEGORY']) ? '' : trim($excel_row['CATEGORY']);
  if (!empty($title)) {
    $insert_row['category_id'] = avbase_get_category_id_by_title($title, 'customer_cat', TRUE);
  }
  $title = empty($excel_row['PAYMENT METHOD']) ? '' : trim($excel_row['PAYMENT METHOD']);
  if (!empty($title)) {
    $insert_row['payment_method_id'] = avbase_get_category_id_by_title($title, 'payment_method', TRUE);
  }
  $title = empty($excel_row['AREA']) ? '' : trim($excel_row['AREA']);
  if (!empty($title)) {
    $insert_row['area_id'] = avbase_get_category_id_by_title($title, 'customer_area', TRUE);
  }

  $insert_row['opening_balance'] = (float) str_replace(',', '', $insert_row['opening_balance']);
  $insert_row['credit_limit'] = (float) str_replace(',', '', $insert_row['credit_limit']);

  $title = empty($excel_row['CONTACT NUMBER']) ? '' : trim($excel_row['CONTACT NUMBER']);
  if (!empty($title)) {
    $insert_row['data']['phone1'] = $title;
  }
  if (!empty($insert_row['data'])) {
    $insert_row['data'] = serialize($insert_row['data']);
  }


  // Add created and initial qty date timestamp.
  $insert_row['created'] = time();

  return avimport_db_insert_row($insert_row, 'customers');
}

/**
 * Convert excel row data into format readable by db_insert().
 * @param $excel_row
 * @param $type
 *
 * @return array
 */
function avimport_excel_row_to_db_row($excel_row, $type) {
  $valid_headers = avimport_get_valid_headers($type);
  $required_headers = empty($valid_headers['required']) ? array() : $valid_headers['required'];
  $optional_headers = empty($valid_headers['optional']) ? array() : $valid_headers['optional'];
  $headers = $required_headers + $optional_headers;

  $function_name = 'av' . $type . '_table_fields_to_save';
  $save_field_ids = $function_name();
  $insert_row = array();
  foreach ($save_field_ids as $k => $field) {
    $field_type = 'string';
    $field_id = $field;
    if (is_array($field)) {
      $field_id = $k;
      $field_type = isset($field['type']) ? $field['type'] : $field_type;
    }

    $excel_row_key = array_search($field_id, $headers);
    if (empty($excel_row_key)) {
      // No matching table field id from $headers.
      continue;
    }

    if (!isset($excel_row[$excel_row_key])) {
      continue;
    }

    settype($excel_row[$excel_row_key], $field_type);
    $insert_row[$field_id] = $field_type == 'string' ? trim($excel_row[$excel_row_key]) : $excel_row[$excel_row_key];
  }

  return $insert_row;
}

/**
 * Do insert row to their corresponding database table.
 * @param $insert_row
 * @param $type
 *
 * @return bool
 */
function avimport_db_insert_row($insert_row, $type) {
  try {
    if (db_insert('avtbl_' . $type)->fields($insert_row)->execute()) {
      return TRUE;
    }
  }
  catch (Exception $e) {
    $error_message = empty($e->errorInfo[2]) ? FALSE : $e->errorInfo[2];
    return $error_message;
  }
}