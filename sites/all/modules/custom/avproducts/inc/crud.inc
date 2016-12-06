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
    'price' => array('type' => 'float'),
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
  $data_ids = array(
    'uoms' => array('type' => 'array'),
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
    '#title' => 'Qty.',
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
    '#title' => 'Qty.',
    '#default_value' => isset($row->qty) ? $row->qty : '',
    '#maxlength' => 10,
    '#access' => empty($row->id),
    '#required' => TRUE,
    '#element_validate' => array('element_validate_integer_positive'),
    '#attributes' => array(
      'autocomplete' => 'off',
    ),
    '#special_element_type' => 'number',
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
    '#id' => 'uom-price',
    '#type' => 'textfield',
    '#title' => 'Sales price',
    '#default_value' => isset($row->price) ? round($row->price, 2) : '',
    '#maxlength' => 19,
    '#element_validate' => array('element_validate_number'),
    '#attributes' => array(
      'autocomplete' => 'off',
      'step' => '0.01',
    ),
    '#special_element_type' => 'number',
  );
  $info['cost'] = array(
    '#id' => 'uom-cost',
    '#type' => 'textfield',
    '#title' => 'Unit cost',
    '#default_value' => isset($row->cost) ? round($row->cost, 2) : '',
    '#maxlength' => 19,
    '#element_validate' => array('element_validate_number'),
    '#attributes' => array(
      'autocomplete' => 'off',
      'step' => '0.01',
    ),
    '#special_element_type' => 'number',
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
  $db_row_uoms = empty($row->data['uoms']) ? array() : $row->data['uoms'];
  $user_input = isset($form_state['input']) ? $form_state['input'] : array();
  $values = isset($form_state['values']) ? $form_state['values'] : array();
  $clicked_button = isset($form_state['triggering_element']) ? $form_state['triggering_element'] : array();
  $uom_values = isset($user_input['uoms']) ? $user_input['uoms'] : $db_row_uoms;
  $base_uom_id = isset($user_input['uom_id']) ? $user_input['uom_id'] : (isset($row->uom_id) ? $row->uom_id : 0);

  if ($clicked_button)
    switch ($clicked_button['#name']) {
      case 'uom_delete':
        $remove_uom_key = isset($clicked_button['#remove_uom_key']) ? $clicked_button['#remove_uom_key'] : NULL;
        unset($uom_values[$remove_uom_key]);
        break;
      case 'uom_add_more':
        $uom_values = empty($user_input['uoms']) ? array() : $user_input['uoms'];
        $uom_values[] = array();
        break;
  }


  $group['uom_group'] = array(
    '#prefix' => '<div id="multiple-uom-wrapper">',
    '#suffix' => '</div>',
    '#theme' => array('uom_multiple'),
  );
  $group['uom_group']['uoms']['#tree'] = TRUE;
  $uom_index = -1;
  $last_uom_id = $base_uom_id;
  foreach ($uom_values as $uom_key => $uom_row) {
    $uom_index++;
    $fields = array();
    $plural_form = avbase_get_category_plural_form(array('group_id' => 'uom', 'id' => $last_uom_id));
    $plural_form = $plural_form ? $plural_form : '(base uom)';
    $plural_form = strtolower(check_plain($plural_form));
    $fields['uom_id'] = array(
      '#id' => 'uom-select-' . $uom_index,
      '#type' => 'select',
      '#title' => 'UOM',
      '#options' => avbase_get_category_options_array('uom'),
      '#default_value' => isset($uom_row['uom_id']) ? $uom_row['uom_id'] : 0,
      '#required' => TRUE,
      '#empty_value' => 0,
      '#empty_option' => '- Select -',
      '#element_validate' => array('element_validate_av_group_uom'),
      '#attributes' => array(
        'class' => array('uom-group-select'),
        'data-uom-index' => $uom_index,
        //'data-uom-last-name' => $plural_form,
      ),
    );

    $current_uom_name = empty($uom_row['uom_id']) ? 'UOM' : $fields['uom_id']['#options'][$uom_row['uom_id']];
    $current_uom_name = strtolower($current_uom_name);
    $fields['qty'] = array(
      '#id' => 'uom-qty-' . $uom_index,
      '#type' => 'textfield',
      '#title' => 'Qty.',
      '#description' => t('number of @uom1 per @uom2', array('@uom1' => $plural_form, '@uom2' => $current_uom_name)),
      '#default_value' => isset($uom_row['qty']) ? $uom_row['qty'] : '',
      '#maxlength' => 10,
      '#element_validate' => array('element_validate_integer_positive'),
      '#required' => TRUE,
      '#attributes' => array(
        'autocomplete' => 'off',
        'data-uom-index' => $uom_index,
        'class' => array('uom-group-qty'),
      ),
      '#special_element_type' => 'number',
    );

    $fields['cost'] = array(
      '#id' => 'uom-cost-' . $uom_index,
      '#type' => 'textfield',
      '#title' => 'Unit cost',
      '#default_value' => isset($uom_row['cost']) ? round($uom_row['cost'], 2) : '',
      '#maxlength' => 19,
      '#element_validate' => array('element_validate_number'),
      '#attributes' => array(
        'autocomplete' => 'off',
        'class' => array('uom-group-cost'),
      ),
      //'#input' => FALSE,
      '#disabled' => TRUE,
    );
    $fields['price'] = array(
      '#id' => 'uom-price-' . $uom_index,
      '#type' => 'textfield',
      '#title' => 'Sales price',
      '#default_value' => !empty($uom_row['price']) ? round($uom_row['price'], 2) : '',
      '#maxlength' => 19,
      '#element_validate' => array('element_validate_number'),
      '#attributes' => array(
        'autocomplete' => 'off',
        'class' => array('uom-group-price'),'step' => '0.01',
      ),
      '#special_element_type' => 'number',
    );
    //$fields['auto_compute_price'] = array(
    //  '#id' => 'uom-price-auto-' . $uom_index,
    //  '#type' => 'checkbox',
    //  '#title' => 'Auto-compute price',
    //  '#default_value' => empty($uom_row['auto_compute_price']) ? 0 : 1,
    //);
    $fields['delete_btn'] = array(
      '#remove_uom_key' => $uom_key,
      '#type' => 'submit',
      '#name' =>'uom_delete',
      '#value' => t('Delete') . $uom_key,
      '#limit_validation_errors' => array(),
      '#submit' => array('avproducts_uom_multiple_submit'),
      '#ajax' => array(
        'callback' => 'avproducts_uom_multiple_js',
        'wrapper' => 'multiple-uom-wrapper',
        'effect' => 'none',
      ),
    );
    $group['uom_group']['uoms'][$uom_key] = $fields;
    $last_uom_id = $uom_row['uom_id'];
  }

  $group['uom_group']['add_btn'] = array(
    '#type' => 'submit',
    '#name' =>'uom_add_more',
    '#value' => t('Add another UOM'),
    //'#attributes' => array('class' => array('field-add-more-submit')),
    '#limit_validation_errors' => array(array('uoms'), array('uom_id')),
    '#submit' => array('avproducts_uom_multiple_submit'),
    '#ajax' => array(
      'callback' => 'avproducts_uom_multiple_js',
      'wrapper' => 'multiple-uom-wrapper',
      'effect' => 'none',
    ),
  );

  $form['uom'] = array(
    '#type' => 'fieldset',
    '#title' => 'Other unit of measure',
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
      //$form_state['uoms']['uom_count']++;
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
      //$uom_count = $form_state['uoms']['uom_count'];
      //$element['uoms'][$uom_count - 1]['#prefix'] = '<div class="ajax-new-content">' . (isset($element[$delta]['#prefix']) ? $element[$delta]['#prefix'] : '');
      //$element['uoms'][$uom_count - 1]['#suffix'] = (isset($element[$delta]['#suffix']) ? $element[$delta]['#suffix'] : '') . '</div>';
      break;
  }
  return $element;
}
