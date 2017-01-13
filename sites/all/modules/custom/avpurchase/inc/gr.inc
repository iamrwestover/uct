<?php
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

  //$form['default'] = array(
  //  '#type' => 'container',
  //  '#attributes' => array(
  //    'id' => 'gr-form-default',
  //  ),
  //);
  $form = avpurchase_gr_form_default($form, $form_state, $row);
  $form['cost_changes'] = avpurchase_gr_form_cost_changes_fields($form_state, $row);
  $form['cost_changes']['#type'] = 'container';
  $form['cost_changes']['#attributes']['id'] = 'avpurchase-gr-cost-changed-wrapper';
  $form['cost_changes']['#theme'] = 'avbase_modal_form';
  $form['cost_changes']['#modal_options'] = array(
    'modal_id' => 'cost-changed',
    'modal_title' => t('Product cost changes'),
    'modal_classes' => array('uk-modal-dialog-large'),
  );

  //switch ($form_page) {
  //  case 'default':
  //    return avpurchase_gr_form_default_fields($form, $form_state, $row);
  //    break;
  //  case 'cost_changes':
  //    return avpurchase_gr_form_cost_changes_fields($form, $form_state, $row);
  //    //return $form;
  //    break;
  //}

//dpm($form);
  // Convert form to non-edit mode if we are on View page.
  //avbase_allow_form_view_mode($form);
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

  // Info group.
  avpurchase_item_list_form_fields($form, $form_state, $row);

  // Product list.
  avpurchase_item_list_product_fields($form, $form_state, $row);

  $form['buttons']['#theme'] = 'avbase_crud_button_group';
  $form['buttons']['submit'] = array(
    '#id' => 'submit-btn',
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

  //$form['#suffix'] = theme('avbase_modal', array('modal_id' => 'cost-changed', 'modal_title' => t('Product cost changes'), 'modal_classes' => array('uk-modal-dialog-large')));
  return $form;
}

/**
 * Product Cost Changes form fields.
 * @param $form_state
 * @param $row
 *
 * @return mixed
 */
function avpurchase_gr_form_cost_changes_fields(&$form_state, $row) {
  $fields = array();
  //$fields['x11111'] = array('#type' => 'textfield', '#title' => 'Sana akoy mapagod na.', '#required' => TRUE);
  //dpm($form_state);
  $fields['buttons']['submit'] = array(
    '#id' => 'cost-changed-submit-btn',
    '#type' => 'submit',
    '#value' => t('Continue'),
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
  $fields['buttons']['cancel'] = array(
    '#id' => 'cost-changed-cancel-btn',
    '#type' => 'submit',
    '#submit' => array('avpurchase_gr_form_ajax_submit'),
    '#value' => t('Cancel'),
    //'#validate' => array('avx'),
    '#attributes' => array(
      'class' => array('uk-modal-close'),
    ),
    //'#ajax' => array(
    //  'callback' => 'avpurchase_gr_form_ajax',
    //  'wrapper' => 'avpurchase-gr-form-wrapper',
    //  'effect' => 'none',
    //  'event' => 'click',
    //),
  );
  return $fields;
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
    'gr_date',
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
  //$terms = avbase_get_categories('payment_term');
  //$terms_js_settings = array('avbasePaymentTerms' => $terms);
  //$form['term_id']['#attached']['js'] = array(
  //  AVVENDORS_MODULE_PATH . '/js/payment_terms.js',
  //  array(
  //    'data' => $terms_js_settings,
  //    'type' => 'setting',
  //  ),
  //);
}

/**
 * Perform actions depending on the kind of change that will be made
 * on the database for Goods received.
 * @param $op
 * @param $row
 * @param $form_state
 * @return object|NULL
 */
function avpurchase_avbase_crud_dbchange_gr($op, $row, $form_state) {
  switch ($op) {
    case 'pre-save':
      $row['gr_date'] = time();
      return $row;
      break;

    case 'after-save':
      $values = empty($form_state['values']) ? array() : $form_state['values'];
      $product_rows = empty($values['product_rows']) ? array() : $values['product_rows'];
      $incoming_qty = array();

      // Save po items.
      $query = db_insert('avtbl_gr_items')->fields(array('gr_id', 'product_id', 'uom_id', 'qty_per_uom', 'qty', 'cost'));
      foreach ($product_rows as $prod_row) {
        if (empty($prod_row['product_id'])) {
          continue;
        }
        $prod_row['gr_id'] = $row['id'];
        $prod_row['cost'] = $prod_row['price'];
        $query->values($prod_row);

        $this_prod_incoming_qty = empty($incoming_qty[$prod_row['product_id']]) ? 0 : $incoming_qty[$prod_row['product_id']];
        $incoming_qty[$prod_row['product_id']] = $this_prod_incoming_qty + ($prod_row['qty'] * $prod_row['qty_per_uom']);
      }
      $query->execute();

      if (!empty($incoming_qty)) {
        $query = "UPDATE {avtbl_products} SET qty = qty + CASE id";
        foreach ($incoming_qty as $k => $v) {
          $query .= " WHEN $k THEN $v";
        }
        $query .= " END WHERE id IN (" . implode(', ', array_keys($incoming_qty)) . ")";
        if (db_query($query)) {
          avbase_custom_watchdog("gr__qty_update", "ID: {$row['id']}", array('#row' => $row, '#incoming_qty' => $incoming_qty, '#entity_name' => 'gr'));
        }
      }
      break;
  }
  return NULL;
}

/**
 * Ajax callback before submitting a Receive Items form.
 */
function avpurchase_gr_form_ajax($form, $form_state) {
  return $form;
}

/**
 * Submit handler for Goods Received form.
 * @param $form
 * @param $form_state
 */
function avpurchase_gr_form_submit($form, &$form_state) {
  $trigger = $form_state['triggering_element'];
  dpm($trigger);
  if (0) {
  }
  else {
    avbase_crud_form_submit($form, $form_state);
    unset($form_state['input']);
  }
  $form_state['rebuild'] = TRUE;
}
