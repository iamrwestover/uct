<?php

/**
 * Load product form.
 * @param object $row
 *    Product row object.
 *
 * @return string
 */
function avproducts_product_form_load($row) {
  return drupal_get_form('avproducts_product_form', $row);
}

/**
 * Product form.
 *
 * @param $form
 * @param $form_state
 * @param $row
 * @return array
 */
function avproducts_product_form($form, &$form_state, $row) {
  $uom_categories = avbase_get_categories('uom');
  $js_settings = array('avbaseUOMCategories' => $uom_categories);
  $form['#attached']['js'] = array(
    AVPRODUCTS_MODULE_PATH . '/js/product_form.js',
    array(
      'data' => $js_settings,
      'type' => 'setting',
    ),
  );

  //drupal_add_js('sites/all/libraries/repeater/jquery.repeater.js');
  $form['id'] = array('#type' => 'value', '#value' => isset($row->id) ? $row->id : NULL);
  $page_title = empty($form['id']['#value']) ? 'New product' : 'Edit product';
  drupal_set_title($page_title);

  // Set crud settings.
  avproducts_product_form_settings($form);

  // Info group.
  avproducts_product_form_info_fields($form, $form_state, $row);
  // UOM group.
  avproducts_product_form_uom_fields($form, $form_state, $row);

  $form['buttons']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  $form['buttons']['cancel'] = array(
    '#markup' => l('Cancel', 'av/products'),
  );
  return $form;
}

/**
 * Set crud form settings.
 * @param $form
 */
function avproducts_product_form_settings(&$form) {
  $table_name = 'avtbl_products';

  // Set generic submit handler.
  $form['#submit'] = array('avbase_crud_form_submit');
  $form['table_name'] = array('#type' => 'value', '#value' => $table_name);
  $form['entity_name'] = array('#type' => 'value', '#value' => 'product');

  // Set field names to save.
  $save_field_ids = array(
    'created',
    'code',
    'title',
    'category_id' => array('type' => 'int'),
    'uom_id' => array('type' => 'int'),
    'description',
    'rop' => array('type' => 'int'),
    'eoq' => array('type' => 'int'),
    'cost' => array('type' => 'float'),
    'price' => array('type' => 'int'),
  );
  if (empty($form['id']['#value'])) {
    $save_field_ids[] = 'qty';
    $save_field_ids[] = 'initial_qty';
    $save_field_ids[] = 'initial_qty_date';
  }
  $form['save_field_ids'] = array(
    '#type' => 'value',
    '#value' => $save_field_ids,
  );
  // Set which fields belong to data field.
  $data_ids = array();
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
function avproducts_product_form_info_fields(&$form, &$form_state, $row) {
  $form['info'] = array(
    '#type' => 'fieldset',
    '#title' => 'Info',
  );
  $info['title'] = array(
    '#type' => 'textfield',
    '#title' => 'Title',
    '#default_value' => isset($row->title) ? check_plain($row->title) : '',
    '#maxlength' => 255,
    '#required' => TRUE,
  );
  $info['code'] = array(
    '#type' => 'textfield',
    '#title' => 'Product code',
    '#default_value' => isset($row->code) ? check_plain($row->code) : '',
    '#maxlength' => 32,
  );
  $info['category_id'] = array(
    '#type' => 'select',
    '#title' => 'Category',
    '#options' => avbase_get_category_options_array('product_cat'),
  );
  $info['description'] = array(
    '#type' => 'textarea',
    '#title' => 'Description',
    '#default_value' => isset($row->description) ? check_plain($row->description) : '',
    '#maxlength' => 65000,
  );
  $info['qty'] = array(
    '#type' => empty($row->id) ? 'textfield' : 'item',
    '#title' => 'On-hand quantity',
//    '#default_value' => isset($row->qty) ? $row->qty : 0,
    '#maxlength' => 10,
    '#access' => !empty($row->id),
    '#element_validate' => empty($row->id) ? array('avproducts_qty_validate') : array(),
  );
  if (!empty($row->id)) {
    $info['qty']['#markup'] = (isset($row->qty) ? $row->qty : 0) . ' ' . l('( adjust quantity )', '#');
  }
  $info['initial_qty'] = array(
    '#type' => 'textfield',
    '#title' => 'Beginning quantity',
    '#default_value' => isset($row->qty) ? $row->qty : '',
    '#maxlength' => 10,
    '#access' => empty($row->id),
    '#required' => TRUE,
    '#element_validate' => array('element_validate_integer_positive'),
  );
  $initial_qty_date_format = 'M. d, Y';
  $info['initial_qty_date'] = array(
    '#type' => 'textfield',
    '#title' => 'Beginning quantity start date',
    '#default_value' => isset($row->initial_qty_date) ? date($initial_qty_date_format, $row->initial_qty_date) : '',
    '#access' => empty($row->id),
    '#required' => TRUE,
    '#element_validate' => array('avbase_element_validate_date'),
    '#av_date_format' => $initial_qty_date_format,
    '#attributes' => array(
      'autocomplete' => 'off',
    ),
  );

  $default_uom = avbase_get_category(array('group_id' => 'uom', 'system_default' => 1));
  $default_uom_id = isset($default_uom->id) ? $default_uom->id : 0;
  $info['uom_id'] = array(
    '#id' => 'uom-id',
    '#type' => 'select',
    '#title' => 'Base UOM',
    '#options' => avbase_get_category_options_array('uom'),
    '#default_value' => isset($row->uom_id) ? $row->uom_id : $default_uom_id,
    '#empty_value' => 0,
    '#empty_option' => '- Select -',
    //'#required' => TRUE,
  );
  $info['rop'] = array(
    '#type' => 'textfield',
    '#title' => 'Reorder point',
    '#default_value' => isset($row->rop) ? $row->rop : '',
    '#maxlength' => 10,
    '#element_validate' => array('element_validate_integer_positive'),
  );
  $info['eoq'] = array(
    '#type' => 'textfield',
    '#title' => 'EOQ',
    '#default_value' => isset($row->eoq) ? $row->eoq : '',
    '#maxlength' => 10,
    '#element_validate' => array('element_validate_integer_positive'),
  );
  $info['price'] = array(
    '#type' => 'textfield',
    '#title' => 'Sales price',
    '#default_value' => isset($row->price) ? avbase_money_format('%01.2f', $row->price) : '',
    '#maxlength' => 19,
    '#element_validate' => array('element_validate_number'),
  );
  //$info['price2'] = array(
  //  '#type' => 'textfield',
  //  '#title' => 'Sales price',
  //  '#default_value' => isset($row->price) ? avbase_money_format('%01.2f', $row->price) : '',
  //  '#maxlength' => 19,
  //  '#element_validate' => array('element_validate_number'),
  //);
  $info['cost'] = array(
    '#type' => 'textfield',
    '#title' => 'Unit cost',
    '#default_value' => isset($row->cost) ? avbase_money_format('%01.2f', $row->cost) : '',
    '#maxlength' => 19,
    '#element_validate' => array('element_validate_number'),
  );
  $form['info'] += $info;
}

/**
 * Product form - generate UOM fields.
 * @param $form
 * @param $form_state
 * @param $row
 */
function avproducts_product_form_uom_fields(&$form, &$form_state, $row) {
  $user_input = isset($form_state['input']) ? $form_state['input'] : array();
  $clicked_button = isset($form_state['triggering_element']) ? $form_state['triggering_element'] : array();
  $uom_count = $form_state['uoms']['uom_count'] = isset($form_state['uoms']['uom_count']) ? $form_state['uoms']['uom_count'] : 0;

  $form['uom'] = array(
    '#type' => 'fieldset',
    '#title' => 'Other unit of measure',
  );

  $group['uom_group'] = array(
    '#prefix' => '<div id="multiple-uom-wrapper">',
    '#suffix' => '</div>',
    '#tree' => TRUE,
    '#theme' => array('uom_multiple'),
  );


  $current_uom_group_values = isset($user_input['uom_group']['uoms']) ? $user_input['uom_group']['uoms'] : array();
  for ($x = 0; $x < $uom_count; $x++) {
    $remove_index = isset($clicked_button['#remove_uom_index']) ? $clicked_button['#remove_uom_index'] : NULL;
    if ($x === $remove_index) {
      $form_state['uoms']['uom_count']--;
      continue;
    }

    $last_uom_id = isset($current_uom_group_values[$x - 1]) ? $current_uom_group_values[$x - 1]['uom_id'] : $user_input['uom_id'];
    $plural_form = avbase_get_category_plural_form(array('group_id' => 'uom', 'id' => $last_uom_id));
    $plural_form = $plural_form ? $plural_form : '(base uom)';
    $plural_form = strtolower(check_plain($plural_form));
    $group['uom_group']['uoms'][$x]['uom_id'] = array(
      '#id' => 'uom-select-' . $x,
      '#type' => 'select',
      '#title' => 'UOM',
      '#options' => avbase_get_category_options_array('uom'),
      //'#default_value' => isset($row->uom_id) ? $row->uom_id : 0,
      '#required' => TRUE,
      '#empty_value' => 0,
      '#empty_option' => '- Select -',
      '#element_validate' => array('element_validate_av_group_uom'),
      '#attributes' => array(
        'class' => array('uom-group-select'),
        'data-uom-index' => $x,
        //'data-uom-last-name' => $plural_form,
      ),
    );

    $current_uom_name = empty($current_uom_group_values[$x]['uom_id']) ? 'UOM' : $group['uom_group']['uoms'][$x]['uom_id']['#options'][$current_uom_group_values[$x]['uom_id']];
    $current_uom_name = strtolower($current_uom_name);
    $group['uom_group']['uoms'][$x]['qty'] = array(
      '#id' => 'uom-qty-' . $x,
      '#type' => 'textfield',
      '#title' => 'Qty.',
      '#description' => t('number of @uom1 per @uom2', array('@uom1' => $plural_form, '@uom2' => $current_uom_name)),
      //'#default_value' => isset($row->rop) ? $row->rop : '',
      '#maxlength' => 10,
      '#element_validate' => array('element_validate_integer_positive'),
      '#required' => TRUE,
      '#attributes' => array(
        'autocomplete' => 'off',
        'class' => array('uom-group-qty'),
      ),
    );
    //$group['uom_group']['uoms'][$x]['per_uom_label'] = array(
    //  '#type' => 'item',
    //  '#title' => '&nbsp;',
    //  '#markup' => 'per box'
    //);
    $group['uom_group']['uoms'][$x]['delete_btn'] = array(
      '#remove_uom_index' => $x,
      '#type' => 'submit',
      '#name' =>'uom_delete',
      '#value' => t('Delete'),
      '#limit_validation_errors' => array(),
      '#submit' => array('avproducts_uom_multiple_submit'),
      '#ajax' => array(
        'callback' => 'avproducts_uom_multiple_js',
        'wrapper' => 'multiple-uom-wrapper',
        'effect' => 'fade',
      ),
    );
  }
  $group['uom_group']['add_btn'] = array(
    '#type' => 'submit',
    '#name' =>'uom_add_more',
    '#value' => t('Add another UOM'),
    //'#attributes' => array('class' => array('field-add-more-submit')),
    '#limit_validation_errors' => array(array('uom_group'), array('uom_id')),
    '#submit' => array('avproducts_uom_multiple_submit'),
    '#ajax' => array(
      'callback' => 'avproducts_uom_multiple_js',
      'wrapper' => 'multiple-uom-wrapper',
      'effect' => 'fade',
    ),
  );
  $form['uom'] += $group;
}

/**
 * Validate qty field.
 * @param $form
 * @param $form_state
 * @param $form
 */
function avproducts_qty_validate($element, &$form_state, $form) {
  // If this is a new record, set qty to be the same as initial qty.
  if (empty($form_state['values']['id'])) {
    $form_state['values']['qty'] = $form_state['values']['initial_qty'];
    $element['#value'] = $form_state['values']['qty'];
  }
  element_validate_integer_positive($element, $form_state);
}

/**
 * Menu callback: confirm product delete.
 * @param $form
 * @param $form_state
 * @param object $row
 *
 * @return array
 */
function avproducts_product_delete_confirm_form($form, &$form_state, $row) {
  $prompt = 'Are you sure you want to delete this product?';
  $row_title = check_plain($row->title);
  $row_description = check_plain($row->description);
  $description = <<<HTML
  <h1 class="uk-article-title">$row_title</h1>$row_description
  <hr />
  <p>This action cannot be undone.</p>
HTML;
  $form_state['row'] = $row;
  return confirm_form(array(), $prompt, '', $description);
}

/**
 * Handler for product delete.
 *
 * @see avproducts_product_delete_confirm_form()
 */
function avproducts_product_delete_confirm_form_submit($form, &$form_state) {
  if (isset($form_state['row']->id)) {
    if (avproducts_product_delete($form_state['row']->id)) {
      drupal_set_message('Record deleted.');
    }
  }
  $form_state['redirect'] = '';
}

/**
 * Submit handler for the UOM form (multiple UOMs) in a product form.
 *
 * This handler is run regardless of whether JS is enabled or not. It makes
 * changes to the form state. If the button was clicked with JS disabled, then
 * the page is reloaded with the complete rebuilt form. If the button was
 * clicked with JS enabled, then ajax_form_callback() calls avproducts_uom_multiple_js()
 * to return just the changed part of the form.
 */
function avproducts_uom_multiple_submit($form, &$form_state) {
  $button = $form_state['triggering_element'];
  switch ($button['#name']) {
    case 'uom_add_more':
      $form_state['uoms']['uom_count']++;
      break;
  }

  $form_state['rebuild'] = TRUE;
}

/**
 * Ajax callback in response to the UOM form (multiple UOMS) being submitted.
 *
 * This returns the new page content to replace the page content made obsolete
 * by the form submission.
 *
 * @see avproducts_uom_multiple_submit()
 */
function avproducts_uom_multiple_js($form, $form_state) {
  $button = $form_state['triggering_element'];
  $element = $form['uom']['uom_group'];
  switch ($button['#name']) {
    case 'uom_add_more':
      $uom_count = $form_state['uoms']['uom_count'];
      $element['uoms'][$uom_count - 1]['#prefix'] = '<div class="ajax-new-content">' . (isset($element[$delta]['#prefix']) ? $element[$delta]['#prefix'] : '');
      $element['uoms'][$uom_count - 1]['#suffix'] = (isset($element[$delta]['#suffix']) ? $element[$delta]['#suffix'] : '') . '</div>';
      break;
  }
  return $element;
}
