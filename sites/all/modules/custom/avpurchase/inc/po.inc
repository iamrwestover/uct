<?php
module_load_include('inc', 'avpurchase', 'inc/item_list.form');

/**
 * Load po form.
 * @param object $row
 *    PO row object.
 *
 * @return string
 */
function avpurchase_po_form_load($row) {
  return drupal_get_form('avpurchase_po_form', $row);
}

/**
 * PO form.
 *
 * @param $form
 * @param $form_state
 * @param $row
 * @return array
 */
function avpurchase_po_form($form, &$form_state, $row) {
  //avpurchase_po_send_to_email(1);
  $form['id'] = array('#type' => 'value', '#value' => isset($row->id) ? $row->id : NULL);
  switch (arg(4)) {
    case 'view':
      $page_title = t('Purchase order details');
      break;
    case 'edit':
      $page_title = t('Edit purchase order');
      break;
    default:
      $page_title = t('New purchase order');
      break;
  }
  drupal_set_title($page_title);

  // Set crud settings.
  avpurchase_po_form_settings($form, $row);

  // Info group.
  avpurchase_item_list_form_fields($form, $form_state, $row);

  // Product list.
  avpurchase_item_list_product_fields($form, $form_state, $row);

  $form['header_total'] = array(
    '#markup' => '<div><div class="uk-text-bold uk-text-uppercase">AMOUNT</div><div><h1 class="product-form-grand-total">0.00</h1></div></div>',
  );
  $form['footer_total'] = array(
    '#markup' => '<div><div class="uk-text-bold uk-text-uppercase">AMOUNT</div><div><h1 class="product-form-grand-total">0.00</h1></div></div>',
  );

  $form['item_list_date']['#title'] = t('P.O. Date');
  $form['message'] = array(
    '#type' => 'textarea',
    '#title' => 'Message to vendor',
    '#default_value' => isset($row->message) ? check_plain($row->message) : '',
    '#maxlength' => 65000,
  );
  $form['address'] = array(
    '#type' => 'item',
    '#title' => 'Shipping address',
    '#markup' => nl2br(check_plain(variable_get('av_company_address', ''))),
    //'#maxlength' => 65000,
  );

  $form['buttons']['#theme'] = 'avbase_crud_button_group';
  $form['buttons']['submit_and_send'] = array(
    '#id' => 'submit_and_send',
    '#type' => 'submit',
    '#value' => t('Save and send'),
  );
  $form['buttons']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  $form['buttons']['cancel'] = array(
    '#markup' => l('Cancel', 'av/purchase'),
  );

  // Convert form to non-edit mode if we are on View page.
  avbase_allow_form_view_mode($form);
  return $form;
}

/**
 * Set crud form settings.
 * @param $form
 */
function avpurchase_po_form_settings(&$form, $row) {
  $table_name = 'avtbl_po';

  // Set flag if this is view mode.
  $form['#av_view_mode'] = (arg(4) == 'view');

  // Set generic submit handler.
  $form['#submit'] = array('avbase_crud_form_submit', 'avpurchase_po_form_submit');

  $form['table_name'] = array('#type' => 'value', '#value' => $table_name);
  $form['entity_name'] = array('#type' => 'value', '#value' => 'po');
  $form['#theme'] = 'avpurchase_item_list_form';

  $po_js_settings = array('avPurchasePODetails' => $row);
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
    'message',
    'po_date',
    'email',
    'term_id' => array('type' => 'int'),
    'discount_type' => array('type' => 'int'),
    'discount_value' => array('type' => 'float'),
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
 * Perform actions depending on the kind of change that will be made
 * on the database for POs.
 * @param $op
 * @param $row
 * @param $form_state
 * @return object|NULL
 */
function avpurchase_avbase_crud_dbchange_po($op, $row, $form_state) {
  switch ($op) {
    case 'pre-save':
      $row['po_date'] = time();
      return $row;
      break;

    case 'after-save':
      $values = empty($form_state['values']) ? array() : $form_state['values'];
      $product_rows = empty($values['product_rows']) ? array() : $values['product_rows'];

      // Save po items.
      $query = db_insert('avtbl_po_items')->fields(array('po_id', 'product_id', 'uom_id', 'qty_per_uom', 'qty', 'cost'));
      $insert_rows = array();
      foreach ($product_rows as $prod_row) {
        if (empty($prod_row['product_id'])) {
          continue;
        }
        $prod_row['po_id'] = $row['id'];
        $prod_row['cost'] = $prod_row['price'];
        $query->values($prod_row);
      }
      $query->execute();
      break;
  }
  return NULL;
}

/**
 * Extra submit handler for purchase order form.
 * @param $form
 * @param $form_state
 */
function avpurchase_po_form_submit($form, &$form_state) {
  $triggering_element = isset($form_state['triggering_element']) ? $form_state['triggering_element'] : array();
  switch ($triggering_element['#id']) {
    case 'submit_and_send':
      $po_id = empty($form_state['values']['id']) ? NULL : $form_state['values']['id'];
      //$mail_to = empty($form_state['values']['email']) ? '' : $form_state['values']['email'];
      avpurchase_po_send_to_email($po_id);
      break;
  }
}
