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

  // Set crud settings.
  avpurchase_gr_form_settings($form, $row);

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
  );
  $terms = avbase_get_categories('payment_term');
  $terms_js_settings = array('avbasePaymentTerms' => $terms);
  $form['term_id']['#attached']['js'] = array(
    AVVENDORS_MODULE_PATH . '/js/payment_terms.js',
    array(
      'data' => $terms_js_settings,
      'type' => 'setting',
    ),
  );

  $form['buttons']['#theme'] = 'avbase_crud_button_group';
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
function avpurchase_gr_form_settings(&$form, $row) {
  $table_name = 'avtbl_gr';

  // Set flag if this is view mode.
  $form['#av_view_mode'] = (arg(4) == 'view');

  // Set generic submit handler.
  $form['#submit'] = array('avbase_crud_form_submit');

  $form['table_name'] = array('#type' => 'value', '#value' => $table_name);
  $form['entity_name'] = array('#type' => 'value', '#value' => 'gr');
  $form['#theme'] = 'avpurchase_item_list_form';

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
