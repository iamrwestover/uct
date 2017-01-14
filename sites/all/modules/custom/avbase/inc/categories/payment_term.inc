<?php

/**
 * Implementation of hook_categories_(payment_term)_form_alter().
 * @param $form
 * @param $db_row
 */
function avbase_category_payment_term_form_alter(&$form, $db_row) {
  $form['title']['#title'] = 'Payment term name';
  $form['title']['#element_validate'][] = 'avbase_element_validate_category_title_unique';
  $form['description']['#access'] = FALSE;

  $form['days'] = array(
    '#type' => 'textfield',
    '#title' => 'Number of days',
    '#default_value' => isset($db_row->data['days']) ? check_plain($db_row->data['days']) : '',
    '#maxlength' => 3,
    '#required' => TRUE,
    '#element_validate' => array('element_validate_integer_positive'),
    '#special_element_type' => 'number',
    '#weight' => 1.1,
  );
  $form['discount_type'] = array(
    '#type' => 'select',
    '#title' => 'Default discount type',
    '#options' => avbase_get_discount_types(),
    '#default_value' => isset($db_row->data['discount_type']) ? $db_row->data['discount_type'] : 1,
    '#weight' => 1.2,
    '#required' => TRUE,
  );
  $form['discount_value'] = array(
    '#type' => 'textfield',
    '#title' => 'Default discount value',
    '#default_value' => isset($db_row->data['discount_value']) ? check_plain($db_row->data['discount_value']) : '',
    '#maxlength' => 19,
    '#element_validate' => array('element_validate_number', 'avbase_element_validate_money_length'),
    '#special_element_type' => 'number',
    '#weight' => 1.3,
    '#attributes' => array(
      //'class' => array(''),
      'step' => 0.01,
    ),
  );

  // Add to data fields that will be saved to database.
  $form['data_ids']['#value'][] = 'days';
  $form['data_ids']['#value'][] = 'discount_type';
  $form['data_ids']['#value'][] = 'discount_value';
}

/**
 * Implementation of hook_categories_(payment_term)_list_alter().
 * Alter UOM List table.
 * @param $table_options
 * @param $db_rows
 */
function avbase_categories_payment_term_list_alter(&$table_options, $db_rows) {
  $header = &$table_options['header'];
  $rows = &$table_options['rows'];
  $prefix = __FUNCTION__ . '_';

  $column_days_key = $prefix . 'days';
  $column_discount_key = $prefix . 'discount';

  // Add extra headers.
  $header[$column_days_key] = array(
    'data' => 'Days',
    '#weight' => 1.1,
  );
  $header[$column_discount_key] = array(
    'data' => 'Discount',
    '#weight' => 1.2,
  );

  // Rename title header.
  $header['title'] = t('Payment term name');
  // Remove Description header.
  unset($header['description']);

  foreach ($rows as $k => &$row) {
    $db_row = isset($db_rows[$k]) ? $db_rows[$k] : array();
    $data = empty($db_row->data) ? array() : $db_row->data;

    $days = isset($data['days']) ? check_plain($data['days']) : -1;
    $row['#weight'] = $days;

    // Add 'Days' column.
    $row['data'][$column_days_key] = array(
      'data' => $days > 0 ? $days : '',
    );

    // Add 'Discount' column.
    $discount = isset($data['discount_value']) ? check_plain($data['discount_value']) : '';
    if ($discount) {
      $discount_type = isset($data['discount_type']) ? $data['discount_type'] : 0;
      $discount = number_format($discount, 2);
      //$discount = $discount_type == 1 ? round($discount, 2) : ' / <span class="uk-text-muted">fixed amount</span>';
      //$discount .= $discount_type == 1 ? '%' : ' / <span class="uk-text-muted">fixed amount</span>';
      $discount .= $discount_type == 1 ? '%' : ' / <span class="uk-text-muted">fixed amount</span>';
    }
    $row['data'][$column_discount_key] = array(
      'data' => $discount,
    );

    // Remove description column.
    unset($row['data']['description']);
  }

  // Sort by days.
  uasort($rows, 'element_sort');
}
