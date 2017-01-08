<?php

/**
 * Implementation of hook_categories_(uom)_list_alter().
 * Alter UOM List table.
 * @param $table_options
 * @param $db_rows
 */
function avbase_categories_uom_list_alter(&$table_options, $db_rows) {
  $header = &$table_options['header'];
  $rows = &$table_options['rows'];
  $prefix = __FUNCTION__;

  $column_default_key = $prefix . '_default';

  $header[$column_default_key] = array(
    'data' => 'Default',
    '#weight' => 1.1,
  );

  foreach ($rows as $k => &$row) {
    $db_row = isset($db_rows[$k]) ? $db_rows[$k] : array();
    $data = empty($db_row->data) ? array() : $db_row->data;

    // Append plural form to UOM Title.
    $plural_form = empty($data['plural_form']) ? '' : strtolower(check_plain($data['plural_form']));
    if ($plural_form) {
      $row['data']['title']['data'] .= '<span class="uk-text-muted"> / ' . $plural_form . '</span>';
    }

    // Add 'Default UOM' column.
    $row['data'][$column_default_key] = array(
      'data' => empty($db_row->system_default) ? '' : 'Yes',
    );
  }
}

/**
 * Implementation of hook_categories_(uom)_form_alter().
 * @param $form
 * @param $db_row
 */
function avbase_category_uom_form_alter(&$form, $db_row) {
  $form['title']['#title'] = t('UOM title');
  $form['title']['#element_validate'][] = 'avbase_element_validate_category_title_unique';
  $form['system_default'] = array(
    '#type' => 'checkbox',
    '#title' => 'Use this UOM as system default',
    '#weight' => 1.2,
    '#default_value' => !empty($db_row->system_default),
  );
  $form['plural_form'] = array(
    '#type' => 'textfield',
    '#title' => 'Plural form',
    '#default_value' => isset($db_row->data['plural_form']) ? check_plain($db_row->data['plural_form']) : '',
    '#maxlength' => 32,
    '#weight' => 1.1,
  );

  //$form['data_ids']['#value'][] = 'default';
  $form['data_ids']['#value'][] = 'plural_form';
  $form['save_field_ids']['#value'][] = 'system_default';
  //array_unshift($form['#submit'], 'avbase_category_uom_form_submit');
}


/**
 * Perform actions depending on the kind of change that will be made
 * on the database for UOM categories.
 * @param $op
 * @param $row
 * @param $form_state
 */
function avbase_avbase_crud_dbchange_uom($op, $row, $form_state) {
  $values = $form_state['values'];
  $table_name = isset($values['table_name']) ? $values['table_name'] : NULL;
  switch ($op) {
    case 'pre-save':
      // Make sure this $row is the only system default UOM if necessary.
      if (!empty($row['system_default'])) {
        db_query("UPDATE {{$table_name}} SET system_default = 0 WHERE group_id = :group_id", array(':group_id' => $row['group_id']));
      }
      break;
  }
}