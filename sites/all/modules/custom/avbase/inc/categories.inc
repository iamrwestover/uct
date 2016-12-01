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
  $category = avbase_category_groups($category_id);
  $category_menu_path = $category['menu_path'];
  $btn_options = array('attributes' => array('class' => AV_STYLE_CLASS_BTN . ' ' . AV_STYLE_CLASS_BTN_PRIMARY));
  $form['new_button'] = array('#markup' => l('New ' . strtolower($category['name']), $category_menu_path . '/add', $btn_options));

  $header = array(
    array('data' => 'ID', 'field' => 'id', 'sort' => 'asc', 'class' => array('uk-hidden')),
    array('data' => 'Title', 'field' => 'title'),
    array('data' => 'Description', 'field' => 'description'),
    '&nbsp;',
  );

  $rows = array();
  $result = db_select('avtbl_categories', 'acg')
    ->fields('acg')
    ->condition('group_id', $category_id)
    ->extend('PagerDefault')
    ->extend('TableSort')
    ->limit(10)
    ->orderByHeader($header)
    ->execute();

  $action_links_all = array();
  //$categories = avbase_get_categories_multiple(array('product_cat', 'uom'));
  foreach ($result as $row) {
    $action_links = array('links' => array(
      'edit' => array('title' => 'edit', 'href' => $category_menu_path . '/' . $row->id . '/edit'),
      'delete' => array('title' => 'delete', 'href' => $category_menu_path . '/' . $row->id . '/delete', 'query' => drupal_get_destination()),
    ));

    $action_links_all[] = $action_links;
    $actions_html = theme('links__bare', $action_links);
    $rows[] = array(
      'data' => array(
        'id' => array('data' => $row->id, 'class' => array('uk-hidden')),
        'title' => check_plain($row->title),
        'description' => check_plain($row->description),
        'actions' => $actions_html,
      ),
      'class' => array('uk-table-middle'),
    );
  }

  $table_attributes = array('id' => 'products-list-table');
  if (empty($rows)) {
    $table_markup = '<p>Nothing recorded yet.</p>';
  }
  else {
    $table_markup = theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => $table_attributes, 'action_links_all' => $action_links_all));
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
  avbase_category_form_settings($form);

  $form['group_id'] = array('#type' => 'value', '#value' => $category_id);
  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => 'Title',
    '#default_value' => isset($row->title) ? check_plain($row->title) : '',
    '#maxlength' => 255,
    '#required' => TRUE,
  );
  $form['description'] = array(
    '#type' => 'textarea',
    '#title' => 'Description',
    '#default_value' => isset($row->description) ? check_plain($row->description) : '',
    '#maxlength' => 255,
  );

  $form['buttons']['#prefix'] = '<div class="uk-margin-top"></div>';
  $form['buttons']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  $form['buttons']['cancel'] = array(
    '#markup' => l('Cancel', $category['menu_path']),
  );
  return $form;
}

/**
 * Set crud form settings.
 * @param $form
 */
function avbase_category_form_settings(&$form) {
  $table_name = 'avtbl_categories';

  // Set generic submit handler.
  $form['#submit'] = array('avbase_crud_form_submit');
  $form['table_name'] = array('#type' => 'value', '#value' => $table_name);

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
 * Alter Category edit/add form.
 * @param $form
 * @param $form_state
 */
function avbase_category_form_alter(&$form, $form_state) {
  $group_id = $form['group_id']['#value'];
  switch ($group_id) {
    case 'uom':
      dsm('s');
      break;
  }
}