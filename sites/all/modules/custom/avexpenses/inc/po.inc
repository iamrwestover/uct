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
  $form['address'] = array(
    '#type' => 'textarea',
    '#title' => 'Shipping address',
    '#default_value' => isset($row->address) ? check_plain($row->address) : '',
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

  $form['product_table'] = array(
    '#theme' => 'avbase_form_table',
    '#header' => array(
      array('data' => '#'),
      array('data' => 'Product'),
      array('data' => 'Description'),
      array('data' => 'Qty.'),
      array('data' => 'Price'),
      array('data' => 'Amount'),
      '&nbsp;',
    ),
    'product_rows' => array('#tree' => TRUE),
  );
  $prod_index = -1;
  foreach ($prod_rows as $prod_key => $prod_row) {
    $prod_index++;
    $table_row = array();

    $table_row[] = array(
      'row_id' => array(
        '#markup' => 'a',
      )
    );

    $table_row[] = array(
      'prod_id' => array(
        '#id' => 'prod-id-' . $prod_index,
        '#type' => 'textfield',
        '#title' => 'Product',
        '#title_display' => 'invisible',
        //'#description' => t('@uom1 per @uom2', array('@uom1' => $plural_form, '@uom2' => $current_uom_name)),
        //'#default_value' => isset($prod_row['qty']) ? $prod_row['qty'] : '',
        '#maxlength' => 255,
        //'#element_validate' => array('element_validate_integer_positive'),
        '#required' => TRUE,
        '#attributes' => array(
          //'data-prod-index' => $prod_index,
          'class' => array('prod-group-id'),
        ),
      )
    );

    $table_row[] = array(
      'description' => array(
        '#id' => 'prod-desc-' . $prod_index,
        '#type' => 'textfield',
        '#title' => 'Description',
        '#title_display' => 'invisible',
        //'#description' => t('@uom1 per @uom2', array('@uom1' => $plural_form, '@uom2' => $current_uom_name)),
        //'#default_value' => isset($prod_row['qty']) ? $prod_row['qty'] : '',
        '#maxlength' => 255,
        //'#element_validate' => array('element_validate_integer_positive'),
        '#required' => TRUE,
        '#attributes' => array(
          //'data-prod-index' => $prod_index,
          'class' => array('prod-group-id'),
        ),
      )
    );

    $table_row[] = array(
      'qty' => array(
        '#id' => 'prod-qty-' . $prod_index,
        '#type' => 'textfield',
        '#title' => 'Qty.',
        '#title_display' => 'invisible',
        //'#description' => t('@uom1 per @uom2', array('@uom1' => $plural_form, '@uom2' => $current_uom_name)),
        //'#default_value' => isset($prod_row['qty']) ? $prod_row['qty'] : '',
        '#maxlength' => 10,
        '#element_validate' => array('element_validate_integer_positive'),
        '#required' => TRUE,
        '#attributes' => array(
          //'data-prod-index' => $prod_index,
          'class' => array('prod-group-qty'),
        ),
      )
    );

    $table_row[] = array(
      'price' => array(
        '#id' => 'prod-price-' . $prod_index,
        '#type' => 'textfield',
        '#title' => 'Price',
        '#title_display' => 'invisible',
        //'#default_value' => isset($prod_row['qty']) ? $prod_row['qty'] : '',
        '#maxlength' => 19,
        //'#element_validate' => array('element_validate_integer_positive'),
        '#required' => TRUE,
        '#attributes' => array(
          //'data-prod-index' => $prod_index,
          'class' => array('prod-group-price'),
        ),
      )
    );

    $table_row[] = array(
      'amount' => array(
        '#id' => 'prod-amt-' . $prod_index,
        '#type' => 'textfield',
        '#title' => 'Amount',
        '#title_display' => 'invisible',
        //'#default_value' => isset($prod_row['qty']) ? $prod_row['qty'] : '',
        '#maxlength' => 19,
        //'#element_validate' => array('element_validate_integer_positive'),
        '#required' => TRUE,
        '#attributes' => array(
          //'data-prod-index' => $prod_index,
          'class' => array('prod-group-amt'),
        ),
      ),
      //'#column_attributes' => array(
      //  'colspan' => 3,
      //  'class' => array('a', 'b'),
      //),
    );



    $form['product_table']['product_rows'][$prod_key] = $table_row;
    //$form['product_table']['product_rows'][$prod_key]['#row_attributes'] = array(
    //  'class' => array('e', 'd'),
    //  'style' => 'background: red;',
    //);
    //if (count($prod_rows) == ($prod_index + 1)) {
    //  $form['products'][$prod_key]['#suffix'] = '<div id="po-new-product-wrapper"></div>';
    //}
  }

  $form['prod_add_btn'] = array(
    '#id' => 'prod-add-btn',
    '#type' => 'submit',
    '#value' => t('Add new'),
    '#limit_validation_errors' => array(),
    '#submit' => array('avexpenses_po_prod_add_submit'),
    '#ajax' => array(
      'callback' => 'avexpenses_po_prod_add_js',
      'wrapper' => 'po-new-product-wrapper',
      'effect' => 'none',
    ),
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
  $button = $form_state['triggering_element'];
  $product_elements = $form['product_table']['product_rows'];

  $keys = element_children($product_elements);
  $last_key = array_pop($keys);
  $last_element = empty($product_elements[$last_key]) ? array() : $product_elements[$last_key];
  switch ($button['#id']) {
    case 'prod-add-btn':
      //$uom_count = $form_state['uoms']['uom_count'];
      $attributes = empty($last_element['#row_attributes']) ? array() : $last_element['#row_attributes'];
      $last_element['#prefix'] = '<tr' . drupal_attributes($attributes) . '>' . (isset($last_element['#prefix']) ? $last_element['#prefix'] : '');
      $last_element['#suffix'] = (isset($last_element['#suffix']) ? $last_element['#suffix'] : '') . '</tr><tr id="po-new-product-wrapper"><td colspan="10">&nbsp;</td></tr>';
      foreach (element_children($last_element) as $key) {
        $attributes = empty($last_element[$key]['#column_attributes']) ? array() : $last_element[$key]['#column_attributes'];
        $last_element[$key]['#prefix'] = '<td' . drupal_attributes($attributes) . '>' . (isset($last_element[$key]['#prefix']) ? $last_element[$key]['#prefix'] : '');
        $last_element[$key]['#suffix'] = (isset($last_element[$key]['#suffix']) ? $last_element[$key]['#suffix'] : '') . '</td>';
      }
      break;
  }
  return $last_element;
}
