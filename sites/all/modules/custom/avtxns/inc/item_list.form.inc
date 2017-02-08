<?php
module_load_include('inc', 'avtxns', 'inc/item_list.handlers');
module_load_include('inc', 'avtxns', 'inc/item_list.form.validate');

/**
 * Item List form - generate info fields.
 * @param $form
 * @param $form_state
 * @param $row
 */
function avtxns_item_list_form_fields(&$form, &$form_state, $txn_type, $row, $view_mode = FALSE) {
  // Get form storage.
  $form_state['storage'] = isset($form_state['storage']) ? $form_state['storage'] : array();
  $_storage = &$form_state['storage'];

  // Get transaction type.
  $transaction = empty($_storage['transaction']) ? '' : $_storage['transaction'];
  //$transaction_type = empty($_storage['transaction_type']) ? '' : $_storage['transaction_type'];

  if (empty($view_mode)) {
    $form['#attached']['js'][] = AVTXNS_MODULE_PATH . '/js/item_list.form.js';
  }
  $form['client_name'] = array(
    '#type' => 'textfield',
    '#maxlength' => 255,
    '#required' => TRUE,
    '#element_validate' => array('avbase_element_validate_display_name_exists'),
    '#default_value' => empty($row->client_name) ? '' : $row->client_name,
    '#title' => t('Client'),
    '#id' => 'client-id',
    '#element_save_id' => 'client_id',
  );
  $form['agent_name'] = array(
    '#id' => 'agent-name',
    '#type' => 'textfield',
    '#title' => t('Agent'),
    '#maxlength' => 255,
    '#autocomplete_path' => 'av/categories/agent_vend/autocomplete',
    '#weight' => 1.2,
    '#element_validate' => array('avbase_element_validate_category_exists'),
    '#element_validate_conditions' => array('group_id' => 'agent_vend'),
    '#element_save_id' => 'agent_id',
    '#default_value' => empty($row->agent_name) ? '' : $row->agent_name,
    '#access' => $txn_type != 'spay',
  );
  $form['memo'] = array(
    '#type' => 'textarea',
    '#title' => 'Memo',
    '#default_value' => isset($row->memo) ? check_plain($row->memo) : '',
    '#maxlength' => 65000,
  );

  if ($transaction == 'sales') {
    $form['agent_name']['#autocomplete_path'] = 'av/categories/agent_cust/autocomplete';
    $form['agent_name']['#element_validate_conditions'] = array('group_id' => 'agent_cust');
  }

  //$form['discount_type'] = array(
  //  '#id' => 'discount-type',
  //  '#title_display' => 'invisible',
  //  '#type' => 'select',
  //  '#title' => 'Discount type',
  //  '#options' => avbase_get_discount_types(),
  //  '#default_value' => isset($row->discount_type) ? $row->discount_type : AVBASE_DISCOUNT_PERCENT,
  //);
  //
  //$form['discount_value'] = array(
  //  '#id' => 'discount-value',
  //  '#title_display' => 'invisible',
  //  '#type' => 'textfield',
  //  '#title' => 'Discount value',
  //  '#default_value' => isset($row->discount_value) ? round($row->discount_value, 2) : '',
  //  '#maxlength' => 19,
  //  '#element_validate' => array('element_validate_number'),
  //  //'#special_element_type' => 'number',
  //  '#attributes' => array(
  //    //'class' => array(''),
  //    //'step' => 0.01,
  //  ),
  //);
  //if (!empty($view_mode) && $form['discount_type']['#default_value'] == AVBASE_DISCOUNT_PERCENT) {
  //  $form['discount_value']['#field_suffix'] = '%';
  //  $form['discount_type']['#default_value'] = '';
  //}

  $form['transaction_date'] = array(
    '#type' => 'item',
    '#title' => 'Date',
    '#title_display' => 'none',
    '#markup' => isset($row->transaction_date) ? format_date($row->transaction_date, 'custom', 'F d, Y') : '',
    '#access' => FALSE,
  );

  $transaction_date = format_date((isset($row->transaction_date) ? $row->transaction_date : time()), 'custom', 'M. d, Y');
  $form['transaction_id'] = array(
    '#type' => 'item',
    '#title' => 'Transaction No.',
    '#title_display' => 'none',
    '#markup' => (empty($row->id) ? '' : t('<strong>Transaction #</strong> @id<br />', array('@id' => $row->id))) . $transaction_date,
    //'#access' => !empty($row->id),
  );

  $form['grand_total'] = array('#type' => 'value', '#value' => isset($row->grand_total) ? number_format($row->grand_total, 2) : '0.00');
  //$form['subtotal'] = array('#type' => 'value', '#value' => isset($row->subtotal) ? number_format($row->subtotal, 2) : '0.00');
  //$form['discounted_value'] = array('#type' => 'value', '#value' => isset($row->discounted_value) ? number_format($row->discounted_value, 2) : '0.00');


  // Product list.
  avtxns_item_list_product_fields($form, $form_state, $txn_type, $row, $view_mode);
}

/**
 * Item List form - generate product list fields.
 * @param $form
 * @param $form_state
 * @param $row
 */
function avtxns_item_list_product_fields(&$form, &$form_state, $txn_type, $row, $view_mode = FALSE) {
  // Initialize to-attached js settings.
  $js_settings = array('avbase' => array());

  // Set extra form validation for product list.
  $form['#validate'][] = 'avtxns_item_list_fields_validate';

  // Get form storage.
  $form_state['storage'] = isset($form_state['storage']) ? $form_state['storage'] : array();
  $_storage = &$form_state['storage'];

  // Get transaction type.
  $transaction = empty($_storage['transaction']) ? '' : $_storage['transaction'];
  //$transaction_type = empty($_storage['transaction_type']) ? '' : $_storage['transaction_type'];

  // Get db item rows.
  $db_row_items = empty($row->items) ? array() : $row->items;
  $initial_item_rows = array();
  foreach ($db_row_items as $item) {
    $reference_type = empty($item->reference_type) ? '' : $item->reference_type;
    $initial_item_rows["{$reference_type}_db_item_{$row->id}_{$item->id}"] = (array) $item;
  }

  // Load selected products if available.
  if (empty($row->id)) {
    $selected_product_rows = empty($_SESSION['selected_products']) ? array() : $_SESSION['selected_products'];
    $initial_item_rows = $selected_product_rows;
    unset($_SESSION['selected_products']);
  }

  // Get item rows.
  $_storage['item_rows'] = isset($_storage['item_rows']) ? $_storage['item_rows'] : $initial_item_rows;
  $_item_rows = &$_storage['item_rows'];

  // Pre-load empty product rows.
  if (empty($_item_rows) && !in_array($txn_type, array('sinv', 'spay'))) {
    $starting_empty_row_count = 10;
    for ($x = 0; $x < $starting_empty_row_count; $x++) {
      $_item_rows['new_' . $x] = array();
    }
    $_storage['new_row_counter'] = $starting_empty_row_count;
  }

  // If there are product ids found inside $_item_rows, remember them in an array.
  $detected_product_ids = array();

  $prod_index = -1;
  $form['product_rows'] = array(
    '#tree' => TRUE,
    '#theme' => 'avbase_nestable_form__' . $txn_type,
    //'#theme' => 'avbase_nestable_form',
    '#transaction' => $transaction,
  );
  //$reference_type = empty($form_state['values']['client_pending_pos_reference_type']) ? '' : $form_state['values']['client_pending_pos_reference_type'];
  foreach ($_item_rows as $prod_key => $prod_row) {
    $prod_index++;
    $fields = array();
    $order_key = '';
    $badge_title = '';
    $badge_type = 'primary';
    $reference_type = empty($prod_row['reference_type']) ? '' : $prod_row['reference_type'];
    switch ($txn_type) {
      case 'gr':
        $order_key = 'po_id';
        $badge_title = 'PO#';
        break;
      case 'dr':
        $order_key = 'so_id';
        $badge_title = 'SO#';
        break;
      case 'ret':
        $order_key = 'dr_id';
        $badge_title = 'DR#';
        break;
      case 'sinv':
        $order_key = $reference_type . '_id';
        $badge_title = strtoupper($reference_type . '#');
        $badge_type = $reference_type == 'ret' ? 'success' : $badge_type;
        break;
    }

    $order_id = empty($prod_row[$order_key]) ? NULL : $prod_row[$order_key];
    if ((strstr($prod_key, 'db_item_') && !empty($order_id) && !empty($txn_type))/* || (empty($view_mode) && !empty($prod_row['reference_id']))*/) {
      //$badge_title .= ' ' . (empty($prod_row['reference_id']) ? $order_id : $prod_row['reference_id']);
      $badge_title .= ' ' . $order_id;
      $fields['badge'] = array('#markup' => '<span class="uk-badge uk-badge-' . $badge_type . '" style="position: absolute; top: -9px; left: 0;">' . $badge_title . '</span>');
    }
    $fields['product_title'] = array(
      '#id' => 'product-title-' . $prod_key,
      '#item_row_key' => $prod_key,
      '#type' => 'textfield',
      '#title' => 'Product',
      '#maxlength' => 255,
      '#autocomplete_path' => 'av/products/autocomplete',
      '#element_validate' => array('avbase_element_validate_product_name_exists'),
      '#element_save_id' => 'product_id',
      '#save_id' => 'product_id',
      '#attributes' => array(
        'data-column-name' => 'title',
        'class' => array('prod-column-title'),
      ),
      '#default_value' => isset($prod_row['product_title']) ? $prod_row['product_title'] : '',
    );
    if (isset($prod_row['qty']) && $prod_row['qty'] == 0 && $view_mode) {
      $fields['product_title']['#default_value'] = '<del>' . $fields['product_title']['#default_value'] . '</del>';
      $fields['product_title']['#field_suffix'] = '<em class="uk-margin-small-left uk-text-uppercase uk-text-small">- Out of stock</em>';
    }
    if ($reference_type == 'ret') {
      $fields['product_title']['#attributes']['class'][] = 'uk-form-success';
    }

    $fields['uom_title'] = array(
      '#id' => 'uom-title-' . $prod_key,
      '#item_row_key' => $prod_key,
      '#type' => 'textfield',
      '#title' => 'UOM',
      '#title_display' => 'none',
      '#maxlength' => 255,
      '#element_validate' => array('avbase_element_validate_category_exists'),
      '#element_validate_conditions' => array('group_id' => 'uom'),
      '#element_save_id' => 'uom_id',
      '#attributes' => array(
        'class' => array('prod-column-uom-title'),
      ),
      '#av_dropdown' => TRUE,
      '#default_value' => isset($prod_row['uom_title']) ? $prod_row['uom_title'] : '',
    );

    $fields['qty_per_uom'] = array(
      '#id' => 'qty-per-uom-' . $prod_key,
      '#item_row_key' => $prod_key,
      '#type' => 'hidden',
      '#title' => 'Qty per UOM',
      '#title_display' => 'none',
      '#maxlength' => 10,
      '#element_validate' => array('avtxns_item_list_row_element_validate'),
      '#av_prod_element_validate' => array('element_validate_integer_positive'),
      '#attributes' => array(
        'class' => array('prod-column-qty-per-uom'),
      ),
      '#default_value' => isset($prod_row['qty_per_uom']) ? $prod_row['qty_per_uom'] : '',
    );

    //$fields['good_stock'] = array(
    //  '#type' => 'checkbox',
    //  '#title' => 'Good stock?',
    //  '#title_display' => 'none',
    //  //'#button_checkbox' => TRUE,
    //  //'#disabled' => TRUE,
    //  //'#attributes' => array(
    //  //  'title' => t('Good stock?'),
    //  //),
    //  '#default_value' => empty($prod_row['good_stock']) ? 0 : 1,
    //);
    //$fields['good_stock'] = array(
    //  '#item_row_key' => $prod_key,
    //  '#type' => 'radios',
    //  '#title' => t('Stock status'),
    //  '#title_display' => 'none',
    //  '#options' => array(
    //    0 => '<i class="uk-icon-ban"></i>',
    //    1 => '<i class="uk-icon-check"></i>',
    //  ),
    //  '#attributes' => array('class' => array('uk-button-group')),
    //  '#default_value' => isset($prod_row['good_stock']) ? $prod_row['good_stock'] : NULL,
    //  '#element_validate' => array('avtxns_item_list_row_element_validate'),
    //  //'#av_prod_element_validate' => array('avbase_element_validate_required'),
    //);
    $fields['good_stock'] = array(
      '#item_row_key' => $prod_key,
      '#type' => 'select',
      '#title' => t('Stock status'),
      '#empty_value' => '',
      '#empty_option' => '-',
      '#options' => array(t('Bad'), t('Good')),
      '#default_value' => isset($prod_row['good_stock']) ? $prod_row['good_stock'] : '',
      '#element_validate' => $txn_type == 'ret' ? array('avtxns_item_list_row_element_validate') : array(),
      //'#av_prod_element_validate' => array('avbase_element_validate_required'),
    );


    $fields['qty'] = array(
      '#id' => 'product-qty-' . $prod_key,
      '#item_row_key' => $prod_key,
      '#type' => 'textfield',
      '#title' => 'Qty.',
      '#maxlength' => 10,
      '#element_validate' => array('avtxns_item_list_row_element_validate'),
      //'#av_prod_element_validate' => array('element_validate_integer_positive'),
      '#attributes' => array(
        'data-column-name' => 'qty',
        'class' => array('prod-column-qty'),
      ),
      '#default_value' => isset($prod_row['qty']) ? $prod_row['qty'] : '',
    );
    if ($txn_type == 'dr') {
      $fields['qty']['#av_prod_element_validate'][] = 'avbase_element_validate_non_negative_number';
    }
    else {
      $fields['qty']['#av_prod_element_validate'][] = 'element_validate_integer_positive';
    }
    if ($txn_type == 'dr' || $txn_type == 'rept') {
      $fields['qty']['#av_prod_element_validate'][] = 'avtxns_element_validate_enough_qty';
    }


    $fields['cost'] = array(
      '#id' => 'product-cost-' . $prod_key,
      '#item_row_key' => $prod_key,
      '#type' => 'textfield',
      '#title' => $transaction == 'sales' ? t('Unit Price') : t('Unit Cost'),
      '#maxlength' => 19,
      '#element_validate' => array('avtxns_item_list_row_element_validate'),
      '#av_prod_element_validate' => array('element_validate_number', 'avbase_element_validate_money_length'),
      '#attributes' => array(
        'data-column-name' => 'cost',
        'data-is-negative' => $reference_type == 'ret' ? TRUE : FALSE,
        'class' => array('prod-column-cost'),
      ),
      '#default_value' => isset($prod_row['cost']) ? avbase_number_format($prod_row['cost'], 2) : '',
    );

    $fields['discount'] = array(
      '#id' => 'product-discount-' . $prod_key,
      '#item_row_key' => $prod_key,
      '#type' => 'textfield',
      '#title' => 'Discount',
      '#maxlength' => 10,
      '#element_validate' => array('avtxns_item_list_row_element_validate'),
      '#av_prod_element_validate' => array('avtxns_element_validate_product_discount'),
      '#attributes' => array(
        'data-column-name' => 'discount',
        'class' => array('prod-column-discount'),
      ),
      '#default_value' => isset($prod_row['discount']) ? $prod_row['discount'] : '',
      '#dont_require' => TRUE,
      //'#access' => !in_array($txn_type, array('ret', 'rept')),
    );

    $fields['total'] = array(
      '#id' => 'product-total-' . $prod_key,
      '#item_row_key' => $prod_key,
      '#type' => 'textfield',
      '#title' => 'Total',
      '#maxlength' => 19,
      //'#element_validate' => array('avtxns_item_list_row_element_validate'),
      //'#av_prod_element_validate' => array('element_validate_number', 'avbase_element_validate_money_length'),
      '#attributes' => array(
        'data-column-name' => 'total',
        'class' => array('prod-column-total'),
        //'readonly' => 'readonly',
      ),
      '#default_value' => isset($prod_row['total']) ? avbase_number_format($prod_row['total'], 2) : '',
    );
    if ($reference_type == 'ret') {
      $fields['total']['#attributes']['class'][] = 'uk-form-success';
    }

    // SPAY fields.
    $fields['transaction_date_formatted'] = array(
      '#type' => 'textfield',
      '#title' => 'Transaction date',
      '#maxlength' => 255,
      '#default_value' => isset($prod_row['transaction_date_formatted']) ? $prod_row['transaction_date_formatted'] : '',
      '#attributes' => array(
        //'class' => array('av-no-border'),
      ),
    );

    $fields['reference_id'] = array(
      '#type' => 'textfield',
      '#title' => 'Reference ID',
      '#maxlength' => 255,
      '#default_value' => isset($prod_row['reference_id']) ? $prod_row['reference_id'] : '',
    );
    $fields['orig_total'] = array(
      '#type' => 'textfield',
      '#title' => 'Original Amount',
      '#maxlength' => 255,
      '#default_value' => isset($prod_row['grand_total']) ? avbase_number_format($prod_row['grand_total'], 2) : '',
      '#attributes' => array(
        'class' => array('uk-text-right'),
      ),
    );
    $fields['previous_payment'] = array(
      '#type' => 'textfield',
      '#title' => 'Total paid',
      '#maxlength' => 255,
      '#default_value' => isset($prod_row['previous_payment']) ? avbase_number_format($prod_row['previous_payment'], 2) : '',
      '#attributes' => array(
        'class' => array('uk-text-right'),
      ),
    );
    $fields['balance'] = array(
      '#type' => 'textfield',
      '#title' => 'Balance',
      '#maxlength' => 255,
      '#default_value' => isset($prod_row['balance']) ? avbase_number_format($prod_row['balance'], 2) : '',
      '#attributes' => array(
        'class' => array('uk-text-right'),
      ),
    );

    $fields['prod_delete_btn'] = array(
      '#name' => 'prod_delete_btn_' . $prod_key,
      '#item_row_key' => $prod_key,
      '#type' => 'submit',
      '#value' => t('Delete'),
      '#limit_validation_errors' => array(),
      '#submit' => array('avtxns_item_list_prod_submit'),
      '#attributes' => array(
        'class' => array('prod-column-delete-btn'),
        'tabindex' => -1,
      ),
      '#ajax' => array(
        'callback' => 'avtxns_item_list_prod_js',
        'wrapper' => 'item-list-product-row-' . $prod_key . '-wrapper',
        'effect' => 'none',
        'event' => 'click',
      ),
    );
    if ($txn_type == 'sinv') {
      $fields['prod_delete_btn']['#access'] = $reference_type == 'ret';
    }

    if (!empty($_item_rows[$prod_key]['product_id'])) {
      $fields['product_title']['#attributes']['data-selected-product-id'] = $_item_rows[$prod_key]['product_id'];
      $detected_product_ids[] = $_item_rows[$prod_key]['product_id'];
    }


    // Set disabled fields
    if (!in_array($txn_type, array('spay'))) {
      $fields['total']['#attributes']['readonly'] = 'readonly';
      if (in_array($txn_type, array('sinv'))) {
        $fields['product_title']['#attributes']['readonly'] = 'readonly';
        $fields['qty']['#attributes']['readonly'] = 'readonly';
        $fields['uom_title']['#attributes']['readonly'] = 'readonly';
      }
    }

    // Disable some fields if necessary.
    $readonly_fields = array();
    if (!in_array($txn_type, array('spay'))) {
      $readonly_fields[] = 'total';
      if (in_array($txn_type, array('sinv'))) {
        $readonly_fields[] = 'product_title';
        $readonly_fields[] = 'qty';
        $readonly_fields[] = 'uom_title';
      }
    }

    $disabled_fields = array(
      'transaction_date_formatted',
      'reference_id',
      'orig_total',
      'balance',
      'previous_payment',
    );
    if ($txn_type == 'spay') {
      $readonly_fields = $disabled_fields;
      $fields['total']['#title'] = t('Payment');
      $fields['total']['#element_save_id'] = 'paid_amount';
      $fields['total']['#required'] = TRUE;
      $paid_amount = isset($prod_row['paid_amount']) ? $prod_row['paid_amount'] : $prod_row['balance'];
      $fields['total']['#default_value'] = empty($paid_amount) ? '' : avbase_number_format($paid_amount, 2);
      $fields['total']['#element_validate'] = array('element_validate_number', 'avbase_element_validate_money_length');
      //$fields['total']['#av_prod_element_validate'] = array('element_validate_number', 'avbase_element_validate_money_length');
      $disabled_fields = array(
        'product_title',
        'qty',
        'uom_title',
        'cost',
        'discount',
      );
    }
    foreach ($fields as $k => $v) {
      if (in_array($k, $disabled_fields)) {
        $fields[$k]['#access'] = FALSE;
        continue;
      }
      if (in_array($k, $readonly_fields)) {
        $fields[$k]['#attributes']['readonly'] = 'readonly';
        $fields[$k]['#attributes']['class'][] = 'uk-text-muted';
      }
    }

    $form['product_rows'][$prod_key] = $fields;
    $form['product_rows'][$prod_key]['#prod_index'] = $prod_index;
    $form['product_rows'][$prod_key]['#theme'] = 'avbase_nestable_form_row__' . $txn_type;
    //$form['product_rows'][$prod_key]['#theme'] = 'avbase_nestable_form_row';
    $form['product_rows'][$prod_key]['#prefix'] = '<div id="item-list-product-row-' . $prod_key . '-wrapper" class="uk-nestable-item">';
    $form['product_rows'][$prod_key]['#suffix'] = '</div>';
  }

  // Save product row details into drupal JS settings.
  if (!empty($detected_product_ids)) {
    $product_rows = avproduct_load_ids($detected_product_ids);
    $js_settings['avbase']['products'] = (object) $product_rows;
  }

  $form['prod_add_btn'] = array(
    '#id' => 'prod-add-btn',
    '#name' => 'prod_add_btn',
    '#type' => 'submit',
    '#value' => t('Add new row'),
    '#limit_validation_errors' => array(),
    '#submit' => array('avtxns_item_list_prod_submit'),
    '#ajax' => array(
      'callback' => 'avtxns_item_list_prod_js',
      'wrapper' => 'item-list-new-product-wrapper',
      'effect' => 'none',
      'event' => 'click',
    ),
    '#access' => !in_array($txn_type, array('sinv', 'spay')),
  );

  $js_settings['avbase']['uoms'] = avbase_get_categories('uom');
  if (empty($view_mode)) {
    $form['#attached']['js'][] = AVTXNS_MODULE_PATH . '/js/nestable_product_form.js';
    $form['#attached']['js'][] = array(
      'data' => $js_settings,
      'type' => 'setting',
    );
  }
}
