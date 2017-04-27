<?php

function avtxns_account_chart_home_form($form, &$form_state) {
  $btn_options = array('attributes' => array('class' => AV_STYLE_CLASS_BTN . ' ' . AV_STYLE_CLASS_BTN_PRIMARY));
  $form['buttons']['new_button'] = array(
    '#markup' => l('New account', 'av/chart-of-accounts/new', $btn_options),
    //'#access' => user_access(AVPRODUCTS_PERM_PRODUCT_CREATE),
  );
  $form['buttons']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save sorting changes'),
    '#attributes' => array(
      'class' => array('uk-margin-small-left'),
      'data-ks' => 'CTRL+S',
    )
  );

  // Identify that the elements in 'tabledrag' are a collection, to
  // prevent Form API from flattening the array when submitted.
  $form['tabledrag']['#tree'] = TRUE;

  $query = "SELECT * FROM {avtbl_account_chart} WHERE status AND IFNULL(parent, 0) < 1 ORDER BY weight";
  $result = db_query($query);
  $rows = array();
  $depth = -1;
  foreach ($result as $row) {
    avtxns_account_chart_get_tree($row, $rows, $depth);
  }

  // Iterate through each database result.
  foreach ($rows as $row) {
    $act = avtxns_account_chart_types($row->account_type);
    // Create a form entry for this item.
    //
    // Each entry will be an array using the the unique id for that item as
    // the array key, and an array of table row data as the value.
    $form['tabledrag'][$row->id] = array(

      // We'll use a form element of type '#markup' to display the item name.
      'title' => array(
        '#markup' => l($row->title, 'av/chart-of-accounts/' . $row->id . '/view', array('query' => drupal_get_destination())),
      ),
      'edit' => array(
        '#markup' => l('EDIT', 'av/chart-of-accounts/' . $row->id . '/edit', array('query' => drupal_get_destination())),
      ),
      'account_type' => array(
        '#markup' => $act['name'],
      ),

      // For parent/child relationships, we also need to add form items to
      // store the current item's unique id and parent item's unique id.
      //
      // We would normally use a hidden element for this, but for this example
      // we'll use a disabled textfield element called 'id' so that we can
      // display the current item's id in the table.
      //
      // Because tabledrag modifies the #value of this element, we use
      // '#default_value' instead of '#value' when defining a hidden element.
      // Also, because tabledrag modifies '#value', we cannot use a markup
      // element, which does not support the '#value' property. (Markup
      // elements use the '#markup' property instead.)
      'id' => array(
         '#type' => 'hidden',
        // '#default_value' => $row->id,
        //'#type' => 'textfield',
        '#size' => 10,
        '#default_value' => $row->id,
        '#disabled' => TRUE,
      ),

      // The same information holds true for the parent id field as for the
      // item id field, described above.
      'parent' => array(
        // '#type' => 'hidden',
        // '#default_value' => $row->parent,
        '#type' => 'textfield',
        '#size' => 10,
        '#default_value' => empty($row->parent) ? 0 : $row->parent,
      ),

      // The 'weight' field will be manipulated as we move the items around in
      // the table using the tabledrag activity.  We use the 'weight' element
      // defined in Drupal's Form API.
      'weight' => array(
        '#type' => 'weight',
        '#title' => t('Weight'),
        '#default_value' => empty($row->weight) ? 0 : $row->weight,
        '#delta' => 10,
        '#title_display' => 'invisible',
      ),

      // We'll use a hidden form element to pass the current 'depth' of each
      // item within our parent/child tree structure to the theme function.
      // This will be used to calculate the initial amount of indentation to
      // add before displaying any child item rows.
      'depth' => array(
        '#type' => 'hidden',
        '#value' => $row->depth,
      ),
      'fixed' => array(
        '#type' => 'value',
        '#value' => empty($row->fixed) ? 0 : $row->fixed,
      ),
    );
  }

  return $form;
}

function avtxns_account_chart_home_form_submit($form, &$form_state) {
  // Because the form elements were keyed with the item ids from the database,
  // we can simply iterate through the submitted values.
  //$ac = avtxns_account_chart_query_load(array('1'));
  //dpm($ac);
  foreach ($form_state['values']['tabledrag'] as $id => $item) {
    db_query(
      "UPDATE {avtbl_account_chart} SET weight = :weight, parent = :parent WHERE id = :id",
      array(':weight' => $item['weight'], ':parent' => $item['parent'], ':id' => $id)
    );
  }
}

/**
 * Recursive function to build chart of account list with parent-children indentations.
 * @param $tree
 * @param int $depth
 *
 * @return array
 */
function avtxns_account_chart_rows($tree, $depth = 0) {
  $rows = array();
  $can_edit = TRUE;
  foreach ($tree as $row) {
    $row = (object) $row;
    $depth_html = '';
    for ($i = 0; $i < $depth; $i++) {
      $depth_html .= '<span class="uk-margin-left"></span>';
    }

    $color_class = empty($row->status) ? 'uk-alert-mute' : '';
    $menu_prefix = 'av/chart-of-accounts/' . $row->id;
    $destination = url('av/chart-of-accounts', array('query' => drupal_get_query_parameters()));
    $action_links =  array();
    if ($can_edit) {
      $action_links['edit'] = array('title' => 'edit', 'href' => $menu_prefix . '/edit', 'query' => array('destination' => $destination));
    }
    $actions_html = theme('links__bare', array('links' => $action_links));
    $linked_name = l($row->title, $menu_prefix . '/view');
    $acd = avtxns_account_chart_types($row->account_type);
    $rows[$row->id] = array(
      'data' => array(
        'title' => $depth_html . $linked_name,
        'type' => check_plain($acd['name']),
        //'balance_text' => array('data' => isset($row->account_balance) ? number_format($row->account_balance, 2) : '-', 'class' => array('uk-text-center')),
        //'balance' => array(
        //  'data' => empty($row->account_balance) ? 0 : $row->account_balance,
        //  'class' =>
        //),
        'actions' => $actions_html,
      ),
      'class' => array($color_class),
    );

    if (!empty($row->children)) {
      $rows += avtxns_account_chart_rows($row->children, $depth + 1);
    }
  }
  return $rows;
}

/**
 * Chart of Account form.
 * @param $form
 * @param $form_state
 * @param $row
 * @param bool|FALSE $view_mode
 *
 * @return mixed
 */
function avtxns_account_chart_form($form, &$form_state, $row, $view_mode = FALSE) {
  $form['id'] = array('#type' => 'value', '#value' => (isset($row->id) ? $row->id : NULL));
  $page_title = empty($form['id']['#value']) ? 'New account' : 'Account';
  drupal_set_title($page_title);

  // Set crud settings.
  avtxns_account_chart_form_settings($form, $row, $view_mode);

  // Fields
  avtxns_account_chart_form_fields($form, $form_state, $row, $view_mode);

  $form['buttons']['#theme'] = 'avbase_crud_button_group';
  $form['buttons']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#attributes' => array('data-ks' => 'CTRL+S'),
  );
  $cancel_text = $view_mode ? 'Close' : 'Cancel';
  $form['buttons']['cancel'] = array(
    '#markup' => l($cancel_text, 'av/chart-of-accounts', array('attributes' => array('class' => array('uk-button')))),
  );

  // Status buttons.
  if ($view_mode && !empty($row->id) && empty($row->fixed)) {
    $can_change_status = TRUE;
    $form['buttons']['activate'] = array(
      '#markup' => l('Enable', "av/chart-of-accounts/{$row->id}/" . AVBASE_RECORD_ACTIVE . "/change-status", array(
        'attributes' => array('class' => 'uk-button uk-button-primary uk-margin-small-right'),
        'query' => array('destination' => 'av/chart-of-accounts/' . $row->id . '/view'),
        'icon_key' => 'check',
        'icon_classes' => array('uk-margin-small-right'),
      )),
      '#avposition' => 'opposite',
      '#access' => $can_change_status && $row->status != AVBASE_RECORD_ACTIVE,
    );
    $form['buttons']['disable'] = array(
      '#markup' => l('Disable', "av/chart-of-accounts/{$row->id}/" . AVBASE_RECORD_DISABLED . "/change-status", array(
        'attributes' => array('class' => 'uk-button uk-margin-small-right'),
        'query' => array('destination' => 'av/chart-of-accounts/' . $row->id . '/view'),
        'icon_key' => 'ban',
        'icon_classes' => array('uk-margin-small-right'),
      )),
      '#avposition' => 'opposite',
      '#access' => $can_change_status && $row->status == AVBASE_RECORD_ACTIVE,
    );

    if ($row->status != AVBASE_RECORD_ACTIVE) {
      $status_title = 'DISABLED';
      $form_prefix = empty($form['#prefix']) ? '' : $form['#prefix'];
      $form_prefix = '<div class="uk-alert uk-alert-mute"><span class="uk-badge uk-badge-void" style="position: absolute;top:10px;left:10px;">' . $status_title . '</span>' . $form_prefix;
      $form['#prefix'] = $form_prefix;
      $form_suffix = empty($form['#suffix']) ? '' : $form['#suffix'];
      $form_suffix .= '</div>';
      $form['#suffix'] = $form_suffix;
    }
  }

  // Convert form to non-edit mode if we are on View page.
  avbase_allow_form_view_mode($form, $view_mode);

  return $form;
}

/**
 * Set crud form settings.
 *
 * @param $form
 * @param $row
 * @param bool|FALSE $view_mode
 */
function avtxns_account_chart_form_settings(&$form, $row, $view_mode = FALSE) {
  $table_name = 'avtbl_account_chart';

  $form['last_changed'] = array('#type' => 'hidden', '#default_value' => (isset($row->changed) ? $row->changed : NULL));

  // Set generic submit handler.
  $form['#submit'] = array('avbase_crud_form_submit');
  $form['table_name'] = array('#type' => 'value', '#value' => $table_name);
  $form['entity_name'] = array('#type' => 'value', '#value' => 'account_chart');

  // Generic form validator.
  $form['#validate'][] = 'avbase_crud_form_validate';

  // Set autocomplete off.
  $form['#attributes']['autocomplete'] = 'off';

  // Set field names to save.
  $db_op = empty($form['id']['#value']) ? 'insert' : 'update';
  $save_field_ids = avtxns_account_chart_table_fields_to_save($db_op);
  $form['save_field_ids'] = array(
    '#type' => 'value',
    '#value' => $save_field_ids,
  );
}

/**
 * Return definition of account chart table fields.
 * @param $db_op
 * @return array
 */
function avtxns_account_chart_table_fields_to_save($db_op = 'insert') {
  $fields = array(
    'title',
    'account_type',
    'parent',
    //'weight' => array('type' => 'int'),
    'created',
    'uid',
    'changed',
  );

  if ($db_op == 'update') {
    unset($fields['created']);
  }
  return $fields;
}

/**
 * Account Chart form fields.
 *
 * @param $form
 * @param $form_state
 * @param $row
 * @param bool|FALSE $view_mode
 */
function avtxns_account_chart_form_fields(&$form, &$form_state, $row, $view_mode = FALSE) {
  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => 'Account name',
    '#default_value' => isset($row->title) ? $row->title : '',
    '#maxlength' => 255,
    '#required' => TRUE,
    '#element_validate' => array('avbase_element_validate_unique_field'),
  );

  $chart_types_options = avtxns_account_chart_types_option_array();
  $form['account_type'] = array(
    '#type' => 'select',
    '#title' => 'Type',
    '#options' => $chart_types_options,
    '#empty_value' => '',
    '#empty_option' => '- Select -',
    '#default_value' => isset($row->account_type) ? $row->account_type : '',
    '#required' => TRUE,
    //'#disabled' => !empty($row->id),
    '#access' => empty($row->fixed),
  );

  $account_type_string = empty($row->account_type) ? '' : $row->account_type;
  $account_type_string = '';
  $form['parent_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Parent account'),
    '#maxlength' => 255,
    '#autocomplete_path' => "av/chart-of-accounts/autocomplete/" . $account_type_string,
    '#element_validate' => array('avtxns_element_validate_account_chart_name'),
    //'#element_validate_conditions' => array('group_id' => "agent{$ctd['type']}"),
    '#element_save_id' => 'parent',
    '#default_value' => empty($row->parent_name) ? '' : $row->parent_name,
    //'#access' => empty($row->fixed),
    '#parent_chart_of_account' => TRUE,
  );


  //$query = "SELECT * FROM avtbl_account_chart";
  //$rs = db_query($query);
  //$rows = array();
  //foreach ($rs as $row) {
  //  $rows[$row->id] = $row;
  //}
  //dpm($rows);
  //$row_tree = avbase_rows_build_tree($rows);
  //dpm($row_tree);
}

/**
 * Menu callback: confirm chart of account status change.
 * @param $form
 * @param $form_state
 * @param object $row
 * @param $status
 *
 * @return array
 */
function avtxns_account_chart_change_status_confirm_form($form, &$form_state, $row, $status) {
  if (!empty($row->fixed)) {
    drupal_set_message('Cannot change status of fixed accounts.', 'error');
    return array();
  }

  switch ($status) {
    case AVCLIENT_STATUS_DISABLED:
      $prompt = 'Are you sure you want to disable this account?';
      break;
    case AVCLIENT_STATUS_ACTIVE:
      $prompt = 'Are you sure you want to enable this account?';
      break;
    default:
      $prompt = 'Are you sure?';
      break;
  }

  $display_name = check_plain($row->title);
  $description = <<<HTML
  <h1 class="uk-article-title">$display_name</h1>
HTML;
  $form_state['row'] = $row;
  $form_state['status'] = $status;
  return confirm_form(array(), $prompt, '', $description);
}

/**
 * Handler for chart of account status change.
 *
 * @see avtxns_account_chart_change_status_confirm_form()
 */
function avtxns_account_chart_change_status_confirm_form_submit($form, &$form_state) {
  if (isset($form_state['row']->id)) {
    $status = isset($form_state['status']) ? $form_state['status'] : '';
    if (avtxns_account_chart_change_status($form_state['row']->id, $status)) {

      switch ($status) {
        case AVBASE_RECORD_DISABLED:
          //drupal_set_message('Client\'s account has been disabled.');
          break;
        case AVBASE_RECORD_ACTIVE:
          drupal_set_message('Account has been enabled.');
          break;
        default:
          break;
      }
    }
  }
  $form_state['redirect'] = '';
}

function theme_avtxns_account_chart_home_form($variables) {
  $form = $variables['form'];

  $output = '';
  $output .= drupal_render($form['buttons']);

  // Initialize the variable which will store our table rows.
  $rows = array();

  // Iterate over each element in our $form['tabledrag'] array.
  foreach (element_children($form['tabledrag']) as $id) {
    $form['tabledrag'][$id]['weight']['#attributes']['class'] = array('tabledrag-weight');

    $form['tabledrag'][$id]['id']['#attributes']['class'] = array('tabledrag-id');
    $form['tabledrag'][$id]['parent']['#attributes']['class'] = array('tabledrag-parent');

    $class = array();
    //if (empty($form['tabledrag'][$id]['fixed']['#value'])) {
      $class[] = 'draggable';
    //}

    // We can add the 'tabledrag-root' class to a row in order to indicate
    // that the row may not be nested under a parent row.  In our sample data
    // for this example, the description for the item with id '8' flags it as
    // a 'root' item which should not be nested.
    //if (!empty($form['tabledrag'][$id]['fixed']['#value']) && empty($form['tabledrag'][$id]['parent']['#default_value'])) {
    //  $class[] = 'tabledrag-root';
    //}

    // We can add the 'tabledrag-leaf' class to a row in order to indicate
    // that the row may not contain child rows.  In our sample data for this
    // example, the description for the item with id '9' flags it as a 'leaf'
    // item which can not contain child items.
    //if ($id == '9') {
    //  $class[] = 'tabledrag-leaf';
    //}

    if (in_array($id, array(AVTXN_AC_SPECIFIC_AR_ID, AVTXN_AC_SPECIFIC_AP_ID))) {
      $class[] = 'tabledrag-leaf';
    }

    $indent = theme('indentation', array('size' => $form['tabledrag'][$id]['depth']['#value']));
    unset($form['tabledrag'][$id]['depth']);

    $rows[] = array(
      'data' => array(
        // Add our 'name' column, being sure to include our indentation.
        $indent . drupal_render($form['tabledrag'][$id]['title']) . drupal_render($form['tabledrag'][$id]['id']),

        // Add our 'description' column.
        drupal_render($form['tabledrag'][$id]['account_type']),
        // Add our 'weight' column.
        drupal_render($form['tabledrag'][$id]['weight']),
        // Add our hidden 'id' column.
        //drupal_render($form['tabledrag'][$id]['id']),
        // Add our hidden 'parent id' column.
        drupal_render($form['tabledrag'][$id]['parent']),
        drupal_render($form['tabledrag'][$id]['edit']),
      ),
      'class' => $class,
    );
  }
  //$header = array(t('Title'), t('Type'), t('Weight'), t('ID'), t('Parent'), '&nbsp;');
  $header = array(t('Title'), t('Type'), t('Weight'), t('Parent'), '&nbsp;');
  $table_id = 'tabledrag-table';

  // We can render our tabledrag table for output.
  $output .= theme('table', array(
    'header' => $header,
    'rows' => $rows,
    'attributes' => array('id' => $table_id, 'class' => array('uk-table-hover uk-table-condensed')),
  ));

  // And then render any remaining form elements (such as our submit button).
  $output .= drupal_render_children($form);
  drupal_add_tabledrag($table_id, 'match', 'parent', 'tabledrag-parent', 'tabledrag-parent', 'tabledrag-id', TRUE);
  drupal_add_tabledrag($table_id, 'order', 'sibling', 'tabledrag-weight', NULL, NULL, TRUE);

  return $output;
}
