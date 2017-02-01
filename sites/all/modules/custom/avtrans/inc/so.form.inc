<?php
module_load_include('inc', 'avtrans', 'inc/so.handlers');
//module_load_include('inc', 'avtrans', 'inc/trans.client_changes.form');
module_load_include('inc', 'avtrans', 'inc/item_list.form');

/**
 * Sales Order form.
 *
 * @param $form
 * @param $form_state
 * @param $row
 * @return array
 */
function avtrans_so_form($form, &$form_state, $row, $view_mode = FALSE) {
  $form['id'] = array('#type' => 'value', '#value' => isset($row->id) ? $row->id : NULL);
  //$form['#attached']['js'][] = AVTRANS_MODULE_PATH . '/js/avtrans.form.js';
  $page_title = empty($row->id) ? t('New sales order') : t('Sales Order');
  drupal_set_title($page_title);

  // Set crud settings.
  avtrans_so_form_settings($form, $form_state, $row, $view_mode);

  // Item List form fields.
  avtrans_item_list_form_fields($form, $form_state, $row, $view_mode);

  // Set common fields when purchasing items.
  avtrans_sales_fields($form, $form_state, $row, $view_mode);

  $form['header_total'] = array(
    '#markup' => '<div><div class="uk-text-bold uk-text-uppercase">AMOUNT</div><div><h1 class="product-form-grand-total">0.00</h1></div></div>',
  );
  $form['footer_total'] = array(
    '#markup' => '<div><div class="uk-text-bold uk-text-uppercase">AMOUNT</div><div><h1 class="product-form-grand-total">0.00</h1></div></div>',
  );

  $form['transaction_date']['#title'] = t('Sales Order Date');

  // Buttons
  $form['buttons'] = avtrans_transaction_buttons('so', $view_mode);

  // Convert form to non-edit mode if we are on View page.
  avbase_allow_form_view_mode($form, $view_mode);
  return $form;
}

/**
 * Set crud form settings.
 * @param $form
 */
function avtrans_so_form_settings(&$form, &$form_state, $row, $view_mode = FALSE) {
  $js_settings = array('avtrans' => array('transaction' => 'sales'));
  $form['#attached']['js'][] = array(
    'data' => $js_settings,
    'type' => 'setting',
  );

  // Get form storage.
  $form_state['storage'] = isset($form_state['storage']) ? $form_state['storage'] : array();
  $_storage = &$form_state['storage'];
  $_storage['transaction'] = $js_settings['avtrans']['transaction'];

  // Table name.
  $table_name = 'avtbl_so';

  // Set generic submit handler.
  $form['#submit'] = array('avbase_crud_form_submit', 'avtrans_redirect_transaction_form');

  $form['table_name'] = array('#type' => 'value', '#value' => $table_name);
  $form['entity_name'] = array('#type' => 'value', '#value' => 'so');
  $form['#success_message'] = t('Sales Order details saved.');
  $form['#theme'] = 'avtrans_item_list_form';

  // Set autocomplete off.
  $form['#attributes']['autocomplete'] = 'off';

  // Set field names to save.
  $save_field_ids = array(
    'created',
    'client_id',
    'agent_id' => array('type' => 'int'),
    'memo',
    //'message',
    //'email',
    //'term_id' => array('type' => 'int'),
    'discount_type' => array('type' => 'int'),
    'discount_value' => array('type' => 'float'),
  );
  if (empty($form['id']['#value'])) {
    $save_field_ids[] = 'transaction_date';
  }

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
