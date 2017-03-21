<?php

/**
 * Load product form.
 * @param object $row
 *    Product row object.
 *
 * @return string
 */
function avproducts_product_form_load($row, $view_mode = FALSE) {
  return drupal_get_form('avproducts_product_form', $row, $view_mode);
}

/**
 * Product form.
 *
 * @param $form
 * @param $form_state
 * @param $row
 * @return array
 */
function avproducts_product_form($form, &$form_state, $row, $view_mode = FALSE) {
  $uom_categories = avbase_get_categories('uom');
  $js_settings = array('avbaseUOMCategories' => $uom_categories);
  $form['#attached']['js'] = array(
    AVPRODUCTS_MODULE_PATH . '/js/product_form.js',
    //AVBASE_LIBRARIES_PATH . '/select2/select2.min.js',
    array(
      'data' => $js_settings,
      'type' => 'setting',
    ),
  );
  $form['#attached']['css'] = array(
    //AVBASE_LIBRARIES_PATH . '/select2/select2.min.css',
  );

  //drupal_add_js('sites/all/libraries/repeater/jquery.repeater.js');
  $form['id'] = array('#type' => 'value', '#value' => (isset($row->id) ? $row->id : NULL));
  $page_title = empty($form['id']['#value']) ? 'New product' : 'Product';
  drupal_set_title($page_title);

  // Set crud settings.
  avproducts_product_form_settings($form, $row, $view_mode);

  // Info group.
  avproducts_product_form_info_fields($form, $form_state, $row, $view_mode);
  // UOM group.
  avproducts_product_form_uom_fields($form, $form_state, $row, $view_mode);

  $form['buttons']['#theme'] = 'avbase_crud_button_group';
  $form['buttons']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  $form['buttons']['cancel'] = array(
    '#markup' => l('Cancel', 'av/products'),
  );

  // Status buttons.
  if ($view_mode && !empty($row->id)) {
    $rsd = avproducts_product_status_details($row->status);
    $can_change_status = user_access(AVPRODUCTS_PERM_CHANGE_STATUS);
    $form['buttons']['activate'] = array(
      '#markup' => l('Enable', "av/products/{$row->id}/" . AVPRODUCTS_STATUS_ACTIVE . "/change-status", array(
        'attributes' => array('class' => 'uk-button uk-button-primary uk-margin-small-right'),
        'query' => array('destination' => 'av/products/' . $row->id . '/view'),
        'icon_key' => 'check',
        'icon_classes' => array('uk-margin-small-right'),
      )),
      '#avposition' => 'opposite',
      '#access' => $can_change_status && $row->status != AVPRODUCTS_STATUS_ACTIVE,
    );
    $form['buttons']['disable'] = array(
      '#markup' => l('Disable', "av/products/{$row->id}/" . AVPRODUCTS_STATUS_DISABLED . "/change-status", array(
        'attributes' => array('class' => 'uk-button uk-margin-small-right'),
        'query' => array('destination' => 'av/products/' . $row->id . '/view'),
        'icon_key' => 'ban',
        'icon_classes' => array('uk-margin-small-right'),
      )),
      '#avposition' => 'opposite',
      '#access' => $can_change_status && $row->status == AVPRODUCTS_STATUS_ACTIVE,
    );

    if ($row->status != AVPRODUCTS_STATUS_ACTIVE) {
      $form_prefix = empty($form['#prefix']) ? '' : $form['#prefix'];
      $form_prefix = '<div class="uk-alert ' . $rsd['color_class'] . '"><span class="uk-badge ' . $rsd['badge_class'] . '" style="position: absolute;top:10px;left:10px;">' . $rsd['title'] . '</span>' . $form_prefix;
      $form['#prefix'] = $form_prefix;
      $form_suffix = empty($form['#suffix']) ? '' : $form['#suffix'];
      $form_suffix .= '</div>';
      $form['#suffix'] = $form_suffix;
    }
  }



  // Convert form to non-edit mode if we are on View page.
  avbase_allow_form_view_mode($form, $view_mode);
  return $form;
}

/**
 * Set crud form settings.
 * @param $form
 */
function avproducts_product_form_settings(&$form, $row, $view_mode = FALSE) {
  $table_name = 'avtbl_products';

  $form['last_changed'] = array('#type' => 'hidden', '#default_value' => (isset($row->changed) ? $row->changed : NULL));

  // Set generic submit handler.
  $form['#submit'] = array('avbase_crud_form_submit');
  $form['table_name'] = array('#type' => 'value', '#value' => $table_name);
  $form['entity_name'] = array('#type' => 'value', '#value' => 'product');

  // Generic form validator.
  $form['#validate'][] = 'avbase_crud_form_validate';

  // Set autocomplete off.
  $form['#attributes']['autocomplete'] = 'off';

  // Set field names to save.
  $db_op = empty($form['id']['#value']) ? 'insert' : 'update';
  $save_field_ids = avproducts_table_fields_to_save($db_op);
  $form['save_field_ids'] = array(
    '#type' => 'value',
    '#value' => $save_field_ids,
  );
  // Set which fields belong to data field.
  $data_ids = empty($save_field_ids['data']) ? array() : $save_field_ids['data'];
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
function avproducts_product_form_info_fields(&$form, &$form_state, $row, $view_mode = FALSE) {
  $form['info'] = array(
    '#type' => 'fieldset',
    '#title' => 'Info',
  );
  $info['title'] = array(
    '#type' => 'textfield',
    '#title' => 'Title',
    '#default_value' => isset($row->title) ? $row->title : '',
    '#maxlength' => 255,
    '#required' => TRUE,
    '#element_validate' => array('avbase_element_validate_unique_field'),
  );
  $info['code'] = array(
    '#type' => 'textfield',
    '#title' => 'Product code',
    '#default_value' => isset($row->code) ? $row->code : '',
    '#maxlength' => 32,
    '#element_validate' => array('avbase_element_validate_unique_field'),
    '#attributes' => array(
      'class' => array('uk-text-uppercase'),
    ),
    '#required' => TRUE,
  );
  $info['category_id'] = array(
    '#type' => 'select',
    '#title' => 'Category',
    '#options' => avbase_get_category_options_array('product_cat'),
    '#empty_value' => 0,
    '#empty_option' => '- Select -',
    '#default_value' => isset($row->category_id) ? $row->category_id : 0,
  );
  //$info['description'] = array(
  //  '#type' => 'textarea',
  //  '#title' => 'Description',
  //  '#default_value' => isset($row->description) ? check_plain($row->description) : '',
  //  '#maxlength' => 255,
  //);
  $info['qty'] = array(
    '#type' => 'textfield',
    '#title' => 'On-hand qty.',
    '#default_value' => isset($row->qty) ? $row->qty : '',
    '#maxlength' => 10,
    '#access' => !empty($row->id),
    '#element_validate' => empty($row->id) ? array('avproducts_qty_validate') : array(),
    '#disabled' => !empty($row->id)
  );
  //if (!empty($row->id)) {
  //  $info['qty']['#suffix'] = '<div class="">' . l('adjust quantity', '#') . '</div>';
  //}
  $info['initial_qty'] = array(
    '#type' => 'textfield',
    '#title' => 'Beginning qty.',
    '#default_value' => isset($row->qty) ? $row->qty : '',
    '#maxlength' => 10,
    '#access' => empty($row->id),
    '#required' => TRUE,
    '#element_validate' => array('element_validate_integer'),
    //'#attributes' => array(
    //  'data-uk-tooltip' => "{pos:'bottom'}",
    //  'title' => 'Beginning quantity',
    //),
    '#special_element_type' => 'number',
  );
  //$initial_qty_date_format = 'M. d, Y';
  //$info['initial_qty_date'] = array(
  //  '#type' => 'textfield',
  //  '#title' => 'Beginning qty. date',
  //  '#default_value' => isset($row->initial_qty_date) ? date($initial_qty_date_format, $row->initial_qty_date) : '',
  //  '#access' => empty($row->id),
  //  '#required' => TRUE,
  //  '#element_validate' => array('avbase_element_validate_date'),
  //  '#av_date_format' => $initial_qty_date_format,
  //  '#attributes' => array(
  //    'autocomplete' => 'off',
  //  ),
  //);

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
    '#required' => TRUE,
  );
  $info['rop'] = array(
    '#type' => 'textfield',
    '#title' => 'ROP',
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
    '#default_value' => isset($row->price) ? avbase_number_format($row->price) : '',
    '#maxlength' => 19,
    '#element_validate' => array('element_validate_number', 'avbase_element_validate_money_length'),
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
    '#default_value' => isset($row->cost) ? avbase_number_format($row->cost) : '',
    '#maxlength' => 19,
    '#element_validate' => array('element_validate_number', 'avbase_element_validate_money_length'),
    '#attributes' => array(
      'step' => '0.01',
    ),
    '#special_element_type' => 'number',
  );

  $date_format = 'M. d, Y';
  $info['expiry_date'] = array(
    '#type' => 'textfield',
    '#title' => 'Expiration date',
    '#default_value' => !empty($row->expiry_date) ? format_date($row->expiry_date, 'custom', $date_format) : '',
    '#element_validate' => array('avbase_element_validate_date'),
    '#av_date_format' => $date_format,
    '#icon_key' => 'calendar',
    '#attributes' => array(
      'data-uk-datepicker' => "{format:'MMM. DD, YYYY', minDate: '" . format_date(time(), 'custom', $date_format) . "'}",
    ),
  );

  $info['lot_no'] = array(
    '#type' => 'textfield',
    '#title' => 'Lot no.',
    '#default_value' => isset($row->lot_no) ? $row->lot_no : '',
    '#maxlength' => 32,
  );
  $info['vendor_id'] = array(
    '#id' => 'vendor-id',
    '#type' => 'textfield',
    '#title' => t('Preferred vendor'),
    '#maxlength' => 255,
    '#autocomplete_path' => 'av/clients/autocomplete/' . AVCLIENT_STATUS_ACTIVE . '/' . AVCLIENT_TYPE_VENDOR,
    '#element_validate' => array('avbase_element_validate_client_name_exists'),
    //'#table_name' => 'avtbl_vendors',
    '#default_value' => empty($row->vendor_name) ? '' : $row->vendor_name,
  );
  $info['account_type_id'] = array(
    '#type' => 'select',
    '#title' => 'Account type',
    '#options' => avbase_get_category_options_array('account_type'),
    '#empty_value' => 0,
    '#empty_option' => '- Select -',
    '#default_value' => isset($row->account_type_id) ? $row->account_type_id : 0,
  );
  $info['shelf_id'] = array(
    '#type' => 'select',
    '#title' => 'Shelf location',
    '#options' => avbase_get_category_options_array('shelf'),
    '#empty_value' => 0,
    '#empty_option' => '- Select -',
    '#default_value' => isset($row->shelf_id) ? $row->shelf_id : 0,
  );
  $info['principal_id'] = array(
    '#type' => 'select',
    '#title' => 'Principal',
    '#options' => avbase_get_category_options_array('principal'),
    '#empty_value' => 0,
    '#empty_option' => '- Select -',
    '#default_value' => isset($row->principal_id) ? $row->principal_id : 0,
  );

  $info['discount'] = array(
    '#type' => 'textfield',
    '#title' => 'Max allowed discount (%)',
    '#maxlength' => 10,
    '#element_validate' => array('avbase_element_validate_product_discount'),
    '#default_value' => isset($row->discount_text) ? $row->discount_text : '',
  );

  $form['info'] += $info;
}

/**
 * Product form - generate UOM fields.
 * @param $form
 * @param $form_state
 * @param $row
 */
function avproducts_product_form_uom_fields(&$form, &$form_state, $row, $view_mode = FALSE) {
  $uom_categories = avbase_get_categories('uom');
  $db_row_uoms = empty($row->data['uoms']) ? array() : $row->data['uoms'];
  $user_input = isset($form_state['input']) ? $form_state['input'] : array();
  $values = isset($form_state['values']) ? $form_state['values'] : array();
  $clicked_button = isset($form_state['triggering_element']) ? $form_state['triggering_element'] : array();
  $uom_values = isset($user_input['uoms']) ? $user_input['uoms'] : $db_row_uoms;
  $base_uom_id = isset($user_input['uom_id']) ? $user_input['uom_id'] : (isset($row->uom_id) ? $row->uom_id : 0);

  if ($clicked_button) {
    switch ($clicked_button['#name']) {
      case 'uom_add_more':
        $uom_values = empty($user_input['uoms']) ? array() : $user_input['uoms'];
        $uom_values[] = array();
        break;
      default:
        $remove_uom_key = isset($clicked_button['#remove_uom_key']) ? $clicked_button['#remove_uom_key'] : NULL;
        unset($uom_values[$remove_uom_key]);
        break;
    }
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
    // Make sure selected UOM still exists.
    if (!empty($uom_row['uom_id']) && !isset($uom_categories[$uom_row['uom_id']])) {
      continue;
    }

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
      '#description' => t('@uom1 per @uom2', array('@uom1' => $plural_form, '@uom2' => $current_uom_name)),
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
      '#default_value' => isset($uom_row['cost']) ? avbase_number_format($uom_row['cost']) : '',
      '#maxlength' => 19,
      '#element_validate' => array('element_validate_number', 'avbase_element_validate_money_length'),
      '#attributes' => array(
        'autocomplete' => 'off',
        'class' => array('uom-group-cost'),
        'step' => '0.01',
      ),
      '#special_element_type' => 'number',
      //'#input' => FALSE,
      '#disabled' => TRUE,
    );
    $fields['price'] = array(
      '#id' => 'uom-price-' . $uom_index,
      '#type' => 'textfield',
      '#title' => 'Sales price',
      '#default_value' => !empty($uom_row['price']) ? avbase_number_format($uom_row['price']) : '',
      '#maxlength' => 19,
      '#element_validate' => array('element_validate_number', 'avbase_element_validate_money_length'),
      '#attributes' => array(
        'autocomplete' => 'off',
        'class' => array('uom-group-price'),
        'step' => '0.01',
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
      '#name' =>'uom_delete_' . $uom_key,
      '#value' => t('Delete'),
      '#limit_validation_errors' => array(),
      '#submit' => array('avproducts_uom_multiple_submit'),
      '#ajax' => array(
        'callback' => 'avproducts_uom_multiple_js',
        'wrapper' => 'multiple-uom-wrapper',
        'effect' => 'none',
      ),
    );
    $group['uom_group']['uoms'][$uom_key] = $fields;
    //$last_uom_id = $uom_row['uom_id'];
  }

  $group['uom_group']['add_btn'] = array(
    '#id' => 'uom-add-btn',
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
  //if (isset($form_state['row']->id)) {
  //  if (avproducts_product_delete($form_state['row']->id)) {
  //    drupal_set_message('Record deleted.');
  //  }
  //}

  drupal_set_message(t('Record delete is disabled'), 'error');
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

/**
 * Perform actions depending on the kind of change that will be made
 * on the database for Products.
 * @param $op
 * @param $row
 * @param $form_state
 */
function avproducts_avbase_crud_dbchange_product($op, $row, $form_state) {
  $values = $form_state['values'];
  switch ($op) {
    case 'pre-save':
      // Make sure uoms uses uom_id as their key on $row['data'];
      $data = unserialize($row['data']);
      $uoms = empty($data['uoms']) ? array() : $data['uoms'];
      $rekeyed_uoms = array();
      foreach ($uoms as $k => $uom) {
        unset($uom['cost']);
        unset($uom['delete_btn']);
        $rekeyed_uoms[$uom['uom_id']] = $uom;
      }

      if ($rekeyed_uoms) {
        $data['uoms'] = $rekeyed_uoms;
        $row['data'] = serialize($data);
      }

      if (empty($row['id']) && empty($row['initial_qty_date'])) {
        $row['initial_qty_date'] = time();
      }

      // Explode discount text to discount array.
      $discount_text = isset($values['discount']) ? trim($values['discount']) : '';
      $discount_parts = explode('/', $discount_text);
      $x = 1;
      foreach ($discount_parts as $k => $v) {
        if (!is_numeric($v)) {
          continue;
        }
        $row['discount' . $x++] = (float) $v;
      }
      return $row;
      break;
  }
}

/**
 * Menu callback: confirm product status change.
 * @param $form
 * @param $form_state
 * @param object $row
 * @param $status
 *
 * @return array
 */
function avproducts_product_change_status_confirm_form($form, &$form_state, $row, $status) {
  switch ($status) {
    case AVCLIENT_STATUS_DISABLED:
      $prompt = 'Are you sure you want to disable this product?';
      break;
    case AVCLIENT_STATUS_ACTIVE:
      $prompt = 'Are you sure you want to enable this product?';
      break;
    default:
      $prompt = 'Are you sure?';
      break;
  }

  $display_name = check_plain($row->title);
  $description = <<<HTML
  <h1 class="uk-article-title">$display_name</h1>
HTML;
  $form_state['row'] = $row;
  $form_state['status'] = $status;
  return confirm_form(array(), $prompt, '', $description);
}

/**
 * Handler for product status change.
 *
 * @see avproducts_product_change_status_confirm_form()
 */
function avproducts_product_change_status_confirm_form_submit($form, &$form_state) {
  if (isset($form_state['row']->id)) {
    $status = isset($form_state['status']) ? $form_state['status'] : '';
    if (avproducts_product_change_status($form_state['row']->id, $status)) {

      switch ($status) {
        case AVPRODUCTS_STATUS_DISABLED:
          //drupal_set_message('Client\'s account has been disabled.');
          break;
        case AVPRODUCTS_STATUS_ACTIVE:
          drupal_set_message('Product has been enabled.');
          break;
        default:
          break;
      }
    }
  }
  $form_state['redirect'] = '';
}
