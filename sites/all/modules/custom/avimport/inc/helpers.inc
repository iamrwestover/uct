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
      $required['COST'] = 'cost';
      $required['SALES PRICE'] = 'price';


      // Optional.
      $optional['ROP'] = 'rop';
      $optional['EOQ'] = 'eoq';
      //$optional['CODE'] = 'code';
      $optional['CATEGORY'] = 'category_title';
      $optional['UOM 1'] = 'uom_1_title';
      $optional['UOM 1 QTY'] = 'uom_1_qty';
      $optional['UOM 2'] = 'uom_2_title';
      $optional['UOM 2 QTY'] = 'uom_2_qty';
      $optional['UOM 3'] = 'uom_3_title';
      $optional['UOM 3 QTY'] = 'uom_3_qty';
      $optional['UOM 4'] = 'uom_4_title';
      $optional['UOM 4 QTY'] = 'uom_4_qty';
      $optional['UOM 5'] = 'uom_5_title';
      $optional['UOM 5 QTY'] = 'uom_5_qty';
      //$optional['EXPIRY DATE'] = 'expiry_date';
      //$optional['LOT NO'] = 'lot_no';
      $optional['SHELF LOCATION'] = 'shelf_title';
      $optional['DISCOUNT'] = 'discount';
      $optional['PRINCIPAL'] = 'principal_title';
      $optional['PREFERRED VENDOR'] = 'preferred_vendor';
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
      //$optional['OPENING BALANCE'] = 'opening_balance';
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
      //$optional['STATUS'] = 'status';
      $optional['CREDIT LIMIT'] = 'credit_limit';
      //$optional['OPENING BALANCE'] = 'opening_balance';
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
  global $user;

  // Build insert row.
  $insert_row = avimport_excel_row_to_db_row($excel_row, 'products');

  // Check if product title already exists.
  //$item_code = isset($insert_row['code']) ? $insert_row['code'] : '-~-';
  if (db_query('SELECT id FROM {avtbl_products} WHERE title = :title', array(':title' => $insert_row['title']/*, ':code' => $item_code*/))->fetchField()) {
    //return t('Product with same title and/or code already exists.');
    return t('Product with same title already exists.');
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

  $title = empty($excel_row['PREFERRED VENDOR']) ? '' : trim($excel_row['PREFERRED VENDOR']);
  if (!empty($title)) {
    $preferred_vendor_id = db_query('SELECT id FROM {avtbl_clients} WHERE display_name = :title', array(':title' => $title))->fetchField();
    $insert_row['vendor_id'] = empty($preferred_vendor_id) ? NULL : $preferred_vendor_id;
  }

  $title = empty($excel_row['DISCOUNT']) ? '' : trim($excel_row['DISCOUNT']);
  $title = str_replace('%', '', $title);
  if (!empty($title)) {
    // Explode discount text to discount array.
    $discount_text = $title;
    //$discount_text = is_numeric($discount_text) ? ($discount_text * 100) : $discount_text;
    $discount_parts = explode('/', $discount_text);
    $x = 1;
    foreach ($discount_parts as $k => $v) {
      if (!is_numeric($v)) {
        continue;
      }
      $dc = (float) $v;
      if ($dc > 0) {
        $insert_row['discount' . $x++] = $dc;
      }
    }
  }

  //$test_uoms = array(
  //  114 => rand(2, 6), // link
  //  92 => rand(4, 6), // card
  //  1 => 20, // box
  //  13 => 10, // pack
  //  31 => 12, // dozen
  //  29 => 24, // case
  //);
  //
  ////array_splice($test_uoms, -1, rand(1, 3));
  //foreach ($test_uoms as $k => $v) {
  //  $insert_row['data']['uoms'][$k] = array(
  //    'uom_id' => $k,
  //    'qty' =>  $v,
  //  );
  //}


  for ($x = 1; $x <= 5; $x++) {
    $title = empty($excel_row['UOM ' . $x]) ? '' : trim($excel_row['UOM ' . $x]);
    if (!empty($title)) {
      $uom_id = avbase_get_category_id_by_title($title, 'uom', TRUE);
      $uom_qty = isset($excel_row['UOM ' . $x . ' QTY']) ? $excel_row['UOM ' . $x . ' QTY'] : NULL;
      if (!empty($uom_id)) {
        if (is_null($uom_qty) || !is_numeric($uom_qty)) {
          return t('UOM ' . $x . ' qty is invalid or not found.');
        }
        $insert_row['data']['uoms'][$uom_id] = array(
          //'uom_id' => avbase_get_category_id_by_title($title, 'uom', TRUE),
          'uom_id' => $uom_id,
          'qty' => $uom_qty,
        );
      }
    }
  }

  // Add created and initial qty date timestamp.
  $insert_row['created'] = $insert_row['initial_qty_date'] = $insert_row['changed'] = time();
  if (!empty($insert_row['expiry_date'])) {
    $insert_row['expiry_date'] = strtotime($insert_row['expiry_date']);
  }
  if (!empty($insert_row['data'])) {
    $insert_row['data'] = serialize($insert_row['data']);
  }
  $insert_row['rop'] = isset($insert_row['rop']) ? (int) str_replace(',', '', $insert_row['rop']) : NULL;
  $insert_row['eoq'] = isset($insert_row['eoq']) ? (int) str_replace(',', '', $insert_row['eoq']) : NULL;
  $insert_row['cost'] = (float) str_replace(',', '', $insert_row['cost']);
  $insert_row['price'] = (float) str_replace(',', '', $insert_row['price']);
  $insert_row['initial_qty'] = (int) str_replace(',', '', $insert_row['initial_qty']);
  $insert_row['qty'] = $insert_row['initial_qty'];
  $insert_row['initial_value'] = $insert_row['cost'] * $insert_row['initial_qty'];

  $insert_row['uid'] = $user->uid;
  return avimport_db_insert_row($insert_row, 'products');
}

/**
 * Insert an excel row into vendors table.
 * @param $excel_row
 * @return bool
 */
function avimport_db_insert_vendors_row($excel_row) {
  return avimport_db_insert_clients_row($excel_row, AVCLIENT_TYPE_VENDOR);

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
  return avimport_db_insert_clients_row($excel_row, AVCLIENT_TYPE_CUSTOMER);
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
 * Insert an excel row into clients table.
 * @param $excel_row
 * @param int $client_type
 * @return bool
 */
function avimport_db_insert_clients_row($excel_row, $client_type) {
  global $user;

  // Build insert row.
  $insert_row = avimport_excel_row_to_db_row($excel_row, 'clients', $client_type);
  $ctd = avclients_client_types($client_type);
  // Check if customer title already exists.
  if ($same_name_count = db_query('SELECT COUNT(id) FROM {avtbl_clients} WHERE display_name LIKE :display_name', array(':display_name' => db_like($insert_row['display_name']) . '%'))->fetchField()) {
    // Check if customer address also matches.
    $address = empty($excel_row['ADDRESS']) ? '' : trim($excel_row['ADDRESS']);
    $province = empty($excel_row['PROVINCE']) ? '' : trim($excel_row['PROVINCE']);
    $city = empty($excel_row['CITY']) ? '' : trim($excel_row['CITY']);
    $same_name_same_addr = db_query(
      'SELECT COUNT(id) FROM {avtbl_clients} WHERE display_name LIKE :display_name AND address = :addr AND province = :prov AND city = :city',
      array(
        ':display_name' => db_like($insert_row['display_name']) . '%',
        ':addr' => $address,
        ':prov' => $province,
        ':city' => $city,
      )
    )->fetchField();
    if ($same_name_same_addr) {
      return t('@type with same name and address already exists.', array('@type' => $ctd['name']));
    }
    else {
      $insert_row['display_name'] .= ' ' . ($same_name_count + 1);
    }
  }

  $title = empty($excel_row['AGENT NAME']) ? '' : trim($excel_row['AGENT NAME']);
  if (!empty($title)) {
    $insert_row['agent_id'] = avbase_get_category_id_by_title($title, 'agent' . $client_type, TRUE);
  }
  $title = empty($excel_row['TERM']) ? '' : trim($excel_row['TERM']);
  if (!empty($title)) {
    $insert_row['term_id'] = avbase_get_category_id_by_title($title, 'payment_term', TRUE);
  }
  $title = empty($excel_row['CATEGORY']) ? '' : trim($excel_row['CATEGORY']);
  if (!empty($title)) {
    $insert_row['category_id'] = avbase_get_category_id_by_title($title, 'category' . $client_type, TRUE);
  }
  $title = empty($excel_row['PAYMENT METHOD']) ? '' : trim($excel_row['PAYMENT METHOD']);
  if (!empty($title)) {
    $insert_row['payment_method_id'] = avbase_get_category_id_by_title($title, 'payment_method', TRUE);
  }
  $title = empty($excel_row['AREA']) ? '' : trim($excel_row['AREA']);
  if (!empty($title)) {
    $insert_row['area_id'] = avbase_get_category_id_by_title($title, 'area', TRUE);
  }

  $insert_row['opening_balance'] = isset($insert_row['opening_balance']) ? (float) str_replace(',', '', $insert_row['opening_balance']) : NULL;
  $insert_row['credit_limit'] = isset($insert_row['credit_limit']) ? (float) str_replace(',', '', $insert_row['credit_limit']) : NULL;

  // Add created and initial qty date timestamp.
  $insert_row['created'] = time();
  $insert_row['changed'] = $insert_row['created'];

  $insert_row['uid'] = $user->uid;
  return avimport_db_insert_row($insert_row, 'clients');
}

/**
 * Convert excel row data into format readable by db_insert().
 * @param $excel_row
 * @param $type
 *
 * @return array
 */
function avimport_excel_row_to_db_row($excel_row, $type, $client_type = NULL) {
  if ($type == 'clients') {
    $ctd = avclients_client_types($client_type);
    $valid_headers = avimport_get_valid_headers(strtolower($ctd['name']) . 's');
  }
  else {
    $valid_headers = avimport_get_valid_headers($type);
  }

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

  if ($type == 'clients') {
    $insert_row['client_type'] = $client_type;
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