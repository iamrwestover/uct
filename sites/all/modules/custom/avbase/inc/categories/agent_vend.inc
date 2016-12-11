<?php

/**
 * Implementation of hook_categories_(agent_vend)_list_alter().
 * Alter Vendor Agents List table.
 * @param $table_options
 * @param $db_rows
 */
function avbase_categories_agent_vend_list_alter(&$table_options, $db_rows) {
  $header = &$table_options['header'];
  $rows = &$table_options['rows'];
  $prefix = __FUNCTION__;

  // Remove Description header.
  unset($header['description']);
  // Rename title header.
  $header['title'] = t('Agent name');

  $column_vendor_key = $prefix . '_vendor';

  //$header[$column_vendor_key] = array(
  //  'data' => 'Vendor',
  //  '#weight' => 1.1,
  //);

  foreach ($rows as $k => &$row) {
    $db_row = isset($db_rows[$k]) ? $db_rows[$k] : array();
    $data = empty($db_row->data) ? array() : $db_row->data;

    // Remove description column.
    unset($row['data']['description']);

    // Add 'Vendor' column.
    //$row['data'][$column_vendor_key] = array(
    //  'data' => empty($db_row->vendor_name) ? '' : check_plain($db_row->vendor_name),
    //);
  }
}

/**
 * Implementation of hook_categories_(agent_vend)_form_alter().
 * @param $form
 * @param $db_row
 */
function avbase_category_agent_vend_form_alter(&$form, $db_row) {
  avbase_category_agent_form_alter($form, $db_row);
}

/**
 * Perform actions depending on the kind of change that will be made
 * on the database for Vendor categories.
 * @param $op
 * @param $row
 * @param $form_state
 */
function avbase_avbase_crud_dbchange_agent_vend($op, $row, $form_state) {
  return;
  $values = $form_state['values'];
  //$table_name = isset($values['table_name']) ? $values['table_name'] : NULL;
  switch ($op) {
    case 'after-save':
      $agent_id = $row['id'];
      $vendor_id = empty($values['vendor_id']) ? NULL : $values['vendor_id'];
      // Make sure this agent is tied to a single vendor. Remove all other ties first.
      db_query('UPDATE {avtbl_vendors} SET agent_id = NULL WHERE agent_id = :agent_id', array(':agent_id' => $agent_id));
      // Tie to the selected agent.
      db_query('UPDATE {avtbl_vendors} SET agent_id = :agent_id WHERE id = :vendor_id', array(':agent_id' => $agent_id, ':vendor_id' => $vendor_id));
      break;
  }
}

/**
 * Implementation of hook_query_TAG_NAME_alter().
 * Alter query that loads agent vendors.
 * @param QueryAlterableInterface $query
 */
function avbase_query_avcategory_load_agent_vend_alter(QueryAlterableInterface $query) {
  return;
  $query->leftJoin('avtbl_vendors', 'avv', 'avv.agent_id = avc.id');
  $query->addField('avv', 'id', 'vendor_id');
  $query->addField('avv', 'display_name', 'vendor_name');
}
