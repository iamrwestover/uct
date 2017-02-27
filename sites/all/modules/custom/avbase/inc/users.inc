<?php

/**
 * Users homepage form.
 *
 * @param $form
 * @param $form_state
 * @return array
 */
function avbase_users_home_form($form, &$form_state) {
  $btn_options = array('attributes' => array('class' => AV_STYLE_CLASS_BTN . ' ' . AV_STYLE_CLASS_BTN_PRIMARY));
  $form['new_button'] = array(
    '#markup' => l('New user', 'av/users/add', $btn_options),
    '#access' => user_access('administer users'),
  );

  $header = array(
    array('data' => 'ID', 'field' => 'uid', 'sort' => 'desc', 'class' => array('uk-hidden')),
    array('data' => 'Username', 'field' => 'name'),
    array('data' => 'First name', 'field' => 'first_name'),
    array('data' => 'Last name', 'field' => 'last_name'),
    array('data' => 'Roles', 'field' => ''),
    '&nbsp;',
  );

  $rows = array();
  $query = db_select('users', 'u');
  $query->leftJoin('avtbl_users', 'avuser', 'avuser.uid = u.uid');
  $query->fields('u');
  $query->fields('avuser', array('first_name', 'last_name'));
    //->fields('avuser', array('first_name', 'last_name'))
  $query->extend('PagerDefault')
    ->extend('TableSort')
    ->limit(10)
    ->orderByHeader($header)
    ->condition('u.uid', 0, '<>')
    ->condition('u.uid', 1, '<>');
  $result = $query->execute();

  $roles = array_map('check_plain', user_roles(TRUE));
  foreach ($result as $row) {
    $users_roles = array();
    $roles_result = db_query('SELECT rid FROM {users_roles} WHERE uid = :uid', array(':uid' => $row->uid));
    foreach ($roles_result as $user_role) {
      $users_roles[] = $roles[$user_role->rid];
    }
    asort($users_roles);

    $row_data = unserialize($row->data);
    $action_links = array('links' => array(
      'edit' => array('title' => 'edit', 'href' => 'av/users/' . $row->uid . '/edit', 'query' => drupal_get_destination()),
      'delete' => array('title' => 'delete', 'href' => 'user/' . $row->uid . '/cancel', 'query' => drupal_get_destination()),
    ));

    $actions_html = theme('links__bare', $action_links);
    $first_name = isset($row->first_name) ? check_plain($row->first_name) : '';
    $last_name = isset($row->last_name) ? check_plain($row->last_name) : '';
    $user_view_link = l($row->name, 'av/users/' . $row->uid . '/view');
    $rows[] = array(
      'data' => array(
        'id' => array('data' => $row->uid, 'class' => array('uk-hidden')),
        'name' => array('data' => $user_view_link),
        'first_name' => array('data' => $first_name),
        'last_name' => array('data' => $last_name),
        'roles' => array('data' => implode(', ', $users_roles)),
        'actions' => $actions_html,
      ),
      'class' => array('uk-table-middle'),
    );
  }

  $table_attributes = array('id' => 'avbase-users-list-table');
  if (empty($rows)) {
    $table_markup = '<p>Nothing recorded yet.</p>';
  }
  else {
    $table_markup = theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => $table_attributes));
  }
  $form['table']['#markup'] = $table_markup;
  $form['pager']['#markup'] = theme('pager', array('tags' => array()));
  return $form;
}

/**
 * New User page.
 * @param object $row
 *    Customer row object.
 *
 * @return string
 */
function avbase_user_form_load($row) {
  return drupal_get_form('avbase_user_form', $row);
}

/**
 * User form.
 *
 * @param $form
 * @param $form_state
 * @param $row
 *
 * @return array
 */
function avbase_user_form($form, &$form_state, $row) {
  $page_title = empty($row->uid) ? 'New user' : 'Edit user';
  drupal_set_title($page_title);

  if (empty($row->uid)) {
    $form = user_register_form($form, $form_state);
    $form['account']['notify']['#access'] = FALSE;
  }
  else {
    module_load_include('inc', 'user', 'user.pages');
    $form = user_profile_form($form, $form_state, $row);
    $form['actions']['submit']['#value'] = 'Save changes';
    $form['actions']['cancel']['#value'] = 'Delete user';
    $form['actions']['cancel']['#attributes']['class'][] = 'uk-button-danger';
  }
//  $form['account']['roles']['#access'] = TRUE;
  $form['account']['mail']['#description'] = '';

  // Set crud settings.
  $avuser_row = avbase_avuser_load($row);
  $form['id'] = array('#type' => 'value', '#value' => isset($avuser_row->id) ? $avuser_row->id : NULL);
  avbase_user_form_settings($form);

  $form['avuser']['first_name'] = array(
    '#type' => 'textfield',
    '#title' => 'First name',
    '#default_value' => isset($avuser_row->first_name) ? check_plain($avuser_row->first_name) : '',
    '#maxlength' => 32,
  );
  $form['avuser']['last_name'] = array(
    '#type' => 'textfield',
    '#title' => 'Last name',
    '#default_value' => isset($avuser_row->last_name) ? check_plain($avuser_row->last_name) : '',
    '#maxlength' => 32,
  );
  $form['avuser']['contact_number'] = array(
    '#type' => 'textfield',
    '#title' => 'Contact number',
    '#default_value' => isset($avuser_row->contact_number) ? check_plain($avuser_row->contact_number) : '',
    '#maxlength' => 255,
  );
  $form['avuser']['address'] = array(
    '#type' => 'textarea',
    '#title' => 'Address',
    '#default_value' => isset($avuser_row->address) ? check_plain($avuser_row->address) : '',
    '#maxlength' => 255,
  );
  $form['actions']['#theme'] = 'avbase_crud_button_group';
  return $form;
}

/**
 * Set crud form settings.
 * @param $form
 */
function avbase_user_form_settings(&$form) {
  $table_name = 'avtbl_users';
  $entity_name = 'user';

  // Set generic submit handler.
  $form['#submit'][] = 'avbase_crud_form_submit';
  $form['table_name'] = array('#type' => 'value', '#value' => $table_name);
  $form['entity_name'] = array('#type' => 'value', '#value' => $entity_name);
  //$form['#hide_crud_messages'] = TRUE;

  // Set autocomplete off.
  $form['#attributes']['autocomplete'] = 'off';

  // Set field names to save.
  $save_field_ids = array(
    'uid',
    'first_name',
    'last_name',
    'contact_number',
    'address',
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
 * Implementation of hook_user_view().
 * @param $account
 * @param $view_mode
 * @param $langcode
 */
function avbase_user_view($account, $view_mode, $langcode) {
  avbase_user_extra_details($account);
  $avuser_row = empty($account->avdata) ? NULL : $account->avdata;
  drupal_set_title($avuser_row->full_name);

  //$account->content['avuser']['first_name'] = array(
  //  '#type' => 'user_profile_item',
  //  '#title' => t('First name'),
  //  '#markup' => empty($avuser_row->first_name) ? '' : $avuser_row->first_name,
  //);
  //$account->content['avuser']['last_name'] = array(
  //  '#type' => 'user_profile_item',
  //  '#title' => t('Address'),
  //  '#markup' => empty($avuser_row->last_name) ? '' : $avuser_row->last_name,
  //);
  $account->content['avuser']['contact_number'] = array(
    '#type' => 'user_profile_item',
    '#title' => t('Contact number'),
    '#markup' => empty($avuser_row->contact_number) ? '' : $avuser_row->contact_number,
  );
  $account->content['avuser']['address'] = array(
    '#type' => 'user_profile_item',
    '#title' => t('Address'),
    '#markup' => empty($avuser_row->address) ? '' : $avuser_row->address,
  );
}
