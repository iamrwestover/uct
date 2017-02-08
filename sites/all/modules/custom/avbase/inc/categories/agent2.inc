<?php

/**
 * Implementation of hook_categories_(agent2)_form_alter().
 * @param $form
 * @param $db_row
 */
function avbase_category_agent2_form_alter(&$form, $db_row) {
  avbase_category_agent_form_alter($form, $db_row);
}

/**
 * Implementation of hook_categories_(agent2)_list_alter().
 * Alter Customer Agents List table.
 * @param $table_options
 * @param $db_rows
 */
function avbase_categories_agent2_list_alter(&$table_options, $db_rows) {
  $header = &$table_options['header'];
  $rows = &$table_options['rows'];
  $prefix = __FUNCTION__;

  // Remove Description header.
  unset($header['description']);
  // Rename title header.
  $header['title'] = t('Agent name');

  $column_contact_key = $prefix . 'contact_number';
  $header[$column_contact_key] = array(
    'data' => 'Contact',
    '#weight' => 1.1,
  );

  foreach ($rows as $k => &$row) {
    $db_row = isset($db_rows[$k]) ? $db_rows[$k] : array();
    $data = empty($db_row->data) ? array() : $db_row->data;

    // Remove description column.
    unset($row['data']['description']);

    // Add 'Contact' column.
    $row['data'][$column_contact_key] = array(
      'data' => empty($data['contact_number']) ? '' : $data['contact_number'],
    );
  }
}
