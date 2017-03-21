<?php

function avtxns_account_chart_home_form($form, &$form_state) {
  $btn_options = array('attributes' => array('class' => AV_STYLE_CLASS_BTN . ' ' . AV_STYLE_CLASS_BTN_PRIMARY));
  $form['new_button'] = array(
    '#markup' => l('New account', 'av/chart-of-accounts/new', $btn_options),
    //'#access' => user_access(AVPRODUCTS_PERM_PRODUCT_CREATE),
  );
  return $form;
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
  );
  $cancel_text = $view_mode ? 'Close' : 'Cancel';
  $form['buttons']['cancel'] = array(
    '#markup' => l($cancel_text, 'av/chart-of-accounts', array('attributes' => array('class' => array('uk-button')))),
  );

  // Status buttons.
  if ($view_mode && !empty($row->id)) {
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
    '#disabled' => !empty($row->id),
  );

  $account_type_string = empty($row->account_type) ? '-' : $row->account_type;
  $form['parent_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Parent'),
    '#maxlength' => 255,
    '#autocomplete_path' => "av/chart-of-accounts/autocomplete/" . $account_type_string,
    '#element_validate' => array('avtxns_element_validate_account_chart_parent'),
    //'#element_validate_conditions' => array('group_id' => "agent{$ctd['type']}"),
    '#element_save_id' => 'parent',
    '#default_value' => empty($row->parent_name) ? '' : $row->parent_name,
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
