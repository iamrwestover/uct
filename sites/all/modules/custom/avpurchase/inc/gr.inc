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
  $form['#id'] = 'avpurchase-gr-form';
  $form['#prefix'] = '<div id="avpurchase-gr-form-wrapper"> <div id="avpurchase-gr-form-wrapper2"></div>';
  $form['#suffix'] = '</div>';

  // Set crud settings.
  avpurchase_gr_form_settings($form, $row);

  //$form['default'] = array(
  //  '#type' => 'container',
  //  '#attributes' => array(
  //    'id' => 'gr-form-default',
  //  ),
  //);
  $form = avpurchase_gr_form_default($form, $form_state, $row);
  $form['cost_changes'] = avpurchase_gr_form_cost_changes_fields($form_state, $row);
  $form['cost_changes']['#type'] = 'container';
  //$form['cost_changes']['#attributes']['id'] = 'avpurchase-gr-cost-changed-wrapper';
  $form['cost_changes']['#theme'] = 'avbase_modal_form';
  $form['cost_changes']['#modal_options'] = array(
    'modal_id' => 'cost-changed',
    'modal_title' => t('Some product costs have been changed. Do you want to update our records?'),
    'modal_classes' => array('uk-modal-dialog-large'),
  );

  $form['vendor_pos'] = avpurchase_gr_form_vendor_pos_fields($form_state, $row);
  $form['vendor_pos']['#type'] = 'container';
  //$form['vendor_pos']['#attributes']['id'] = 'avpurchase-gr-cost-changed-wrapper';
  $form['vendor_pos']['#theme'] = 'avbase_modal_form';
  $form['vendor_pos']['#modal_options'] = array(
    'modal_id' => 'vendor-pos',
    'modal_title' => t('Pending purchase orders to this vendor'),
    //'modal_classes' => array('uk-modal-dialog-large'),
  );

  //switch ($form_page) {
  //  case 'default':
  //    return avpurchase_gr_form_default_fields($form, $form_state, $row);
  //    break;
  //  case 'cost_changes':
  //    return avpurchase_gr_form_cost_changes_fields($form, $form_state, $row);
  //    //return $form;
  //    break;
  //}


  // Convert form to non-edit mode if we are on View page.
  //avbase_allow_form_view_mode($form);
  return $form;
}

/**
 * Default Goods Received form fields.
 * @param $form
 * @param $form_state
 * @param $row
 *
 * @return array
 */
function avpurchase_gr_form_default(&$form, &$form_state, $row) {
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

  $form['#theme'] = 'avpurchase_item_list_form';



  // Goods Received fields.
  avpurchase_gr_form_fields($form, $form_state, $row);

  // Info group.
  avpurchase_item_list_form_fields($form, $form_state, $row);

  // Product list.
  avpurchase_item_list_product_fields($form, $form_state, $row);

  $form['buttons']['#theme'] = 'avbase_crud_button_group';
  $form['buttons']['submit'] = array(
    '#id' => 'detect-cost-change-btn',
    '#type' => 'submit',
    '#value' => t('Receive these items'),
    '#attributes' => array(
      'class' => array('av-ajax-trigger', 'uk-button-primary'),
    ),
    //'#limit_validation_errors' => array(),
    '#ajax' => array(
      'callback' => 'avpurchase_gr_form_ajax',
      //'wrapper' => 'avpurchase-gr-cost-changed-wrapper',
      'wrapper' => 'avpurchase-gr-form-wrapper',
      'effect' => 'none',
      'event' => 'click',
    ),
  );
  $form['buttons']['check_po'] = array(
    '#id' => 'check-po-btn',
    '#type' => 'submit',
    '#value' => t('Check for floating po\'s'),
    '#attributes' => array(
      'class' => array('av-ajax-trigger', 'uk-button-primary', 'uk-hidden'),
    ),
    '#submit' => array('avpurchase_gr_vendor_pos_submit'),
    '#limit_validation_errors' => array(array('vendor_id')),
    '#ajax' => array(
      'callback' => 'avpurchase_gr_form_ajax',
      //'wrapper' => 'avpurchase-gr-cost-changed-wrapper',
      'wrapper' => 'avpurchase-gr-form-wrapper',
      'effect' => 'none',
      'event' => 'click',
    ),
  );
  $form['buttons']['cancel'] = array(
    '#markup' => l('Cancel', 'av/purchase'),
  );


  //$form['#suffix'] = theme('avbase_modal', array('modal_id' => 'cost-changed', 'modal_title' => t('Product cost changes'), 'modal_classes' => array('uk-modal-dialog-large')));
  return $form;
}

function avpurchase_gr_vendor_pos_submit($form, &$form_state) {
  $form_state['rebuild'] = TRUE;
  $trigger = $form_state['triggering_element'];
  switch ($trigger['#id']) {
    case 'check-po-btn':
      $vendor_id = (int) $form_state['values']['vendor_id'];
      $conditions = array();
      $conditions[] = array('avpo.vendor_id', $vendor_id);
      $conditions[] = array('avpo.status', 1);
      $pos = avpurchase_po_query_load($conditions);
      foreach ($pos as &$po) {
        avpurchase_po_attach_received_items($po);
        //$po_items = (array) $po->items;
        //$po_item_ids[$po->id] = array_keys($po_items);
      }
      $form_state['values']['vendor_open_pos'] = $pos;
      break;

    case 'pos-selected-submit-btn':
      $selected_pos = empty($form_state['values']['selected_pos']['table']['rows']) ? array() : $form_state['values']['selected_pos']['table']['rows'];
      $receive_pos = array();
      foreach ($selected_pos as $k => $v) {
        $v_values = empty($v['values']) ? array() : $v['values'];
        $will_receive = empty($v_values['action']) ? FALSE : ($v_values['action'] == 'receive');
        if (!$will_receive || empty($v_values['po']->id)) {
          continue;
        }
        $receive_pos[$k] = $v_values['po'];
      }

      // Add received po items into product rows.
      $remove_empty_product_rows = FALSE;
      foreach ($receive_pos as $v) {
        $v_items = empty($v->items) ? array() : $v->items;
        foreach ($v_items as $v_item) {
          $v_item_arr = (array) $v_item;
          $v_item_arr['po_item_id'] = $v_item->id;
          $v_item_arr['product_id'] = $v_item->product_title;
          $v_item_arr['uom_id'] = $v_item->uom_title;
          $v_item_arr['price'] = number_format($v_item->cost, 2, '.', '');
          $v_item_arr['amount'] = number_format($v_item->total, 2, '.', '');
          $form_state['input']['product_rows'][] = $v_item_arr;
          $remove_empty_product_rows = TRUE;
        }
      }

      if ($remove_empty_product_rows) {
        foreach ($form_state['input']['product_rows'] as $k => $v) {
          $remove = TRUE;
          foreach ($v as $v_v) {
            if (trim($v_v) != '') {
              $remove = FALSE;
              break;
            }
          }
          if ($remove) {
            unset($form_state['input']['product_rows'][$k]);
          }
        }
      }
      break;
  }
}

/**
 * Product Cost Changes form fields.
 * @param $form_state
 * @param $row
 *
 * @return mixed
 */
function avpurchase_gr_form_cost_changes_fields(&$form_state, $row) {
  $detected_cost_changes = empty($form_state['values']['detected_cost_changes']) ? array() : $form_state['values']['detected_cost_changes'];
  $fields = array();
  $fields['product_cost_changes'] = array('#tree' => TRUE);
  $table_rows = array();
  foreach ($detected_cost_changes as $k => $v) {
    $table_cols = array();
    $current_cost = $v['current_cost'];
    $current_sales_price = $v['current_sales_price'];
    $proposed_cost = $v['proposed_cost'];
    $proposed_sales_price = $v['proposed_sales_price'];
    $price_markup = $v['price_markup'];
    $direction_class = array('arrow-up' => 'uk-text-primary', 'arrow-down' => 'uk-text-warning');

    $table_cols['product_title'] = array(
      '#title' => 'Product',
      '#title_display' => 'none',
      '#type' => 'item',
      '#markup' => check_plain($v['title']),
      '#column_attributes' => array('class' => array('uk-width-1-5')),
    );

    $current_cost_formatted = number_format($current_cost, 2);
    $proposed_cost_formatted = number_format($proposed_cost, 2);
    $direction = ($proposed_cost > $current_cost) ? 'arrow-up' : 'arrow-down';
    $table_cols['cost'] = array(
      '#title' => 'Proposed cost change',
      '#title_display' => 'none',
      '#type' => 'item',
      '#markup' => t('!direction from @current_cost to @proposed_cost', array('!direction' => "<i class=\"uk-icon-{$direction}\"></i>", '@current_cost' => $current_cost_formatted, '@proposed_cost' => $proposed_cost_formatted)),
      '#column_attributes' => array('class' => array('uk-width-1-5', $direction_class[$direction])),
    );

    $current_sales_price_formatted = number_format($current_sales_price, 2);
    $proposed_sales_price_formatted = number_format($proposed_sales_price, 2);
    $direction = ($proposed_sales_price > $current_sales_price) ? 'arrow-up' : 'arrow-down';
    $table_cols['sales_price'] = array(
      '#title' => 'Proposed sales price change',
      '#title_display' => 'none',
      '#type' => 'item',
      '#markup' => t('!direction from @current_sales_price to @proposed_sales_price', array('!direction' => "<i class=\"uk-icon-{$direction}\"></i>", '@current_sales_price' => $current_sales_price_formatted, '@proposed_sales_price' => $proposed_sales_price_formatted)),
      '#column_attributes' => array('class' => array("uk-width-1-5", $direction_class[$direction])),
    );
    $table_cols['values'] = array(
      'action' => array(
        '#type' => 'radios',
        '#title' => t('Action'),
        '#title_display' => 'none',
        '#options' => array(
          'ignore' => t('Leave unchanged'),
          'cost' => t('Update cost'),
          'cost_and_price' => t('Update cost and sales price'),
        ),
        '#default_value' => 'ignore',
        '#attributes' => array('class' => array('uk-button-group')),
      ),
      'new_cost' => array('#type' => 'value', '#value' => $proposed_cost),
      'new_sales_price' => array('#type' => 'value', '#value' => $proposed_sales_price),
      '#column_attributes' => array('class' => array('uk-width-2-5')),
    );

    $table_rows[$k] = $table_cols;
    $table_rows[$k]['#row_attributes'] = array('class' => array('uk-table-middle'));
  }

  if (!empty($table_rows)) {
    $table = array(
      '#theme' => 'avbase_form_table',
      '#header' => array(
        array('data' => t('Product')),
        array('data' => t('Change on cost')),
        array('data' => t('Change on sales price')),
        '&nbsp;',
      ),
      //'product_rows' => array('#tree' => TRUE),
    );
    $table['rows'] = $table_rows;
    $table['#attributes'] = array('class' => array('uk-table-striped', 'uk-table-condensed'));

    //$fields['product_cost_changes'] += $pcfields;
    //$fields['intro'] = array('#weight' => -5, '#markup' => '<p>Some product costs have been changed. Do you want to update?</p>');
    $fields['product_cost_changes'] += $table;
  }

  // Buttons
  $fields['buttons']['submit'] = array(
    '#id' => 'submit-btn',
    '#type' => 'submit',
    '#value' => t('Continue'),
    '#attributes' => array(
      'class' => array('av-ajax-trigger', 'uk-button-primary'),
    ),
    '#ajax' => array(
      'callback' => 'avpurchase_gr_form_ajax',
      //'wrapper' => 'avpurchase-gr-cost-changed-wrapper',
      'wrapper' => 'avpurchase-gr-form-wrapper',
      'effect' => 'none',
      'event' => 'click',
    ),
  );
  $fields['buttons']['cancel'] = array(
    '#id' => 'cost-changed-cancel-btn',
    '#type' => 'submit',
    //'#submit' => array('avpurchase_gr_form_ajax_submit'),
    '#value' => t('Cancel'),
    //'#validate' => array('avx'),
    '#attributes' => array(
      'class' => array('uk-modal-close'),
    ),
  );
  return $fields;
}

/**
 * Fields for Vendor Pending purchase orders.
 *
 * @param $form_state
 * @param $row
 *
 * @return array
 */
function avpurchase_gr_form_vendor_pos_fields(&$form_state, $row) {
  $fields = array();
  $fields['selected_pos'] = array(
    '#type' => 'container',
    '#attributes' => array(
      'id' => 'avpurchase-gr-form-vendor-pos-wrapper',
    ),
    '#tree' => TRUE,
  );

  $vendor_open_pos = empty($form_state['values']['vendor_open_pos']) ? array() : $form_state['values']['vendor_open_pos'];
  $table_rows = array();
  foreach ($vendor_open_pos as $k => $v) {
    $table_cols = array();
    $table_cols['po_date'] = array(
      '#title' => 'PO Date',
      '#title_display' => 'none',
      '#type' => 'item',
      '#markup' => format_date($v->po_date, 'custom', 'Y - F - d'),
      '#column_attributes' => array('class' => array('uk-width-4-10')),
    );
    $table_cols['total'] = array(
      '#title' => 'Total',
      '#title_display' => 'none',
      '#type' => 'item',
      '#markup' => number_format($v->grand_total, 2),
      '#column_attributes' => array('class' => array('uk-width-3-10', 'uk-text-right')),
    );
    $table_cols['po_num'] = array(
      '#title' => 'PO #',
      '#title_display' => 'none',
      '#type' => 'item',
      '#markup' => t('%po_num', array('%po_num' => $v->id)),
      '#column_attributes' => array('class' => array('uk-width-1-10', 'uk-text-right')),
    );
    $table_cols['values'] = array(
      'po' => array('#type' => 'value', '#value' => $v),
      'action' => array(
        '#type' => 'radios',
        '#title' => t('Action'),
        '#title_display' => 'none',
        '#options' => array(
          'ignore' => '<i class="uk-icon-ban"></i>',
          'receive' => '<i class="uk-icon-check"></i>',
        ),
        '#default_value' => 'ignore',
        '#attributes' => array('class' => array('uk-button-group')),
      ),
      //'new_cost' => array('#type' => 'value', '#value' => $proposed_cost),
      //'new_sales_price' => array('#type' => 'value', '#value' => $proposed_sales_price),
      '#column_attributes' => array('class' => array('uk-width-2-5', 'uk-text-center')),
    );
    $table_rows[$k] = $table_cols;
    $table_rows[$k]['#row_attributes'] = array('class' => array('uk-table-middle'));
  }

  if (!empty($table_rows)) {
    $table = array(
      '#theme' => 'avbase_form_table',
      '#header' => array(
        array('data' => t('P.O. Date')),
        array('data' => t('Total'), 'class' => 'uk-text-right'),
        array('data' => t('P.O. #'), 'class' => 'uk-text-right'),
        '&nbsp;',
      ),
      //'product_rows' => array('#tree' => TRUE),
    );
    $table['rows'] = $table_rows;
    $table['#attributes'] = array('class' => array('uk-table-striped', 'uk-table-condensed'));

    //$fields['product_cost_changes'] += $pcfields;
    //$fields['intro'] = array('#weight' => -5, '#markup' => '<p>Some product costs have been changed. Do you want to update?</p>');
    $fields['selected_pos']['table'] = $table;
  }

  // Buttons
  $fields['buttons']['submit'] = array(
    '#id' => 'pos-selected-submit-btn',
    '#type' => 'submit',
    '#value' => t('Continue'),
    '#attributes' => array(
      'class' => array('av-ajax-trigger', 'uk-button-primary'),
    ),
    '#submit' => array('avpurchase_gr_vendor_pos_submit'),
    '#limit_validation_errors' => array(array('selected_pos')),
    '#ajax' => array(
      'callback' => 'avpurchase_gr_form_ajax',
      //'wrapper' => 'avpurchase-gr-cost-changed-wrapper',
      'wrapper' => 'avpurchase-gr-form-wrapper',
      'effect' => 'none',
      'event' => 'click',
    ),
  );
  $fields['buttons']['cancel'] = array(
    '#id' => 'po-check-cancel-btn',
    '#type' => 'submit',
    '#value' => t('Cancel'),
    '#attributes' => array(
      'class' => array('uk-modal-close'),
    ),
  );
  return $fields;
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
  //$form['#submit'] = array('avbase_crud_form_submit');

  $form['table_name'] = array('#type' => 'value', '#value' => $table_name);
  $form['entity_name'] = array('#type' => 'value', '#value' => 'gr');

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
 * Generic GR form fields.
 * @param $form
 * @param $form_state
 * @param $row
 */
function avpurchase_gr_form_fields(&$form, &$form_state, $row) {
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
    '#avbase_payment_terms_js' => array('preventChildrenHide' => TRUE),
  );
  //$terms = avbase_get_categories('payment_term');
  //$terms_js_settings = array('avbasePaymentTerms' => $terms);
  //$form['term_id']['#attached']['js'] = array(
  //  AVVENDORS_MODULE_PATH . '/js/payment_terms.js',
  //  array(
  //    'data' => $terms_js_settings,
  //    'type' => 'setting',
  //  ),
  //);
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

      // Update product on-hand qty.
      if (!empty($incoming_qty)) {
        $query = "UPDATE {avtbl_products} SET qty = qty + CASE id";
        foreach ($incoming_qty as $k => $v) {
          $query .= " WHEN $k THEN $v";
        }
        $query .= " END WHERE id IN (" . implode(', ', array_keys($incoming_qty)) . ")";
        if (db_query($query)) {
          avbase_custom_watchdog("gr__qty_update", "ID: {$row['id']}", array('#row' => $row, '#incoming_qty' => $incoming_qty, '#entity_name' => 'gr'));
        }

        // Update cost and price.
        $cost_changes = empty($values['product_cost_changes']) ? array() : $values['product_cost_changes'];
        $cost_set = array();
        $price_set = array();
        $cost_args = array();
        $price_args = array();
        foreach (array_keys($incoming_qty) as $k) {
          $changes = empty($cost_changes['rows'][$k]['values']) ? array() : $cost_changes['rows'][$k]['values'];
          switch ($changes['action']) {
            case 'cost':
            case 'cost_and_price':
              $cost_set[$k] = "WHEN $k THEN :cost{$k}";
              $cost_args[":cost{$k}"] = $changes['new_cost'];
              if ($changes['action'] == 'cost_and_price') {
                $price_set[$k] = "WHEN $k THEN :price{$k}";
                $price_args[":price{$k}"] = $changes['new_sales_price'];
              }
              break;
            default:
              continue;
              break;
          }
        }

        if (!empty($cost_set)) {
          $query = "UPDATE {avtbl_products} SET cost = CASE id " . implode(' ', $cost_set) . " END WHERE ID IN(:product_ids)";
          $cost_args[':product_ids'] = array_keys($cost_set);
          if (db_query($query, $cost_args)) {
            avbase_custom_watchdog("gr__cost_update", "ID: {$row['id']}", array('#row' => $row, '#cost_changes' => $cost_changes, '#cost_set' => $cost_set, '#args' => $cost_args));
          }
        }
        if (!empty($price_set)) {
          $query = "UPDATE {avtbl_products} SET price = CASE id " . implode(' ', $price_set) . " END WHERE ID IN(:product_ids)";
          $price_args[':product_ids'] = array_keys($price_set);
          if (db_query($query, $price_args)) {
            avbase_custom_watchdog("gr__price_update", "ID: {$row['id']}", array('#row' => $row, '#cost_changes' => $cost_changes, '#price_set' => $price_set, '#args' => $price_args));
          }
        }
      }
      break;
  }
  return NULL;
}

/**
 * Ajax callback before submitting a Receive Items form.
 */
function avpurchase_gr_form_ajax($form, $form_state) {
  if (form_get_errors()) {
    // Prevent modal from popping out.
    unset($form['cost_changes']['product_cost_changes']);
  }
  return $form;
}

/**
 * Submit handler for Goods Received form.
 * @param $form
 * @param $form_state
 */
function avpurchase_gr_form_submit($form, &$form_state) {
  $form_state['rebuild'] = TRUE;
  $trigger = $form_state['triggering_element'];
  switch ($trigger['#id']) {
    case 'detect-cost-change-btn':
      dpm($form_state);
      $product_rows = empty($form_state['values']['product_rows']) ? array() : $form_state['values']['product_rows'];
      $entered_products = array();
      foreach ($product_rows as $row) {
        if (empty($row['product_id'])) {
          continue;
        }
        $entered_products[$row['product_id']] = $row;
      }

      $product_details = array();
      if (!empty($entered_products)) {
        $product_ids = array_keys($entered_products);
        $product_details = avproduct_load_ids($product_ids);
      }

      $cost_changes = array();
      foreach ($product_details as $v) {
        $entered_product = $entered_products[$v->id];
        $incoming_qty = $entered_product['qty'] * $entered_product['qty_per_uom'];
        $current_cost = (float) $v->cost;
        $current_sales_price = (float) $v->price;
        $new_cost = $entered_product['price'] / $entered_product['qty_per_uom'];
        $new_cost = round((float) $new_cost, AVBASE_MONEY_DECIMAL_MAX);

        if (round($current_cost, 2) == round($new_cost, 2)) {
          continue;
        }

        $proposed_cost = avpurchase_compute_product_cost($current_cost, $v->qty, $new_cost, $incoming_qty);
        $price = avpurchase_compute_sales_price($current_sales_price, $current_cost, $proposed_cost);
        $price_markup = $price['markup'] * 100;
        $proposed_sales_price = $price['price'];
        $cost_changes[$v->id] = array(
          'title' => check_plain($v->title),
          'current_cost' => $current_cost,
          'proposed_cost' => $proposed_cost,
          'current_sales_price' => $current_sales_price,
          'proposed_sales_price' => $proposed_sales_price,
          'price_markup' => $price_markup,
        );
      }

      if (!empty($cost_changes)) {
        $form_state['values']['detected_cost_changes'] = $cost_changes;
        return;
      }
      break;
  }

  avbase_crud_form_submit($form, $form_state);
  // Reset form.
  unset($form_state['input']);
  $form_state['values'] = array();
}
