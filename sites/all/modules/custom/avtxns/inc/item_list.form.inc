<?php
module_load_include('inc', 'avtxns', 'inc/item_list.handlers');
module_load_include('inc', 'avtxns', 'inc/item_list.form.validate');

/**
 * Item List form - generate info fields.
 * @param $form
 * @param $form_state
 * @param $transaction_type
 * @param $row
 * @param bool $view_mode
 */
function avtxns_item_list_form_fields(&$form, &$form_state, $transaction_type, $row, $view_mode = FALSE) {
  global $user;
  // Get form storage.
  //$form_state['storage'] = isset($form_state['storage']) ? $form_state['storage'] : array();
  //$_storage = &$form_state['storage'];

  // Get transaction type details.
  $ttd = avtxns_txn_types($transaction_type);

  // Get transaction type.
  //$transaction = empty($_storage['transaction']) ? '' : $_storage['transaction'];
  //$transaction_type = empty($_storage['transaction_type']) ? '' : $_storage['transaction_type'];

  if (empty($view_mode)) {
    $form['#attached']['js'][] = AVTXNS_MODULE_PATH . '/js/item_list.form.js';
  }

  $client_status_string = isset($ttd['client_statuses']) ? implode('-', $ttd['client_statuses']) : AVCLIENT_STATUS_ACTIVE;
  $form['client_name'] = array(
    '#type' => 'textfield',
    '#maxlength' => 255,
    '#required' => TRUE,
    '#element_validate' => array('avbase_element_validate_client_name_exists'),
    '#default_value' => empty($row->client_name) ? '' : $row->client_name,
    '#title' => $ttd['ctd']['name'],
    '#title_display' => $view_mode ? 'none' : 'before',
    '#id' => 'client-id',
    '#attributes' => array(
      //'placeholder' => $ttd['ctd']['name'],
    ),
    '#element_save_id' => 'client_id',
    '#autocomplete_path' => "av/clients/autocomplete/{$ttd['ctd']['type']}/$client_status_string",
    '#avbase_autocomplete' => array(
      'entity_group' => 'clients',
    ),
  );
  if (!empty($row->id) && $view_mode && $row->client_status != AVCLIENT_STATUS_ACTIVE) {
    $csd = avclients_client_status_details($row->client_status);
    $form['client_name']['#field_suffix'] = '<em class="uk-margin-small-left">[ACCOUNT IS ' . strtoupper($csd['title']) . ']</em>';
  }
  $form['client_address_string'] = array(
    '#id' => 'client-address',
    '#type' => 'textarea',
    '#title' => 'Address',
    '#title_display' => $view_mode ? 'none' : 'before',
    //'#markup' => '<div id="client-address-string">' . (empty($row->address_string) ? '-' : $row->address_string) . '</div>',
    '#default_value' => empty($row->address_string) ? '' : $row->address_string,
    '#maxlength' => 65000,
    '#attributes' => array(
      'rows' => 3,
      'readonly' => 'readonly',
    ),
    '#resizable' => FALSE,
    '#access' => $view_mode,
  );

  $agent_category_id = 'agent' . $ttd['ctd']['type'];
  $form['agent_name'] = array(
    '#id' => 'agent-name',
    '#type' => 'textfield',
    '#title' => t('Agent'),
    '#title_display' => $view_mode ? 'inline-before' : 'before',
    '#maxlength' => 255,
    '#autocomplete_path' => "av/categories/$agent_category_id/autocomplete",
    '#weight' => 1.2,
    '#attributes' => array(
      //'placeholder' => 'Agent name',
    ),
    '#element_validate' => array('avbase_element_validate_category_exists'),
    '#element_validate_conditions' => array('group_id' => $agent_category_id),
    '#element_save_id' => 'agent_id',
    '#default_value' => empty($row->agent_name) ? '' : $row->agent_name,
    '#access' => $transaction_type != AVTXNS_TXN_TYPE_PAYMENT,
  );
  $form['reference_id'] = array(
    '#type' => 'textfield',
    '#title' => 'Reference ID',
    '#title_display' => $view_mode ? 'inline-before' : 'before',
    '#default_value' => isset($row->reference_id) ? $row->reference_id : '',
    '#maxlength' => 32,
    '#required' => TRUE,
    '#access' => in_array($transaction_type, array(AVTXNS_TXN_TYPE_RECEIVE, AVTXNS_TXN_TYPE_PURCHASE_RETURN, AVTXNS_TXN_TYPE_SALES_RETURN, AVTXNS_TXN_TYPE_PAYMENT)),
  );

  if (!empty($form_state['values']['selected_pos']['transaction_id'])) {
    $form_state['input']['return_type'] = AVTXNS_RETURN_TYPE_RUD;
  }
  $form['return_type'] = array(
    '#id' => 'return-type',
    '#type' => 'select',
    '#title' => 'Return type',
    '#title_display' => $view_mode ? 'inline-before' : 'before',
    '#options' => avtxns_get_return_types(),
    '#default_value' => isset($row->return_type) ? $row->return_type : '',
    '#required' => TRUE,
    '#empty_value' => '',
    '#empty_option' => '- Select -',
    '#access' => in_array($transaction_type, array(AVTXNS_TXN_TYPE_PURCHASE_RETURN, AVTXNS_TXN_TYPE_SALES_RETURN)),
  );
  $term_options = avbase_get_category_options_array('payment_term');
  asort($term_options);
  $form['term_id'] = array(
    '#id' => 'term-id',
    '#type' => 'select',
    '#title' => 'Terms',
    '#title_display' => $view_mode ? 'inline-before' : 'before',
    '#options' => $term_options,
    '#default_value' => isset($row->term_id) ? $row->term_id : 0,
    '#empty_value' => 0,
    '#empty_option' => '',
    '#avbase_payment_terms_js' => array('preventChildrenHide' => TRUE),
    '#access' => !in_array($transaction_type, array(AVTXNS_TXN_TYPE_SALES_RETURN, AVTXNS_TXN_TYPE_PURCHASE_RETURN, AVTXNS_TXN_TYPE_PAYMENT)),
    //'#access' => $transaction_type == AVTXNS_TXN_TYPE_INVOICE,
  );
  $date_format = 'M. d, Y';
  $form['due_date'] = array(
    '#id' => 'due-date',
    '#type' => 'textfield',
    '#title' => 'Due date',
    '#title_display' => $view_mode ? 'inline-before' : 'before',
    '#default_value' => !empty($row->due_date) ? format_date($row->due_date, 'custom', $date_format) : '',
    '#element_validate' => array('avbase_element_validate_date'),
    '#av_date_format' => $date_format,
    '#icon_key' => 'calendar',
    '#attributes' => array(
      'data-uk-datepicker' => "{format:'MMM. DD, YYYY', minDate: '" . format_date(time(), 'custom', $date_format) . "'}",
    ),
    '#required' => TRUE,
    '#access' => $transaction_type == AVTXNS_TXN_TYPE_INVOICE,
  );
  $form['email'] = array(
    '#id' => 'vendor-email',
    '#type' => 'textfield',
    '#title' => 'E-mail address',
    '#title_display' => 'before',
    '#default_value' => isset($row->email) ? $row->email : '',
    '#maxlength' => 255,
    '#attributes' => array(
      'placeholder' => 'E-mail address',
    ),
    '#element_validate' => array('avbase_element_validate_email'),
    '#access' => $transaction_type == AVTXNS_TXN_TYPE_PURCHASE_ORDER && empty($view_mode),
  );
  $form['message'] = array(
    '#type' => 'textarea',
    '#title' => 'Message to vendor',
    '#default_value' => isset($row->message) ? $row->message : '',
    '#maxlength' => 65000,
    '#access' => $transaction_type == AVTXNS_TXN_TYPE_PURCHASE_ORDER,
  );
  $form['address'] = array(
    '#type' => 'item',
    '#title' => 'Shipping address',
    '#markup' => nl2br(variable_get('av_company_address', '')),
    '#access' => $transaction_type == AVTXNS_TXN_TYPE_PURCHASE_ORDER,
  );
  $form['memo'] = array(
    '#type' => 'textarea',
    '#title' => 'Memo',
    '#default_value' => isset($row->memo) ? $row->memo : '',
    '#maxlength' => 65000,
  );

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

  $date_format = 'F d, Y';
  $txn_date_timestamp = isset($row->transaction_date) ? $row->transaction_date : time();
  $transaction_date_text = format_date($txn_date_timestamp, 'custom', $date_format);
  $form['transaction_date'] = array(
    '#type' => 'textfield',
    '#title' => 'Date',
    //'#title_display' => 'none',
    '#default_value' => $transaction_date_text,
    '#element_validate' => array('avbase_element_validate_date'),
    '#av_date_format' => $date_format,
    '#icon_key' => 'calendar',
    '#attributes' => array(
      'data-uk-datepicker' => "{format:'MMMM DD, YYYY'}",
    ),
    '#required' => TRUE,
    '#access' => empty($view_mode),
    '#disabled' => $transaction_type == AVTXNS_TXN_TYPE_PAYMENT,
  );

  $form['transaction_id'] = array(
    '#type' => 'item',
    '#title' => 'Transaction No.',
    '#title_display' => 'none',
    '#markup' => (empty($row->id) ? '' : t('<strong>Transaction #</strong> @id<br />@date', array('@id' => $row->id, '@date' => $transaction_date_text))),
    '#access' => $view_mode,
  );

  $form['grand_total'] = array('#type' => 'value', '#value' => isset($row->grand_total) ? number_format($row->grand_total, 2) : '0.00');
  //$form['subtotal'] = array('#type' => 'value', '#value' => isset($row->subtotal) ? number_format($row->subtotal, 2) : '0.00');
  //$form['discounted_value'] = array('#type' => 'value', '#value' => isset($row->discounted_value) ? number_format($row->discounted_value, 2) : '0.00');

  avbase_user_extra_details($user);
  $form['prepared_by'] = array(
    '#type' => 'item',
    '#title' => 'Prepared by',
    '#title_display' => 'inline-before',
    '#markup' => $user->avdata->full_name,
    //'#maxlength' => 65000,
    '#access' => $view_mode,
  );
  $form['received_by'] = array(
    '#type' => 'item',
    '#title' => 'Received by',
    '#title_display' => 'inline-before',
    '#markup' => '_______________________________',
    //'#maxlength' => 65000,
    '#access' => $view_mode,
  );
  //$user = user_load($user->uid);
  //dpm($user);
  // Product list.
  avtxns_item_list_product_fields($form, $form_state, $transaction_type, $row, $view_mode);
}

/**
 * Item List form - generate product list fields.
 * @param $form
 * @param $form_state
 * @param $row
 */
function avtxns_item_list_product_fields(&$form, &$form_state, $transaction_type, $row, $view_mode = FALSE) {
  // Initialize to-attached js settings.
  $js_settings = array('avbase' => array());

  // Set extra form validation for product list.
  $form['#validate'][] = 'avtxns_item_list_fields_validate';

  // Get form storage.
  $form_state['storage'] = isset($form_state['storage']) ? $form_state['storage'] : array();
  $_storage = &$form_state['storage'];

  // Get transaction type.
  //$transaction = empty($_storage['transaction']) ? '' : $_storage['transaction'];
  //$transaction_type = empty($_storage['transaction_type']) ? '' : $_storage['transaction_type'];
  $ttd = avtxns_txn_types($transaction_type);

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
  if (empty($_item_rows) && !in_array($transaction_type, array(AVTXNS_TXN_TYPE_INVOICE, AVTXNS_TXN_TYPE_PAYMENT))) {
    $starting_empty_row_count = 10;
    for ($x = 0; $x < $starting_empty_row_count; $x++) {
      $_item_rows['new_' . $x] = array();
    }
    $_storage['new_row_counter'] = $starting_empty_row_count;
  }

  //$rx = db_query("SELECT *, title AS product_title FROM avtbl_products LIMIT 250");
  //$xx = 0;
  //foreach ($rx as $ss) {
  //  $ss->item_id = $ss->id;
  //  $_item_rows['new_' . $xx++] = (array) $ss;
  //}

  // If there are product ids found inside $_item_rows, remember them in an array.
  $detected_product_ids = array();

  $prod_index = -1;
  $form['product_rows'] = array(
    '#tree' => TRUE,
    '#theme' => 'avbase_nestable_form__' . $transaction_type,
    //'#theme' => 'avbase_nestable_form',
    '#transaction' => $ttd['transaction'],
  );
  //$reference_type = empty($form_state['values']['client_pending_pos_reference_type']) ? '' : $form_state['values']['client_pending_pos_reference_type'];

  // Group item rows by category title.
  //$item_rows_by_category = array();
  //foreach ($_item_rows as $prod_key => $prod_row) {
  //  $item_rows_by_category[$prod_row['category_title']][$prod_key] = $prod_row;
  //}
  //dpm($item_rows_by_category);
  //foreach ($item_rows_by_category as $category_title => $prod_rows) {
  //
  //}
  foreach ($_item_rows as $prod_key => $prod_row) {
    $prod_index++;
    $fields = array();
    $reference_transaction_type = empty($prod_row['reference_type']) ? '' : $prod_row['reference_type'];
    $order_id = empty($prod_row['ref_txn_id']) ? NULL : $prod_row['ref_txn_id'];
    if ($transaction_type != AVTXNS_TXN_TYPE_PAYMENT && !empty($reference_transaction_type)) {
      $badge_title = strtoupper($reference_transaction_type) . '#';
      $badge_type = $reference_transaction_type == AVTXNS_TXN_TYPE_SALES_RETURN ? 'success' : 'primary';

      if ((strstr($prod_key, 'db_item_') && !empty($order_id) && !empty($transaction_type))/* || (empty($view_mode) && !empty($prod_row['reference_id']))*/) {
        //$badge_title .= ' ' . (empty($prod_row['reference_id']) ? $order_id : $prod_row['reference_id']);
        $badge_title .= ' ' . $order_id;
        $fields['badge'] = array('#markup' => '<span class="uk-badge uk-badge-' . $badge_type . '" style="position: absolute; top: -9px; left: 0;">' . $badge_title . '</span>');
      }
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
    if ($order_id) {
      $fields['product_title']['#attributes']['readonly'] = 'readonly';
    }
    if ($transaction_type == AVTXNS_TXN_TYPE_DELIVERY && isset($prod_row['qty']) && $prod_row['qty'] == 0 && $view_mode) {
      $fields['product_title']['#avhtml'] = TRUE;
      $fields['product_title']['#default_value'] = '<del>' . check_plain($fields['product_title']['#default_value']) . '</del>';
      $fields['product_title']['#field_suffix'] = '<em class="uk-margin-small-left uk-text-uppercase uk-text-small">- Out of stock</em>';
    }
    //if ($reference_transaction_type == AVTXNS_TXN_TYPE_SALES_RETURN) {
    //  $fields['product_title']['#attributes']['class'][] = 'uk-form-success';
    //}

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
      '#element_validate' => $transaction_type == AVTXNS_TXN_TYPE_SALES_RETURN ? array('avtxns_item_list_row_element_validate') : array(),
      //'#av_prod_element_validate' => array('avbase_element_validate_required'),
      '#access' => $transaction_type == AVTXNS_TXN_TYPE_SALES_RETURN,
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
        'data-transaction-id' => empty($row->id) ? 0 : $row->id,
        'data-uk-tooltip' => '',
        'class' => array('prod-column-qty'),
      ),
      '#default_value' => isset($prod_row['qty']) ? $prod_row['qty'] : '',
    );
    if (in_array($transaction_type, array(AVTXNS_TXN_TYPE_DELIVERY, AVTXNS_TXN_TYPE_INVOICE))) {
      $fields['qty']['#av_prod_element_validate'][] = 'avbase_element_validate_non_negative_number';
    }
    else {
      $fields['qty']['#av_prod_element_validate'][] = 'element_validate_integer_positive';
    }
    if ($transaction_type == AVTXNS_TXN_TYPE_DELIVERY || $transaction_type == AVTXNS_TXN_TYPE_PURCHASE_RETURN) {
      $fields['qty']['#av_prod_element_validate'][] = 'avtxns_element_validate_enough_qty';
      $fields['qty']['#attributes']['class'][] = 'js-qty-validate';
      //$fields['qty_check_field'] = array(
      //  //'#id' => 'product-qty-check-' . $prod_key,
      //  //'#item_row_key' => $prod_key,
      //  //'#type' => 'textfield',
      //  //'#title' => 'Qty Check',
      //  //'#attributes' => array(
      //  //  'data-column-name' => 'qty-check',
      //  //  'class' => array('prod-column-qty-check'),
      //  //),
      //  '#name' => 'qty_check_btn_' . $prod_key,
      //  '#item_row_key' => $prod_key,
      //  '#type' => 'button',
      //  '#value' => t('Check qty'),
      //  '#limit_validation_errors' => array(),
      //  //'#submit' => array('avtxns_item_list_prod_submit'),
      //  '#attributes' => array(
      //    //'class' => array('prod-column-delete-btn'),
      //    'tabindex' => -1,
      //  ),
      //  '#ajax' => array(
      //    'callback' => 'avtxns_qty_check_and_reserve',
      //    //'path' => 'av/ajax',
      //    //'wrapper' => 'item-list-product-row-' . $prod_key . '-wrapper',
      //    'effect' => 'none',
      //    'event' => 'click',
      //    'progress' => array(),
      //  ),
      //);
    }

//dpm('a');
    $fields['cost'] = array(
      '#id' => 'product-cost-' . $prod_key,
      '#item_row_key' => $prod_key,
      '#type' => 'textfield',
      '#title' => $ttd['transaction'] == 'sales' ? t('Unit Price') : t('Unit Cost'),
      '#maxlength' => 19,
      '#element_validate' => array('avtxns_item_list_row_element_validate'),
      '#av_prod_element_validate' => array('element_validate_number', 'avbase_element_validate_money_length'),
      '#attributes' => array(
        'data-column-name' => 'cost',
        'data-is-negative' => $reference_transaction_type == AVTXNS_TXN_TYPE_SALES_RETURN ? TRUE : FALSE,
        'class' => array('prod-column-cost'),
      ),
      '#default_value' => isset($prod_row['cost']) ? avbase_number_format($prod_row['cost'], 2) : '',
    );

    $max_discount_html = '';
    if (!empty($prod_row['discount_over_limit'])) {
      $max_discount_html = '<br /><div class="uk-text-small">' . ($view_mode ? 'Limit: ' : '') . '<em>' . $prod_row['max_discount_text'] . '</em></div>';
    }
    $fields['discount'] = array(
      '#id' => 'product-discount-' . $prod_key,
      '#item_row_key' => $prod_key,
      '#type' => 'textfield',
      '#title' => 'Discount',
      '#maxlength' => 10,
      '#element_validate' => array('avtxns_item_list_row_element_validate'),
      '#av_prod_element_validate' => array('avbase_element_validate_product_discount'),
      '#attributes' => array(
        'data-column-name' => 'discount',
        'class' => array('prod-column-discount'),
      ),
      '#default_value' => isset($prod_row['discount_text']) ? $prod_row['discount_text'] : '',
      '#dont_require' => TRUE,
      //'#access' => !in_array($transaction_type, array(AVTXNS_TXN_TYPE_SALES_RETURN, AVTXNS_TXN_TYPE_PURCHASE_RETURN)),
      '#field_suffix' => $max_discount_html,
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
    if ($reference_transaction_type == AVTXNS_TXN_TYPE_SALES_RETURN) {
      $fields['total']['#attributes']['class'][] = 'uk-form-success';
    }

    $fields['transaction_date_formatted'] = array(
      '#type' => 'textfield',
      '#title' => 'Transaction date',
      '#maxlength' => 255,
      '#default_value' => isset($prod_row['transaction_date_formatted']) ? $prod_row['transaction_date_formatted'] : '',
      '#attributes' => array(
        //'class' => array('av-no-border'),
      ),
    );

    $fields['ref_txn_dtl_id'] = array(
      '#type' => 'textfield',
      '#title' => 'Reference ID',
      '#maxlength' => 255,
      '#default_value' => isset($prod_row['ref_txn_dtl_id']) ? $prod_row['ref_txn_dtl_id'] : '',
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
      '#access' => $transaction_type != AVTXNS_TXN_TYPE_INVOICE,
    );

    if (!empty($_item_rows[$prod_key]['item_id'])) {
      $fields['product_title']['#attributes']['data-selected-product-id'] = $_item_rows[$prod_key]['item_id'];
      $detected_product_ids[] = $_item_rows[$prod_key]['item_id'];
    }


    // Set disabled fields
    if (!in_array($transaction_type, array(AVTXNS_TXN_TYPE_PAYMENT))) {
      $fields['total']['#attributes']['readonly'] = 'readonly';
      if (in_array($transaction_type, array(AVTXNS_TXN_TYPE_INVOICE))) {
        $fields['product_title']['#attributes']['readonly'] = 'readonly';
        $fields['qty']['#attributes']['readonly'] = 'readonly';
        $fields['uom_title']['#attributes']['readonly'] = 'readonly';
      }
    }

    // Disable some fields if necessary.
    $readonly_fields = array();
    if (!in_array($transaction_type, array(AVTXNS_TXN_TYPE_PAYMENT))) {
      $readonly_fields[] = 'total';
      if (in_array($transaction_type, array(AVTXNS_TXN_TYPE_INVOICE))) {
        $readonly_fields[] = 'product_title';
        $readonly_fields[] = 'qty';
        $readonly_fields[] = 'uom_title';
      }
    }

    $disabled_fields = array(
      'transaction_date_formatted',
      'ref_txn_dtl_id',
      'orig_total',
      'balance',
      'previous_payment',
    );
    if ($transaction_type == AVTXNS_TXN_TYPE_PAYMENT) {
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
    $form['product_rows'][$prod_key]['#prod_category'] = empty($prod_row['category_title']) ? '' : $prod_row['category_title'];
    $form['product_rows'][$prod_key]['#theme'] = 'avbase_nestable_form_row__' . $transaction_type;
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
    '#value' => t('Add new rows'),
    '#limit_validation_errors' => array(),
    '#submit' => array('avtxns_item_list_prod_submit'),
    '#ajax' => array(
      'callback' => 'avtxns_item_list_prod_js',
      'wrapper' => 'item-list-new-product-wrapper',
      'effect' => 'none',
      'event' => 'click',
    ),
    '#access' => !in_array($transaction_type, array(AVTXNS_TXN_TYPE_INVOICE, AVTXNS_TXN_TYPE_PAYMENT)),
  );

  $js_settings['avbase']['uoms'] = avbase_get_categories('uom');
  if (!empty($row->id)) {
    $js_settings['avbase']['transactions'][$row->id] = $row;
  }
  if (empty($view_mode)) {
    $form['#attached']['js'][] = AVTXNS_MODULE_PATH . '/js/nestable_product_form.js';
    $form['#attached']['js'][] = array(
      'data' => $js_settings,
      'type' => 'setting',
    );
  }

  //$user_reserved_qty = variable_get('avbase_user_reserved_qty', array());
  //dpm($user_reserved_qty);
}
