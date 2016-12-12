<?php

/**
 * Load po form.
 * @param object $row
 *    PO row object.
 *
 * @return string
 */
function avexpenses_po_form_load($row) {
  return drupal_get_form('avexpenses_po_form', $row);
}

/**
 * PO form.
 *
 * @param $form
 * @param $form_state
 * @param $row
 * @return array
 */
function avexpenses_po_form($form, &$form_state, $row) {
  $form['#attached']['js'] = array(
    AVEXPENSES_MODULE_PATH . '/js/po_form.js',
  );
  $form['#attached']['css'] = array(
    //AVBASE_LIBRARIES_PATH . '/select2/select2.min.css',
  );

  $form['id'] = array('#type' => 'value', '#value' => isset($row->id) ? $row->id : NULL);
  $page_title = empty($form['id']['#value']) ? 'New Purchase Order' : 'Edit Purchase Order';
  drupal_set_title($page_title);

  // Set crud settings.
  avexpenses_po_form_settings($form);

  // Info group.
  avexpenses_po_form_fields($form, $form_state, $row);

  // Product list.
  avexpenses_po_product_fields($form, $form_state, $row);

  $form['buttons']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  $form['buttons']['cancel'] = array(
    '#markup' => l('Cancel', 'av/expenses'),
  );
  return $form;
}

/**
 * Set crud form settings.
 * @param $form
 */
function avexpenses_po_form_settings(&$form) {
  $table_name = 'avtbl_po';

  // Set generic submit handler.
  $form['#submit'] = array('avbase_crud_form_submit');
  $form['table_name'] = array('#type' => 'value', '#value' => $table_name);
  $form['entity_name'] = array('#type' => 'value', '#value' => 'po');

  // Set autocomplete off.
  $form['#attributes']['autocomplete'] = 'off';

  // Set field names to save.
  $save_field_ids = array(
    'vendor_id',
    'message',
    'po_date',
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
function avexpenses_po_form_fields(&$form, &$form_state, $row) {
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
  $form['email'] = array(
    '#id' => 'vendor-email',
    '#type' => 'textfield',
    '#title' => 'E-mail address',
    '#default_value' => isset($row->email) ? check_plain($row->email) : '',
    '#maxlength' => 255,
    '#element_validate' => array('avbase_element_validate_email'),
  );
  $form['po_date'] = array(
    '#type' => 'item',
    '#title' => 'P.O. Date',
    '#markup' => format_date(time(), 'custom', 'F d, Y'),
  );
  $form['address'] = array(
    '#type' => 'textarea',
    '#title' => 'Shipping address',
    '#default_value' => variable_get('av_company_address', ''),
    '#maxlength' => 65000,
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
function avexpenses_po_product_fields(&$form, &$form_state, $row) {
  // Set extra form validation for product list.
  $form['#validate'][] = 'avexpenses_po_product_fields_validate';
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

  $clicked_button = isset($form_state['triggering_element']) ? $form_state['triggering_element'] : array();
  if ($clicked_button) {
    switch ($clicked_button['#id']) {
      case 'prod_delete':
        $remove_prod_key = isset($clicked_button['#remove_prod_key']) ? $clicked_button['#remove_prod_key'] : NULL;
        unset($prod_rows[$remove_prod_key]);
        break;
      case 'prod-add-btn':
        $prod_rows = empty($user_input['product_rows']) ? array() : $user_input['product_rows'];
        $prod_rows[] = array();
        break;
    }
  }

  $form['product_rows'] = array(
    '#tree' => TRUE,
    '#theme' => 'avbase_nestable_form',
  );
  $prod_index = -1;
  foreach ($prod_rows as $prod_key => $prod_row) {
    $prod_index++;
    $fields = array();
    $fields['product_id'] = array(
      '#id' => 'product-id-' . $prod_index,
      '#type' => 'textfield',
      '#title' => 'Product',
      '#maxlength' => 255,
      //'#required' => TRUE,
      '#autocomplete_path' => 'av/products/autocomplete',
      '#element_validate' => array('avbase_element_validate_product_name_exists'),
      '#attributes' => array(
        'data-row-index' => $prod_index,
        'class' => array('prod-group-id'),
      ),
    );

    $fields['code'] = array(
      '#type' => 'item',
      '#markup' => '<div id="prod-code-' . $prod_index . '"></div>',
      '#title' => 'Code',
      //'#title_display' => 'invisible',
      //'#description' => t('@uom1 per @uom2', array('@uom1' => $plural_form, '@uom2' => $current_uom_name)),
      //'#default_value' => isset($prod_row['qty']) ? $prod_row['qty'] : '',
      //'#maxlength' => 255,
      ////'#element_validate' => array('element_validate_integer_positive'),
      //'#required' => TRUE,
      //'#attributes' => array(
      //  //'data-prod-index' => $prod_index,
      //  'class' => array('prod-group-id'),
      //),
    );

    $fields['uom_id'] = array(
      '#id' => 'uom-id-' . $prod_index,
      '#type' => 'select',
      '#title' => 'UOM',
      '#options' => avbase_get_category_options_array('uom'),
      '#default_value' => 0,
      '#empty_value' => 0,
      '#empty_option' => '',
      '#attributes' => array(
        'data-prod-index' => $prod_index,
        'class' => array('prod-group-uom-id'),
      ),
      //'#required' => TRUE,
    );

    $fields['qty'] = array(
      '#id' => 'prod-qty-' . $prod_index,
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
        'data-prod-index' => $prod_index,
        'class' => array('prod-group-qty'),
      ),
    );

    $fields['price'] = array(
      '#id' => 'prod-price-' . $prod_index,
      '#type' => 'textfield',
      '#title' => 'Price',
      //'#title_display' => 'invisible',
      //'#default_value' => isset($prod_row['qty']) ? $prod_row['qty'] : '',
      '#maxlength' => 19,
      '#element_validate' => array('element_validate_number'),
      //'#required' => TRUE,
      '#attributes' => array(
        //'data-prod-index' => $prod_index,
        'class' => array('prod-group-price'),
      ),
    );

    $fields['amount'] = array(
      '#id' => 'prod-amt-' . $prod_index,
      '#type' => 'textfield',
      '#title' => 'Amount',
      //'#title_display' => 'invisible',
      //'#default_value' => isset($prod_row['qty']) ? $prod_row['qty'] : '',
      '#maxlength' => 19,
      '#element_validate' => array('element_validate_number'),
      //'#required' => TRUE,
      '#attributes' => array(
        //'data-prod-index' => $prod_index,
        'class' => array('prod-group-amt'),
      ),
    );



    $form['product_rows'][$prod_key] = $fields;
    $form['product_rows'][$prod_key]['#prod_index'] = $prod_index;
    $form['product_rows'][$prod_key]['#theme'] = 'avbase_nestable_form_row';
    if (count($prod_rows) == ($prod_index + 1)) {
      $form['product_rows'][$prod_key]['#suffix'] = '<div id="po-new-product-wrapper"></div>';
    }
  }

  $form['prod_add_btn'] = array(
    '#id' => 'prod-add-btn',
    '#type' => 'submit',
    '#value' => t('Add new row'),
    '#limit_validation_errors' => array(),
    '#submit' => array('avexpenses_po_prod_add_submit'),
    '#ajax' => array(
      'callback' => 'avexpenses_po_prod_add_js',
      'wrapper' => 'po-new-product-wrapper',
      'effect' => 'none',
    ),
  );
  $form['header_total'] = array(
    '#markup' => '<div><div class="uk-text-bold uk-text-uppercase">AMOUNT</div><div><h1>0.00</h1></div></div>',
  );
}

/**
 * Submit handler when adding products on a PO form.
 */
function avexpenses_po_prod_add_submit($form, &$form_state) {
  //dpm($form);
  //$button = $form_state['triggering_element'];
  //switch ($button['#name']) {
  //  case 'prod_add_more':
  //    //$form_state['uoms']['uom_count']++;
  //    break;
  //}

  $form_state['rebuild'] = TRUE;
}

/**
 * Ajax callback  when adding products on a PO form.
 */
function avexpenses_po_prod_add_js($form, $form_state) {
  //$button = $form_state['triggering_element'];
  $product_elements = $form['product_rows'];

  $keys = element_children($product_elements);
  $last_key = array_pop($keys);
  $last_element = empty($product_elements[$last_key]) ? array() : $product_elements[$last_key];
  //switch ($button['#id']) {
  //  case 'prod-add-btn':
  //    $last_element['#prefix'] = '<div  class="uk-nestable-item">' . (isset($last_element['#prefix']) ? $last_element['#prefix'] : '');
  //    $last_element['#suffix'] = '</div>' . (isset($last_element['#suffix']) ? $last_element['#suffix'] : '') ;
  //    break;
  //}
  return $last_element;
}
