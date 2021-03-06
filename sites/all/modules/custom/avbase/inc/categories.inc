<?php

// Include specific category inc file.
module_load_include('inc', 'avbase', 'inc/categories/' . arg(2));

/**
 * Categories home page.
 * @return string
 */
function avbase_categories_home() {
  $output = '<div class="uk-grid uk-grid-small uk-margin-top">';

  $categories = avbase_category_groups();
  $x = -1;
  foreach ($categories as $k => $v) {
    $category_title = isset($v['plural_name']) ? $v['plural_name'] : $v['name'];

    $output .= '<div class="uk-width-1-4 uk-margin-bottom">';
    //$output .= '<li>';
    $output .= l(/*'<i class="uk-icon-envelope-o"></i>' . */$category_title, 'av/categories/' . $k, array('html' => TRUE,
      'attributes' => array('class' => array('uk-button', 'uk-width-1-1', 'uk-button-large', 'uk-button-primary'))));
    //$output .= '</li>';
    $output .= '</div>';
  }
  $output .= '</div>';

  return $output;
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
  $category_menu_path = 'av/categories/' . $category_id;
  $btn_options = array('attributes' => array('class' => AV_STYLE_CLASS_BTN . ' uk-button-primary'), 'icon_key' => 'file', 'icon_classes' => array('uk-margin-small-right'));
  $form['top_buttons']['new_button'] = array('#markup' => l(' New ' . strtolower($category['name']), $category_menu_path . '/add', $btn_options));
  //$form['top_buttons']['#theme'] = 'avbase_crud_button_group';

  $header = array(
    'id' => array('data' => 'ID', 'field' => 'id', 'sort' => 'asc', 'class' => array('uk-hidden'), '#weight' => 0),
    'title' => array('data' => 'Title', 'field' => 'title', '#weight' => 1),
    'description' => array('data' => 'Description', 'field' => 'description', '#weight' => 2),
    'actions' => array('data' => '&nbsp;', '#weight' => 3),
  );

  $result = db_select('avtbl_categories', 'avc')
    ->addTag('avcategory_load_' . $category_id)
    ->fields('avc')
    ->condition('group_id', $category_id)
    ->extend('PagerDefault')
    ->extend('TableSort')
    ->limit(50)
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
      //'delete' => array('title' => 'delete', 'href' => $category_menu_path . '/' . $db_row->id . '/delete', 'query' => $destination_path),
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

  $table_attributes = array('id' => 'categories-list-table', 'class' => array('uk-table-condensed uk-table-hover'));

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
  $form['pager']['#markup'] = '<br />' . theme('pager', array('tags' => array()));

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
    '#default_value' => isset($row->title) ? $row->title : '',
    '#maxlength' => 32,
    '#required' => TRUE,
    '#weight' => 1,
    '#element_validate' => array('avbase_element_validate_category_title_unique'),
  );
  $form['description'] = array(
    '#type' => 'textarea',
    '#title' => 'Description',
    '#default_value' => isset($row->description) ? $row->description : '',
    '#maxlength' => 255,
    '#weight' => 2,
  );

  $form['buttons']['#theme'] = 'avbase_crud_button_group';
  $form['buttons']['#weight'] = 3;
  $form['buttons']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#attributes' => array('data-ks' => 'CTRL+S'),
  );
  $form['buttons']['cancel'] = array(
    '#markup' => l('Cancel', 'av/categories/' . $category_id),
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

  // Set autocomplete off.
  $form['#attributes']['autocomplete'] = 'off';

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
  //if (isset($form_state['row']->id)) {
  //  $row = $form_state['row'];
  //  if (avbase_category_delete($row->id)) {
  //    drupal_set_message('Record deleted.');
  //    avbase_custom_watchdog("{$row->group_id}__delete", "ID: {$row->id}", array('#row' => $row, '#group_id' => $row->group_id));
  //  }
  //}
  drupal_set_message(t('Record delete is disabled'), 'error');
  $form_state['redirect'] = '';
}

/**
 * Alter Agent add/edit form.
 * @param $form
 * @param $db_row
 */
function avbase_category_agent_form_alter(&$form, $db_row) {
  $entity_name = $form['entity_name']['#value'];
  $form['title']['#title'] = t('Agent name');
  $form['description']['#access'] = FALSE;

  $form['#attached']['js'] = array(
    AVBASE_MODULE_PATH . '/js/agents.js',
  );

  switch ($entity_name) {
    case 'agent_vend':
      break;
  }

  //$form['title']['#element_validate'][] = 'avbase_element_validate_category_title_unique';
  $form['contact_number'] = array(
    '#type' => 'textfield',
    '#title' => 'Contact number',
    '#default_value' => isset($db_row->data['contact_number']) ? $db_row->data['contact_number'] : '',
    '#maxlength' => 32,
    '#weight' => 1.1,
  );
  $form['address'] = array(
    '#type' => 'textarea',
    '#title' => 'Address',
    '#default_value' => isset($db_row->data['address']) ? $db_row->data['address'] : '',
    '#maxlength' => 255,
    '#weight' => 1.2,
  );
  $form['data_ids']['#value'][] = 'contact_number';
  $form['data_ids']['#value'][] = 'address';
}

/**
 * Menu callback - retrieve a JSON object containing autocomplete suggestions for existing categories.
 * @param $category_ids_string
 * @param string $string
 */
function avbase_category_autocomplete($category_ids_string, $string = '') {
  $matches = array();
  $string = trim($string);
  if ($string) {
    $where = array();
    $args = array();
    $where_or = array();
    $string_arr = explode(' ', $string);
    $like_where = array();
    foreach ($string_arr as $k => $s) {
      $like_where[] = 'title LIKE :string_' . $k;
      $args[':string_' . $k] = '%' . db_like($s) . '%';
    }
    $where_or[] = '(' . implode(' AND ', $like_where) . ')';
    $where_or_str = '(' . implode(' OR ', $where_or) . ')';
    $where[] = $where_or_str;

    $category_ids = explode('-', $category_ids_string);
    $category_ids = array_filter($category_ids);
    if (!empty($category_ids)) {
      $where[] = "group_id IN(:category_ids)";
      $args[':category_ids'] = $category_ids;
    }

    // Build query.
    $query = "SELECT * FROM avtbl_categories WHERE " . implode(' AND ', $where) . "
    ORDER BY
      CASE WHEN title LIKE :like_entire_string THEN 1 WHEN title = :entire_string THEN 2 ELSE 3 END,
      title LIMIT 8";
    $args[':like_entire_string'] = '%' . db_like($string) . '%';
    $args[':entire_string'] = $string;
    $rs = db_query($query, $args);
    $ordered_rows = avbase_order_db_rows_by_best_match($rs, 'title', $string);


    foreach ($ordered_rows as $row) {
      $matches[$row->title] = check_plain($row->title);
    }
  }

  drupal_json_output($matches);
}

