<?php
module_load_include('inc', 'avpurchase', 'inc/gr.handlers');
module_load_include('inc', 'avpurchase', 'inc/gr.form.validate');
module_load_include('inc', 'avpurchase', 'inc/gr.form.ch');
module_load_include('inc', 'avpurchase', 'inc/gr.form.vp');
module_load_include('inc', 'avpurchase', 'inc/item_list.form');

/**
 * Load "Receive items" form.
 * @param object $row
 *    RI row object.
 *
 * @return string
 */
function avpurchase_gr_form_load($row) {
  return drupal_get_form('avpurchase_gr_form', $row);
}

/**
 * "Receive items" form.
 *
 * @param $form
 * @param $form_state
 * @param $row
 * @return array
 */
function avpurchase_gr_form($form, &$form_state, $row) {
  $form['#id'] = 'avpurchase-gr-form';
  $form['#prefix'] = '<div id="avpurchase-gr-form-wrapper"> <div id="avpurchase-gr-form-wrapper2"></div>';
  $form['#suffix'] = '</div>';

  // Set crud settings.
  avpurchase_gr_form_settings($form, $row);

  // Add generic Goods Received form fields.
  $form = avpurchase_gr_form_default($form, $form_state, $row);

  // Add fields for Cost Changes popup.
  avpurchase_gr_form_cost_changes_fields($form, $form_state, $row);

  // Add fields for Vendor POs popup.
  avpurchase_gr_form_vendor_pos_fields($form, $form_state, $row);

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
function avpurchase_gr_form_default(&$form, &$form_state, $row) {
  $form['id'] = array('#type' => 'value', '#value' => isset($row->id) ? $row->id : NULL);
  switch (arg(4)) {
    case 'view':
      $page_title = t('Received item details');
      break;
    case 'edit':
      $page_title = t('Edit purchase order');
      break;
    default:
      $page_title = t('Receive items');
      break;
  }
  drupal_set_title($page_title);

  $form['#theme'] = 'avpurchase_item_list_form';

  // Goods Received fields.
  avpurchase_gr_form_fields($form, $form_state, $row);

  // Item List form fields.
  avpurchase_item_list_form_fields($form, $form_state, $row);

  // Buttons.
  avpurchase_gr_form_buttons($form, $form_state, $row);

  return $form;
}

/**
 * Set crud form settings.
 * @param $form
 */
function avpurchase_gr_form_settings(&$form, $row) {
  $table_name = 'avtbl_gr';

  // Set flag if this is view mode.
  $form['#av_view_mode'] = (arg(4) == 'view');

  // Set generic submit handler.
  //$form['#submit'] = array('avbase_crud_form_submit');

  $form['table_name'] = array('#type' => 'value', '#value' => $table_name);
  $form['entity_name'] = array('#type' => 'value', '#value' => 'gr');
  $form['#success_message'] = t('Item receipt details saved.');

  $po_js_settings = array('avpurchaseRIDetails' => $row);
  $form['#attached']['js'][] = AVPURCHASE_MODULE_PATH . '/js/gr_form.js';
  $form['#attached']['js'][] = array(
    'data' => $po_js_settings,
    'type' => 'setting',
  );
  $form['#attached']['css'] = array(
    //AVBASE_LIBRARIES_PATH . '/select2/select2.min.css',
  );

  // Set autocomplete off.
  $form['#attributes']['autocomplete'] = 'off';

  // Set field names to save.
  $save_field_ids = array(
    'created',
    'vendor_id',
    'term_id' => array('type' => 'int'),
  );
  if (empty($form['id']['#value'])) {
    $save_field_ids[] = 'gr_date';
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

/**
 * Generic GR form fields.
 * @param $form
 * @param $form_state
 * @param $row
 */
function avpurchase_gr_form_fields(&$form, &$form_state, $row) {
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
function avpurchase_gr_form_buttons(&$form, $form_state, $row) {
  $form['buttons']['#theme'] = 'avbase_crud_button_group';
  $form['buttons']['submit'] = array(
    '#id' => 'detect-cost-change-btn',
    '#type' => 'submit',
    '#value' => t('Receive these items'),
    '#attributes' => array(
      'class' => array('av-ajax-trigger', 'uk-button-primary'),
    ),
    '#ajax' => array(
      'callback' => 'avpurchase_gr_form_ajax',
      //'wrapper' => 'avpurchase-gr-cost-changed-wrapper',
      'wrapper' => 'avpurchase-gr-form-wrapper',
      'effect' => 'none',
      'event' => 'click',
    ),
  );
  $form['buttons']['cancel'] = array(
    '#markup' => l('Cancel', 'av/purchase'),
  );
}
