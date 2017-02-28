<?php

/**
 * Users homepage form.
 *
 * @param $form
 * @param $form_state
 * @return array
 */
function avbase_users_home_form($form, &$form_state) {
  $form['new_button'] = array(
    '#markup' => l('New user', 'admin/people/create', array('attributes' => array('class' => 'uk-button uk-margin-small-right uk-button-primary'))),
    '#access' => user_access('administer users'),
  );
  $form['permissions_button'] = array(
    '#markup' => l('Permissions', 'av/users/permissions', array('attributes' => array('class' => 'uk-button uk-margin-small-right'))),
    '#access' => user_access('administer users'),
  );

  $header = array(
    array('data' => 'ID', 'field' => 'uid', 'sort' => 'desc', 'class' => array('uk-hidden')),
    array('data' => 'Name', 'field' => 'first_name'),
    array('data' => 'Username', 'field' => 'name'),
    array('data' => 'Roles'),
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
      'edit' => array('title' => 'edit', 'href' => 'user/' . $row->uid . '/edit', 'query' => drupal_get_destination()),
      'delete' => array('title' => 'delete', 'href' => 'user/' . $row->uid . '/cancel', 'query' => drupal_get_destination()),
    ));

    $actions_html = theme('links__bare', $action_links);
    $name_details = array(
      'first_name' => empty($row->first_name) ? NULL : $row->first_name,
      'last_name' => empty($row->last_name) ? NULL : $row->last_name,
    );
    $name_details = array_filter($name_details);
    $full_name = implode(' ', $name_details);
    $user_view_link = l($full_name, 'user/' . $row->uid);
    $rows[] = array(
      'data' => array(
        'id' => array('data' => $row->uid, 'class' => array('uk-hidden')),
        'name' => array('data' => $user_view_link),
        'username' => array('data' => check_plain($row->name)),
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

function avbase_user_permissions_form($form, &$form_state, $rid = NULL) {
  drupal_set_title('Permissions');

  // Retrieve role names for columns.
  $role_names = user_roles();
  unset($role_names[1]);
  unset($role_names[2]);
  unset($role_names[3]);
  if (is_numeric($rid)) {
    $role_names = array($rid => $role_names[$rid]);
  }
  // Fetch permissions for all roles or the one selected role.
  $role_permissions = user_role_permissions($role_names);

  // Store $role_names for use when saving the data.
  $form['role_names'] = array(
    '#type' => 'value',
    '#value' => $role_names,
  );
  // Render role/permission overview:
  $options = array();
  //$hide_descriptions = system_admin_compact_mode();

  // Get a list of all the modules implementing a hook_permission() and sort by
  // display name.
  //$modules = array();
  //foreach (module_implements('permission') as $module) {
  //  $modules[$module] = $module_info[$module]['name'];
  //}
  $modules = array(
    'avclients' => 'CLIENTS',
    'avproducts' => 'PRODUCTS',
    'avtxns' => 'TRANSACTIONS',
    'avreports' => 'REPORTS',
    'avbase' => 'GENERAL',
  );
  //asort($modules);

  foreach ($modules as $module => $display_name) {
    if ($permissions = module_invoke($module, 'permission')) {
      $form['permission'][] = array(
        '#markup' => $display_name,
        '#id' => $module,
      );
      foreach ($permissions as $perm => $perm_item) {
        // Fill in default values for the permission.
        $perm_item += array(
          'description' => '',
          'restrict access' => FALSE,
          'warning' => !empty($perm_item['restrict access']) ? t('Warning: Give to trusted roles only; this permission has security implications.') : '',
        );
        $options[$perm] = '';
        $form['permission'][$perm] = array(
          '#type' => 'item',
          '#markup' => $perm_item['title'],
          '#description' => theme('user_permission_description', array('permission_item' => $perm_item)),
        );
        foreach ($role_names as $rid => $name) {
          // Builds arrays for checked boxes for each role
          if (isset($role_permissions[$rid][$perm])) {
            $status[$rid][] = $perm;
          }
        }
      }
    }
  }

  // Have to build checkboxes here after checkbox arrays are built
  foreach ($role_names as $rid => $name) {
    $form['checkboxes'][$rid] = array(
      '#type' => 'checkboxes',
      '#options' => $options,
      '#default_value' => isset($status[$rid]) ? $status[$rid] : array(),
      '#attributes' => array('class' => array('rid-' . $rid)),
    );
    $form['role_names'][$rid] = array('#markup' => check_plain($name), '#tree' => TRUE);
  }

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array('#type' => 'submit', '#value' => t('Save permissions'));

  $form['#attached']['js'][] = drupal_get_path('module', 'user') . '/user.permissions.js';
  $form['#theme'] = 'user_admin_permissions';
  $form['actions']['#theme'] = 'avbase_crud_button_group';
  $form['#submit'][] = 'user_admin_permissions_submit';

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
 * After build alterations for user_profile_form.
 * @param $form
 * @param $form_state
 *
 * @return mixed
 */
function avbase_form_user_profile_form_after_build(&$form, $form_state) {
  $form['account']['notify']['#access'] = FALSE;

  $disallow_keys = array(
    'mimemail',
    'timezone',
    'block',
  );
  foreach ($disallow_keys as $k) {
    if (isset($form[$k])) {
      $form[$k]['#access'] = FALSE;
    }
  }
  return $form;
}
