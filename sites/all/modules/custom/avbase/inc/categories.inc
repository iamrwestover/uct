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
  $category_menu_path = 'av/categories/' . $category['menu_path'];
  $btn_options = array('attributes' => array('class' => AV_STYLE_CLASS_BTN . ' ' . AV_STYLE_CLASS_BTN_PRIMARY));
  $form['new_button'] = array('#markup' => l('New ' . strtolower($category['name']), $category_menu_path . '/add', $btn_options));

  $header = array(
    array('data' => 'ID', 'field' => 'id', 'sort' => 'asc', 'class' => array('uk-hidden')),
    array('data' => 'Name', 'field' => 'title'),
    array('data' => 'Description', 'field' => 'description'),
    '&nbsp;',
  );

  $rows = array();
  $result = db_select('avtbl_category_groups', 'acg')
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
  return $form;
}

/**
 * Set crud form settings.
 * @param $form
 */
function avbase_category_form_settings(&$form) {
  $table_name = 'avtbl_category_groups';

  // Set generic submit handler.
  $form['#submit'] = array('avbase_crud_form_submit');
  $form['table_name'] = array('#type' => 'value', '#value' => $table_name);

  // Set field names to save.
  $save_field_ids = array(
    'code',
    'title',
    'category_id',
    'uom_id',
    'description',
    'qty',
    'rop',
    'eoq',
    'cost',
    'price',
  );
  $form['save_field_ids'] = array(
    '#type' => 'value',
    '#value' => $save_field_ids,
  );
  // Set which fields belong to data field.
  $form['data_ids'] = array(
    '#type' => 'value',
    '#value' => array(),
  );
}
