<?php

/**
 * Item List form - generate info fields.
 * @param $form
 * @param $form_state
 * @param $row
 */
function avpurchase_item_list_form_fields(&$form, &$form_state, $row) {
  //dpm($row);
  $form['#attached']['js'][] = AVPURCHASE_MODULE_PATH . '/js/item_list_form.js';
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

  $form['item_list_date'] = array(
    '#type' => 'item',
    '#title' => 'Date',
    '#markup' => format_date(time(), 'custom', 'F d, Y'),
  );


}

/**
 * Item List form - generate product list fields.
 * @param $form
 * @param $form_state
 * @param $row
 */
function avpurchase_item_list_product_fields(&$form, &$form_state, $row) {
  $js_settings = array('avBase' => array('uoms' => avbase_get_categories('uom')));
  $form['#attached']['js'][] = AVPURCHASE_MODULE_PATH . '/js/nestable_product_form.js';
  $form['#attached']['js'][] = array(
    'data' => $js_settings,
    'type' => 'setting',
  );

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
      //'#limit_validation_errors' => array(),
      //'#submit' => array('avpurchase_item_list_prod_submit'),
      //'#ajax' => array(
      //  'callback' => 'avpurchase_item_list_prod_js',
      //  'wrapper' => 'uom-id-' . $prod_key,
      //  'effect' => 'none',
      //),
    );

    $fields['uom_id'] = array(
      '#id' => 'uom-id-' . $prod_key,
      '#type' => 'textfield',
      '#title' => 'UOM',
      '#title_display' => 'none',
      '#maxlength' => 255,
      '#element_validate' => array('avbase_element_validate_category_exists'),
      '#element_validate_conditions' => array('group_id' => 'uom'),
      '#attributes' => array(
        'class' => array('prod-column-uom-id'),
      ),
      '#av_dropdown' => TRUE,
    );

    //$fields['uom_id'] = array(
    //  '#id' => 'uom-id-' . $prod_key,
    //  '#type' => 'select',
    //  '#title' => 'UOM',
    //  '#options' => avbase_get_category_options_array('uom'),
    //  '#default_value' => 0,
    //  '#empty_value' => 0,
    //  '#empty_option' => '',
    //  '#attributes' => array(
    //    'class' => array('prod-column-uom-id'),
    //  ),
    //  '#prefix' => '<div id="uom-id-' . $prod_key . '-wrapper">',
    //  '#suffix' => '</div>',
    //);

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
      '#submit' => array('avpurchase_item_list_prod_submit'),
      '#attributes' => array(
        'class' => array('prod-column-delete-btn'),
        'tabindex' => -1,
      ),
      '#ajax' => array(
        'callback' => 'avpurchase_item_list_prod_js',
        'wrapper' => 'item-list-product-row-' . $prod_key . '-wrapper',
        'effect' => 'none',
      ),
    );



    $form['product_rows'][$prod_key] = $fields;
    $form['product_rows'][$prod_key]['#prod_index'] = $prod_index;
    $form['product_rows'][$prod_key]['#theme'] = 'avbase_nestable_form_row';
    $form['product_rows'][$prod_key]['#prefix'] = '<div id="item-list-product-row-' . $prod_key . '-wrapper" class="uk-nestable-item">';
    $form['product_rows'][$prod_key]['#suffix'] = '</div>';
    //if (count($prod_rows) == ($prod_index + 1)) {
    //  $form['product_rows'][$prod_key]['#suffix'] .= '<div id="item-list-new-product-wrapper"></div>';
    //}
  }

  $form['prod_add_btn'] = array(
    '#id' => 'prod-add-btn',
    '#name' => 'prod_add_btn',
    '#type' => 'submit',
    '#value' => t('Add new row'),
    '#limit_validation_errors' => array(),
    '#submit' => array('avpurchase_item_list_prod_submit'),
    '#ajax' => array(
      'callback' => 'avpurchase_item_list_prod_js',
      'wrapper' => 'item-list-new-product-wrapper',
      'effect' => 'none',
    ),
  );


}

/**
 * Submit handler when adding or removing products on a PO form.
 */
function avpurchase_item_list_prod_submit($form, &$form_state) {
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
function avpurchase_item_list_prod_js($form, $form_state) {
  $trigger = $form_state['triggering_element'];
  $output = array();
  if ($trigger['#name'] == 'prod_add_btn') {
    $product_elements = $form['product_rows'];
    $keys = element_children($product_elements);
    $last_key = array_pop($keys);
    $last_element = empty($product_elements[$last_key]) ? array() : $product_elements[$last_key];
    $last_element['#suffix'] .= '<div id="item-list-new-product-wrapper"></div>';
    $js_settings = array('avNestableProductForm' => arraY('ajaxAction' => 'add'));
    $last_element['#attached']['js'] = array(
      array(
        'data' => $js_settings,
        'type' => 'setting',
      ),
    );

    $output = $last_element;
  }
  elseif (strstr($trigger['#name'], 'product_id')) {
    $parents = $trigger['#parents'];
    array_pop($parents);
    $parents[] = 'uom_id';
    $x = drupal_array_get_nested_value($form, $parents);
    $x['#title_display'] = 'none';
    $output = $x;
    //$output = '<div>' . $x['#name'] . '</div>';
  }
  else {
    $js_settings = array('avNestableProductForm' => array('ajaxAction' => 'remove'));
    $output['#attached']['js'] = array(
      array(
        'data' => $js_settings,
        'type' => 'setting',
      ),
    );
  }

  return $output;
}
