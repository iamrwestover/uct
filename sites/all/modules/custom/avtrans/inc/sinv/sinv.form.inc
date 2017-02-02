<?php
module_load_include('inc', 'avtrans', 'inc/item_list.form');
module_load_include('inc', 'avtrans', 'inc/trans.cost_changes.form');
//module_load_include('inc', 'avtrans', 'inc/trans.client_changes.form');
module_load_include('inc', 'avtrans', 'inc/trans.pending_orders.form');
module_load_include('inc', 'avtrans', 'inc/sinv/sinv.handlers');
module_load_include('inc', 'avtrans', 'inc/gr/gr.handlers');

/**
 * sinv form.
 *
 * @param $form
 * @param $form_state
 * @param $row
 * @return array
 */
function avtrans_sinv_form($form, &$form_state, $row, $view_mode = FALSE) {
  $form['#id'] = 'avtrans-sinv-form';
  //$form['#attached']['js'][] = AVTRANS_MODULE_PATH . '/js/avtrans.form.js';
  $form['#prefix'] = '<div id="avtrans-transaction-form-wrapper"><!-- Do not remove me. -->';
  $form['#suffix'] = '</div>';

  // Set crud settings.
  avtrans_sinv_form_settings($form, $form_state, $row, $view_mode);

  // Add generic sinv form fields.
  avtrans_sinv_form_default($form, $form_state, $row, $view_mode);

  // Add fields for Cost Changes popup.
  //avtrans_cost_changes_form_fields($form, $form_state, $row, $view_mode);

  // Add fields for Customer SOs popup.
  avtrans_pending_orders_form_fields($form, $form_state, $row, $view_mode);

  // Add extra button to load customer returns.
  $form['buttons']['check_returns'] = $form['pending_orders']['buttons']['check_po'];
  $form['buttons']['check_returns']['#avposition'] = 'opposite';
  $form['buttons']['check_returns']['#value'] = t('Add returns');
  //$form['buttons']['check_returns']['#weight'] = -1;
  //$form['buttons']['check_returns']['#icon_key'] = 'money';
  $form['buttons']['check_returns']['#id'] = 'check-ret-btn';
  $form['buttons']['check_returns']['#reference_type'] = 'ret';
  $form['buttons']['check_returns']['#access'] = TRUE;
  $form['buttons']['check_returns']['#disabled'] = TRUE;

  //$form['buttons']['check_returns']['#attributes']['class']['margin'] = 'uk-margin-large-right';
  unset($form['buttons']['check_returns']['#attributes']['class']['color']);
  unset($form['buttons']['check_returns']['#attributes']['class']['hide']);

  // Count pending returns.
  $client_name = empty($row->client_name) ? NULL : $row->client_name;
  $client_name = empty($client_name) ? (empty($form_state['values']['client_name']) ? NULL : $form_state['values']['client_name']) : $client_name;
  if (!empty($client_name)) {
    $query = "SELECT COUNT(avret.id) FROM {avtbl_ret} avret
    INNER JOIN {avtbl_customers} avcust ON avcust.id = avret.client_id AND avcust.display_name = :client_name
    WHERE avret.status = 1";
    $count = db_query($query, array(':client_name' => $client_name))->fetchField();
    if (!empty($count)) {
      //$form['buttons']['check_returns']['#attributes']['class']['color'] = 'uk-button-success';
      $form['buttons']['check_returns']['#value'] .= " - <em>$count pending</em>";
      $form['buttons']['check_returns']['#disabled'] = FALSE;
    }
  }

  // Convert form to non-edit mode if we are on View page.
  avbase_allow_form_view_mode($form, $view_mode);
  return $form;
}

/**
 * Default Goods Received form fields.
 * @param $form
 * @param $form_state
 * @param $row
 *
 * @return array
 */
function avtrans_sinv_form_default(&$form, &$form_state, $row, $view_mode = FALSE) {
  $form['id'] = array('#type' => 'value', '#value' => isset($row->id) ? $row->id : NULL);
  $page_title = empty($row->id) ? t('New Invoice') : t('Invoice');
  drupal_set_title($page_title);

  $form['#theme'] = 'avtrans_item_list_form';

  // Goods Received fields.
  avtrans_sinv_form_fields($form, $form_state, $row, $view_mode);

  // Item List form fields.
  avtrans_item_list_form_fields($form, $form_state, $row, $view_mode);

  // Set common fields when purchasing items.
  avtrans_sales_fields($form, $form_state, $row, $view_mode);

  // Buttons.
  avtrans_gr_form_buttons($form, $form_state, $row, $view_mode);
  unset($form['buttons']['submit']['#id']);

  $form['#hide_discount_fields'] = TRUE;
  $form['discount_type']['#access'] = FALSE;
  $form['discount_value']['#access'] = FALSE;
}

/**
 * Set crud form settings.
 * @param $form
 * @param $form_state
 * @param $row
 */
function avtrans_sinv_form_settings(&$form, &$form_state, $row, $view_mode = FALSE) {
  $js_settings = array('avtrans' => array('transaction' => 'sales'));
  $form['#attached']['js'][] = array(
    'data' => $js_settings,
    'type' => 'setting',
  );

  // Get form storage.
  $form_state['storage'] = isset($form_state['storage']) ? $form_state['storage'] : array();
  $_storage = &$form_state['storage'];
  $_storage['transaction'] = $js_settings['avtrans']['transaction'];
  $_storage['transaction_type'] = 'sinv';

  // Set generic submit handler.
  //$form['#submit'] = array('avbase_crud_form_submit');

  $table_name = 'avtbl_sinv';
  $form['table_name'] = array('#type' => 'value', '#value' => $table_name);
  $form['entity_name'] = array('#type' => 'value', '#value' => 'sinv');
  $form['#success_message'] = t('Invoice details saved.');

  // Set autocomplete off.
  $form['#attributes']['autocomplete'] = 'off';

  // Set field names to save.
  $save_field_ids = array(
    'created',
    'client_id',
    'memo',
    'due_date',
    //'reference_id',
    //'return_type' => array('type' => 'int'),
    'term_id' => array('type' => 'int'),
    'agent_id' => array('type' => 'int'),
  );
  $form['save_field_ids'] = array(
    '#type' => 'value',
    '#value' => $save_field_ids,
  );

  // Set which fields belong to data field.
  $data_ids = array(
    //'uoms' => array('type' => 'array'),
  );
  $form['data_ids'] = array(
    '#type' => 'value',
    '#value' => $data_ids,
  );
}

/**
 * Generic DR form fields.
 * @param $form
 * @param $form_state
 * @param $row
 */
function avtrans_sinv_form_fields(&$form, &$form_state, $row, $view_mode = FALSE) {
  $term_options = avbase_get_category_options_array('payment_term');
  asort($term_options);
  $form['term_id'] = array(
    '#id' => 'term-id',
    '#type' => 'select',
    '#title' => 'Payment term',
    '#options' => $term_options,
    '#default_value' => isset($row->term_id) ? $row->term_id : 0,
    '#empty_value' => 0,
    '#empty_option' => '',
    '#avbase_payment_terms_js' => array('preventChildrenHide' => TRUE),
  );

  $date_format = 'M. d, Y';
  $form['due_date'] = array(
    '#id' => 'due-date',
    '#type' => 'textfield',
    '#title' => 'Due date',
    '#default_value' => !empty($row->due_date) ? format_date($row->due_date, 'custom', $date_format) : '',
    '#element_validate' => array('avbase_element_validate_date'),
    '#av_date_format' => $date_format,
    '#icon_key' => 'calendar',
    '#attributes' => array(
      'data-uk-datepicker' => "{format:'MMM. DD, YYYY', minDate: '" . format_date(time(), 'custom', $date_format) . "'}",
    ),
  );

  //$form['reference_id'] = array(
  //  '#type' => 'textfield',
  //  '#title' => 'Reference ID',
  //  '#default_value' => isset($row->reference_id) ? $row->reference_id : '',
  //  '#maxlength' => 32,
  //  '#required' => TRUE,
  //);

  //if (!empty($form_state['values']['selected_pos']['transaction_id'])) {
  //  $form_state['input']['return_type'] = AVTRANS_RETURN_TYPE_RUD;
  //}
  //$form['return_type'] = array(
  //  '#id' => 'return-type',
  //  '#type' => 'select',
  //  '#title' => 'Return type',
  //  '#options' => avtrans_get_return_types(),
  //  '#default_value' => isset($row->return_type) ? $row->return_type : '',
  //  '#required' => TRUE,
  //  '#empty_value' => '',
  //  '#empty_option' => '- Select -',
  //);
}
