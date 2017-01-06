<?php

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
  avpurchase_po_form_fields($form, $form_state, $row);

  // Product list.
  avpurchase_po_product_fields($form, $form_state, $row);

  $form['header_total'] = array(
    '#markup' => '<div><div class="uk-text-bold uk-text-uppercase">AMOUNT</div><div><h1 class="product-form-grand-total">0.00</h1></div></div>',
  );
  $form['footer_total'] = array(
    '#markup' => '<div><div class="uk-text-bold uk-text-uppercase">AMOUNT</div><div><h1 class="product-form-grand-total">0.00</h1></div></div>',
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

  $po_js_settings = array('avpurchasePODetails' => $row);
  $form['#attached']['js'] = array(
    AVPURCHASE_MODULE_PATH . '/js/po_form.js',
    array(
      'data' => $po_js_settings,
      'type' => 'setting',
    ),
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
 * Product form - generate info fields.
 * @param $form
 * @param $form_state
 * @param $row
 */
function avpurchase_po_form_fields(&$form, &$form_state, $row) {
  //dpm($row);
  $form['vendor_id'] = array(
    '#id' => 'vendor-id',
    '#type' => 'textfield',
    '#title' => t('Vendor'),
    '#maxlength' => 255,
    '#autocomplete_path' => 'av/vendors/autocomplete',
    '#required' => TRUE,
    '#element_validate' => array('avbase_element_validate_display_name_exists'),
    '#table_name' => 'avtbl_vendors',
    '#default_value' => empty($row->vendor_name) ? '' : check_plain($row->vendor_name),
  );
  $form['vendor_id']['#attached']['js'] = array(
    AVVENDORS_MODULE_PATH . '/js/autocomplete.js',
  );
  $form['email'] = array(
    '#id' => 'vendor-email',
    '#type' => 'textfield',
    '#title' => 'E-mail address',
    '#default_value' => isset($row->email) ? check_plain($row->email) : '',
    '#maxlength' => 255,
    '#element_validate' => array('avbase_element_validate_email'),
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
  $form['discount_type'] = array(
    '#id' => 'discount-type',
    '#title_display' => 'invisible',
    '#type' => 'select',
    '#title' => 'Discount type',
    '#options' => avbase_get_discount_types(),
    '#default_value' => isset($row->discount_type) ? $row->discount_type : AVBASE_DISCOUNT_PERCENT,
  );
  $form['discount_value'] = array(
    '#id' => 'discount-value',
    '#title_display' => 'invisible',
    '#type' => 'textfield',
    '#title' => 'Discount value',
    '#default_value' => isset($row->discount_value) ? round($row->discount_value, 2) : '',
    '#maxlength' => 19,
    '#element_validate' => array('element_validate_number'),
    //'#special_element_type' => 'number',
    '#attributes' => array(
      //'class' => array(''),
      //'step' => 0.01,
    ),
  );
  if (!empty($form['#av_view_mode']) && $form['discount_type']['#default_value'] == AVBASE_DISCOUNT_PERCENT) {
    $form['discount_value']['#field_suffix'] = '%';
    $form['discount_type']['#default_value'] = '';
  }

  $form['po_date'] = array(
    '#type' => 'item',
    '#title' => 'P.O. Date',
    '#markup' => format_date(time(), 'custom', 'F d, Y'),
  );
  $form['address'] = array(
    '#type' => 'item',
    '#title' => 'Shipping address',
    '#markup' => nl2br(check_plain(variable_get('av_company_address', ''))),
    //'#maxlength' => 65000,
  );
  $form['message'] = array(
    '#type' => 'textarea',
    '#title' => 'Message to vendor',
    '#default_value' => isset($row->message) ? check_plain($row->message) : '',
    '#maxlength' => 65000,
  );
}

/**
 * Product form - generate product list fields.
 * @param $form
 * @param $form_state
 * @param $row
 */
function avpurchase_po_product_fields(&$form, &$form_state, $row) {
  $form['#attached']['js'][] = AVPURCHASE_MODULE_PATH . '/js/nestable_product_form.js';

  // Set extra form validation for product list.
  $form['#validate'][] = 'avpurchase_po_product_fields_validate';
  // Get user input.
  $user_input = isset($form_state['input']) ? $form_state['input'] : array();
  // Get saved product rows.
  $db_row_products = empty($row->products) ? array() : $row->products;
  $prod_rows = isset($user_input['product_rows']) ? $user_input['product_rows'] : $db_row_products;

  // Pre-load empty product rows.
  if (empty($prod_rows)) {
    for ($x = 0; $x < 3; $x++) {
      $prod_rows[] = array();
    }
  }

  $triggering_button = isset($form_state['triggering_element']) ? $form_state['triggering_element'] : array();
  if ($triggering_button) {
    switch ($triggering_button['#name']) {
      case 'prod_add_btn':
        $prod_rows = empty($user_input['product_rows']) ? array() : $user_input['product_rows'];
        $prod_rows[] = array();
        break;
      default:
        $remove_prod_key = isset($triggering_button['#remove_prod_key']) ? $triggering_button['#remove_prod_key'] : NULL;
        unset($prod_rows[$remove_prod_key]);
        break;
    }
  }
  //dpm($user_input);
  //dpm($remove_prod_key);
  //dpm($prod_rows);

  $form['product_rows'] = array(
    '#tree' => TRUE,
    '#theme' => 'avbase_nestable_form',
  );
  $prod_index = -1;
  foreach ($prod_rows as $prod_key => $prod_row) {
    $prod_index++;
    $fields = array();
    $fields['product_id'] = array(
      '#id' => 'product-id-' . $prod_key,
      '#type' => 'textfield',
      '#title' => 'Product',
      '#maxlength' => 255,
      //'#default_value' => $prod_key,
      '#autocomplete_path' => 'av/products/autocomplete',
      '#element_validate' => array('avbase_element_validate_product_name_exists'),
      '#attributes' => array(
        'data-column-name' => 'id',
        'class' => array('prod-column-id'),
      ),
    );

    $fields['uom_id'] = array(
      '#id' => 'uom-id-' . $prod_key,
      '#type' => 'select',
      '#title' => 'UOM',
      '#options' => avbase_get_category_options_array('uom'),
      '#default_value' => 0,
      '#empty_value' => 0,
      '#empty_option' => '',
      '#attributes' => array(
        'class' => array('prod-column-uom-id'),
      ),
      //'#required' => TRUE,
    );

    $fields['qty'] = array(
      '#id' => 'product-qty-' . $prod_key,
      '#type' => 'textfield',
      '#title' => 'Qty.',
      //'#title_display' => 'invisible',
      //'#description' => t('@uom1 per @uom2', array('@uom1' => $plural_form, '@uom2' => $current_uom_name)),
      //'#default_value' => isset($prod_row['qty']) ? $prod_row['qty'] : '',
      '#maxlength' => 10,
      '#element_validate' => array('avbase_element_validate_po_element'),
      '#av_prod_element_validate' => array('element_validate_integer_positive'),
      //'#av_prod_element_required' => TRUE,
      //'#required' => TRUE,
      '#attributes' => array(
        'data-column-name' => 'qty',
        'class' => array('prod-column-qty'),
      ),
    );

    $fields['price'] = array(
      '#id' => 'product-price-' . $prod_key,
      '#type' => 'textfield',
      '#title' => 'Price',
      //'#title_display' => 'invisible',
      //'#default_value' => isset($prod_row['qty']) ? $prod_row['qty'] : '',
      '#maxlength' => 19,
      '#element_validate' => array('avbase_element_validate_po_element'),
      '#av_prod_element_validate' => array('element_validate_number'),
      //'#required' => TRUE,
      '#attributes' => array(
        'data-column-name' => 'price',
        'class' => array('prod-column-price'),
      ),
    );

    $fields['amount'] = array(
      '#id' => 'product-amt-' . $prod_key,
      '#type' => 'textfield',
      '#title' => 'Amount',
      //'#title_display' => 'invisible',
      //'#default_value' => isset($prod_row['qty']) ? $prod_row['qty'] : '',
      '#maxlength' => 19,
      '#element_validate' => array('avbase_element_validate_po_element'),
      '#av_prod_element_validate' => array('element_validate_number'),
      //'#required' => TRUE,
      '#attributes' => array(
        'data-column-name' => 'amt',
        'class' => array('prod-column-amt'),
      ),
    );

    $fields['prod_delete_btn'] = array(
      '#name' => 'prod_delete_btn_' . $prod_key,
      '#remove_prod_key' => $prod_key,
      '#type' => 'submit',
      '#value' => t('Delete'),
      '#limit_validation_errors' => array(),
      '#submit' => array('avpurchase_po_prod_submit'),
      '#attributes' => array(
        'class' => array('prod-column-delete-btn'),
        'tabindex' => -1,
      ),
      '#ajax' => array(
        'callback' => 'avpurchase_po_prod_js',
        'wrapper' => 'po-product-row-' . $prod_key . '-wrapper',
        'effect' => 'none',
      ),
    );



    $form['product_rows'][$prod_key] = $fields;
    $form['product_rows'][$prod_key]['#prod_index'] = $prod_index;
    $form['product_rows'][$prod_key]['#theme'] = 'avbase_nestable_form_row';
    $form['product_rows'][$prod_key]['#prefix'] = '<div id="po-product-row-' . $prod_key . '-wrapper" class="uk-nestable-item">';
    $form['product_rows'][$prod_key]['#suffix'] = '</div>';
    //if (count($prod_rows) == ($prod_index + 1)) {
    //  $form['product_rows'][$prod_key]['#suffix'] .= '<div id="po-new-product-wrapper"></div>';
    //}
  }

  $form['prod_add_btn'] = array(
    '#id' => 'prod-add-btn',
    '#name' => 'prod_add_btn',
    '#type' => 'submit',
    '#value' => t('Add new row'),
    '#limit_validation_errors' => array(),
    '#submit' => array('avpurchase_po_prod_submit'),
    '#ajax' => array(
      'callback' => 'avpurchase_po_prod_js',
      'wrapper' => 'po-new-product-wrapper',
      'effect' => 'none',
    ),
  );


}

/**
 * Submit handler when adding or removing products on a PO form.
 */
function avpurchase_po_prod_submit($form, &$form_state) {
  //dpm($form);
  //$button = $form_state['triggering_element'];
  //switch ($button['#name']) {
  //  case 'prod_add_more':
  //    //$form_state['uoms']['uom_count']++;
  //    break;
  //}
  //$button = $form_state['triggering_element'];
  //$product_elements = $form['product_rows'];
  //$keys = element_children($product_elements);
  //$last_key = array_pop($keys);
  //$last_element = empty($product_elements[$last_key]) ? array() : $product_elements[$last_key];
  //$output = $last_element;
  //dpm($button);
  $form_state['rebuild'] = TRUE;
}

/**
 * Ajax callback  when adding products or removing on a PO form.
 */
function avpurchase_po_prod_js($form, $form_state) {
  $button = $form_state['triggering_element'];
  $output = array();
  switch ($button['#name']) {
    case 'prod_add_btn':
      $product_elements = $form['product_rows'];
      $keys = element_children($product_elements);
      $last_key = array_pop($keys);
      $last_element = empty($product_elements[$last_key]) ? array() : $product_elements[$last_key];
      $last_element['#suffix'] .= '<div id="po-new-product-wrapper"></div>';
      $js_settings = array('avNestableProductForm' => arraY('ajaxAction' => 'add'));
      $last_element['#attached']['js'] = array(
        array(
          'data' => $js_settings,
          'type' => 'setting',
        ),
      );

      $output = $last_element;
      break;
    default:
      $js_settings = array('avNestableProductForm' => arraY('ajaxAction' => 'remove'));
      $output['#attached']['js'] = array(
        array(
          'data' => $js_settings,
          'type' => 'setting',
        ),
      );
      break;
  }

  return $output;
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
      $query = db_insert('avtbl_po_items')->fields(array('po_id', 'product_id', 'uom_id', 'qty', 'cost'));
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
