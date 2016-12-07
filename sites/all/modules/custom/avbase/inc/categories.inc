<?php

/**
 * Categories home page.
 * @return string
 */
function avbase_categories_home() {
  return 'Categories Maintenance';
}

/**
 * List of categories.
 * @param $form
 * @param $form_state
 * @param $category_id
 *
 * @return mixed
 */
function avbase_categories_form($form, &$form_state, $category_id) {
  $form['group_id'] = array('#type' => 'value', '#value' => $category_id);
  $category = avbase_category_groups($category_id);
  $category_menu_path = $category['menu_path'];
  $btn_options = array('attributes' => array('class' => AV_STYLE_CLASS_BTN . ' ' . AV_STYLE_CLASS_BTN_PRIMARY));
  $form['new_button'] = array('#markup' => l('New ' . strtolower($category['name']), $category_menu_path . '/add', $btn_options));

  $header = array(
    'id' => array('data' => 'ID', 'field' => 'id', 'sort' => 'asc', 'class' => array('uk-hidden'), '#weight' => 0),
    'title' => array('data' => 'Title', 'field' => 'title', '#weight' => 1),
    'description' => array('data' => 'Description', 'field' => 'description', '#weight' => 2),
    'actions' => array('data' => '&nbsp;', '#weight' => 3),
  );

  $result = db_select('avtbl_categories', 'acg')
    ->fields('acg')
    ->condition('group_id', $category_id)
    ->extend('PagerDefault')
    ->extend('TableSort')
    ->limit(10)
    ->orderByHeader($header)
    ->execute();

  //$action_links_all = array();
  //$categories = avbase_get_categories_multiple(array('product_cat', 'uom'));
  $table_rows = array();
  $db_rows = array();
  $destination_path = drupal_get_destination();
  foreach ($result as $db_row) {
    $db_row->data = unserialize($db_row->data);
    $db_rows[] = $db_row;

    $action_links = array('links' => array(
      'edit' => array('title' => 'edit', 'href' => $category_menu_path . '/' . $db_row->id . '/edit', 'query' => $destination_path),
      'delete' => array('title' => 'delete', 'href' => $category_menu_path . '/' . $db_row->id . '/delete', 'query' => $destination_path),
    ));

    //$action_links_all[] = $action_links;
    $actions_html = theme('links__bare', $action_links);
    $table_rows[] = array(
      'data' => array(
        'id' => array('data' => $db_row->id, 'class' => array('uk-hidden')),
        'title' => array('data' => check_plain($db_row->title)),
        'description' => array('data' => check_plain($db_row->description)),
        'actions' => array('data' => $actions_html),
      ),
      'class' => array('uk-table-middle'),
    );
  }

  $table_attributes = array('id' => 'categories-list-table');

  // Let other modules alter table data.
  $table_options = array(
    'header' => $header,
    'rows' => $table_rows,
    'attributes' => $table_attributes,
  );

  drupal_alter('categories_' . $category_id . '_list', $table_options, $db_rows);

  // Sort headers and row data.
  uasort($table_options['header'], 'element_sort');
  foreach ($table_options['rows'] as $k => &$row) {
    foreach ($row['data'] as $row_key => &$row_data) {
      if (isset($table_options['header'][$row_key]['#weight'])) {
        $row_data['#weight'] = $table_options['header'][$row_key]['#weight'];
      }
    }
    uasort($row['data'], 'element_sort');
  }

  if (empty($table_rows)) {
    $table_markup = '<p>Nothing recorded yet.</p>';
  }
  else {
    $table_markup = theme('table', $table_options);
  }
  $form['table']['#markup'] = $table_markup;
  $form['pager']['#markup'] = theme('pager', array('tags' => array()));

  return $form;
}

/**
 * Load category form.
 * @param object $row
 * @param string $category_id
 *
 * @return string
 */
function avbase_category_form_load($category_id, $row) {
  return drupal_get_form('avbase_category_form', $category_id, $row);
}

/**
 * Category add/edit form.
 * @param $form
 * @param $form_state
 * @param $category_id
 * @param $row
 *
 * @return mixed
 */
function avbase_category_form($form, &$form_state, $category_id, $row) {
  $category = avbase_category_groups($category_id);
  $form['id'] = array('#type' => 'value', '#value' => isset($row->id) ? $row->id : NULL);

  $page_title = empty($form['id']['#value']) ? 'New ' : 'Edit ';
  $page_title .= strtolower($category['name']);
  drupal_set_title($page_title);

  // Set crud settings.
  avbase_category_form_settings($form, $category_id);

  $form['group_id'] = array('#type' => 'value', '#value' => $category_id);
  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => 'Title',
    '#default_value' => isset($row->title) ? check_plain($row->title) : '',
    '#maxlength' => 32,
    '#required' => TRUE,
    '#weight' => 1,
  );
  $form['description'] = array(
    '#type' => 'textarea',
    '#title' => 'Description',
    '#default_value' => isset($row->description) ? check_plain($row->description) : '',
    '#maxlength' => 255,
    '#weight' => 2,
  );

  $form['buttons']['#weight'] = 3;
  $form['buttons']['#prefix'] = '<div class="uk-margin-top"></div>';
  $form['buttons']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  $form['buttons']['cancel'] = array(
    '#markup' => l('Cancel', $category['menu_path']),
  );

  drupal_alter('category_' . $category_id . '_form', $form, $row);
  return $form;
}

/**
 * Set crud form settings.
 * @param $form
 * @param $category_id
 */
function avbase_category_form_settings(&$form, $category_id) {
  $table_name = 'avtbl_categories';
  $entity_name = $category_id;

  // Set generic submit handler.
  $form['#submit'] = array('avbase_crud_form_submit');
  $form['table_name'] = array('#type' => 'value', '#value' => $table_name);
  $form['entity_name'] = array('#type' => 'value', '#value' => $entity_name);

  // Set field names to save.
  $save_field_ids = array(
    'title',
    'description',
    'group_id',
  );
  $form['save_field_ids'] = array(
    '#type' => 'value',
    '#value' => $save_field_ids,
  );
  // Set which fields belong to data field.
  $data_ids = array();
  $form['data_ids'] = array(
    '#type' => 'value',
    '#value' => $data_ids,
  );
}

/**
 * Menu callback: confirm category delete.
 * @param $form
 * @param $form_state
 * @param object $row
 *
 * @return array
 */
function avbase_category_delete_confirm_form($form, &$form_state, $category_id, $row) {
  $category = avbase_category_groups($category_id);
  $prompt = 'Are you sure you want to delete this ' . $category['name'] . '?';
  $display_name = check_plain($row->title);
  $desc = check_plain($row->description);
  $description = <<<HTML
  <h1 class="uk-article-title">$display_name</h1>$desc
  <hr />
  <p>This action cannot be undone.</p>
HTML;
  $form_state['row'] = $row;
  return confirm_form(array(), $prompt, '', $description);
}

/**
 * Handler for category delete.
 */
function avbase_category_delete_confirm_form_submit($form, &$form_state) {
  if (isset($form_state['row']->id)) {
    if (avbase_category_delete($form_state['row']->id)) {
      drupal_set_message('Record deleted.');
    }
  }
  $form_state['redirect'] = '';
}

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
 * @param $table_name
 */
function avbase_avbase_crud_dbchange_uom($op, $row, $table_name) {
  switch ($op) {
    case 'pre-save':
      // Make sure this $row is the only system default UOM if necessary.
      if (!empty($row['system_default'])) {
        db_query("UPDATE {{$table_name}} SET system_default = 0 WHERE group_id = :group_id", array(':group_id' => $row['group_id']));
      }
      break;
  }
}

/**
 * Implementation of hook_categories_(agent_cust)_form_alter().
 * @param $form
 * @param $db_row
 */
function avbase_category_agent_cust_form_alter(&$form, $db_row) {
  avbase_category_agent_form_alter($form, $db_row);
}

/**
 * Implementation of hook_categories_(agent_vend)_form_alter().
 * @param $form
 * @param $db_row
 */
function avbase_category_agent_vend_form_alter(&$form, $db_row) {
  avbase_category_agent_form_alter($form, $db_row);
}

function avbase_category_agent_form_alter(&$form, $db_row) {
  $entity_name = $form['entity_name']['#value'];
  //switch ($entity_name) {
  //
  //}
  $form['title']['#title'] = t('Agent name');
  $form['description']['#access'] = FALSE;

  $form['#attached']['js'] = array(
    AVBASE_MODULE_PATH . '/js/agents.js',
  );
  switch ($entity_name) {
    case 'agent_vend':
      $form['vendor_name'] = array(
        '#id' => 'vendor-name',
        '#type' => 'textfield',
        '#title' => t('Vendor'),
        '#size' => 30,
        '#maxlength' => 60,
        '#autocomplete_path' => 'user/autocomplete',
        //'#default_value' => 'aAron',
        '#weight' => 1.2,
        '#required' => TRUE,
        '#attributes' => array(
          'data-twin-id' => 'vendor-id',
        ),
      );
      $form['vendor_id'] = array(
        '#id' => 'vendor-id',
        '#type' => 'textfield',
        '#element_validate' => array('element_validate_integer_positive'),
        '#required' => TRUE,
      );
      break;
  }
  //dpm($form);
  //$form['system_default'] = array(
  //  '#type' => 'checkbox',
  //  '#title' => 'Use this UOM as system default',
  //  '#weight' => 1.2,
  //  '#default_value' => !empty($db_row->system_default),
  //);
  //$form['plural_form'] = array(
  //  '#type' => 'textfield',
  //  '#title' => 'Plural form',
  //  '#default_value' => isset($db_row->data['plural_form']) ? check_plain($db_row->data['plural_form']) : '',
  //  '#maxlength' => 32,
  //  '#weight' => 1.1,
  //);
  //
  ////$form['data_ids']['#value'][] = 'default';
  //$form['data_ids']['#value'][] = 'plural_form';
  //$form['save_field_ids']['#value'][] = 'system_default';
  ////array_unshift($form['#submit'], 'avbase_category_uom_form_submit');
}