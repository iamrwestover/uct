<?php
module_load_include('inc', 'avtrans', 'inc/item_list.form');
module_load_include('inc', 'avtrans', 'inc/trans.cost_changes.form');
module_load_include('inc', 'avtrans', 'inc/trans.pending_orders.form');
module_load_include('inc', 'avtrans', 'inc/delivery/delivery.handlers');
module_load_include('inc', 'avtrans', 'inc/gr/gr.handlers');

/**
 * Delivery form.
 *
 * @param $form
 * @param $form_state
 * @param $row
 * @return array
 */
function avtrans_delivery_form($form, &$form_state, $row) {
  $form['#id'] = 'avtrans-gr-form';
  $form['#prefix'] = '<div id="avtrans-transaction-form-wrapper"><!-- Do not remove me. -->';
  $form['#suffix'] = '</div>';

  // Set crud settings.
  avtrans_delivery_form_settings($form, $form_state, $row);

  // Add generic Delivery form fields.
  avtrans_delivery_form_default($form, $form_state, $row);

  // Add fields for Cost Changes popup.
  avtrans_cost_changes_form_fields($form, $form_state, $row);

  // Add fields for Customer SOs popup.
  avtrans_pending_orders_form_fields($form, $form_state, $row);

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
function avtrans_delivery_form_default(&$form, &$form_state, $row) {
  $form['id'] = array('#type' => 'value', '#value' => isset($row->id) ? $row->id : NULL);
  switch (arg(4)) {
    case 'view':
      $page_title = t('Delivery item details');
      break;
    case 'edit':
      $page_title = t('Edit purchase order');
      break;
    default:
      $page_title = t('New Delivery');
      break;
  }
  drupal_set_title($page_title);

  $form['#theme'] = 'avtrans_item_list_form';

  // Goods Received fields.
  avtrans_delivery_form_fields($form, $form_state, $row);

  // Item List form fields.
  avtrans_item_list_form_fields($form, $form_state, $row);

  // Set common fields when purchasing items.
  avtrans_sales_fields($form, $form_state, $row);

  // Buttons.
  avtrans_delivery_form_buttons($form, $form_state, $row);
}

/**
 * Set crud form settings.
 * @param $form
 * @param $form_state
 * @param $row
 */
function avtrans_delivery_form_settings(&$form, &$form_state, $row) {
  // Get form storage.
  $form_state['storage'] = isset($form_state['storage']) ? $form_state['storage'] : array();
  $_storage = &$form_state['storage'];
  $_storage['transaction_type'] = 'dr';

  // Set flag if this is view mode.
  $form['#av_view_mode'] = (arg(4) == 'view');

  // Set generic submit handler.
  //$form['#submit'] = array('avbase_crud_form_submit');

  $table_name = 'avtbl_dr';
  $form['table_name'] = array('#type' => 'value', '#value' => $table_name);
  $form['entity_name'] = array('#type' => 'value', '#value' => 'dr');
  $form['#success_message'] = t('Delivery details saved.');

  // Set autocomplete off.
  $form['#attributes']['autocomplete'] = 'off';

  // Set field names to save.
  $save_field_ids = array(
    'created',
    'client_id',
    'term_id' => array('type' => 'int'),
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
function avtrans_delivery_form_fields(&$form, &$form_state, $row) {
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
}

/**
 * Buttons for Goods Received form.
 * @param $form
 * @param $form_state
 * @param $row
 */
function avtrans_delivery_form_buttons(&$form, $form_state, $row) {
  $form['buttons']['#theme'] = 'avbase_crud_button_group';
  $form['buttons']['submit'] = array(
    '#id' => 'detect-cost-change-btn',
    '#type' => 'submit',
    '#value' => t('Save delivery details'),
    '#attributes' => array(
      'class' => array('av-ajax-trigger', 'uk-button-primary'),
    ),
    '#ajax' => array(
      'callback' => 'avtrans_transaction_form_ajax',
      //'wrapper' => 'avtrans-gr-cost-changed-wrapper',
      'wrapper' => 'avtrans-transaction-form-wrapper',
      'effect' => 'none',
      'event' => 'click',
    ),
  );
  $form['buttons']['cancel'] = array(
    '#markup' => l('Cancel', 'av/transactions'),
  );
}
