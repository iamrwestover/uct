<?php
module_load_include('inc', 'avtrans', 'inc/item_list.handlers');
module_load_include('inc', 'avtrans', 'inc/item_list.form.validate');

/**
 * Item List form - generate info fields.
 * @param $form
 * @param $form_state
 * @param $row
 */
function avtrans_item_list_form_fields(&$form, &$form_state, $row) {
  $form['#attached']['js'][] = AVTRANS_MODULE_PATH . '/js/item_list.form.js';
  $form['client_name'] = array(
    '#type' => 'textfield',
    '#maxlength' => 255,
    '#required' => TRUE,
    '#element_validate' => array('avbase_element_validate_display_name_exists'),
    //'#default_value' => empty($row->vendor_name) ? '' : check_plain($row->vendor_name),
    '#title' => t('Client'),
    '#id' => 'client-id',
    '#element_save_id' => 'client_id',
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

  // Product list.
  avtrans_item_list_product_fields($form, $form_state, $row);
}

/**
 * Item List form - generate product list fields.
 * @param $form
 * @param $form_state
 * @param $row
 */
function avtrans_item_list_product_fields(&$form, &$form_state, $row) {
  // Initialize to-attached js settings.
  $js_settings = array('avbase' => array());

  // Set extra form validation for product list.
  $form['#validate'][] = 'avtrans_item_list_fields_validate';

  // Get form storage.
  $form_state['storage'] = isset($form_state['storage']) ? $form_state['storage'] : array();
  $_storage = &$form_state['storage'];

  // Get transaction type.
  $transaction_type = empty($_storage['transaction_type']) ? 'gr' : $_storage['transaction_type'];

  // Get item rows.
  $_storage['item_rows'] = isset($_storage['item_rows']) ? $_storage['item_rows'] : array();
  $_item_rows = &$_storage['item_rows'];

  // Pre-load empty product rows.
  if (empty($_item_rows)) {
    $starting_empty_row_count = 3;
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
    '#theme' => 'avbase_nestable_form',
  );
  foreach ($_item_rows as $prod_key => $prod_row) {
    $prod_index++;
    $fields = array();
    $order_key = $transaction_type == 'gr' ? 'po_id' : 'so_id';
    $order_id = empty($prod_row[$order_key]) ? NULL : $prod_row[$order_key];
    if (!empty($order_id)) {
      $badge_title = $transaction_type == 'gr' ? 'PO# ' : 'SO# ';
      $badge_title .= $order_id;
      $fields['badge'] = array('#markup' => '<span class="uk-badge uk-badge-success" style="position: absolute; top: -9px; left: 0;">' . $badge_title . '</span>');
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
    );

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
    );

    $fields['qty_per_uom'] = array(
      '#id' => 'qty-per-uom-' . $prod_key,
      '#item_row_key' => $prod_key,
      '#type' => 'hidden',
      '#title' => 'Qty per UOM',
      '#title_display' => 'none',
      '#maxlength' => 10,
      '#element_validate' => array('avtrans_item_list_row_element_validate'),
      '#av_prod_element_validate' => array('element_validate_integer_positive'),
      '#attributes' => array(
        'class' => array('prod-column-qty-per-uom'),
      ),
    );

    $fields['qty'] = array(
      '#id' => 'product-qty-' . $prod_key,
      '#item_row_key' => $prod_key,
      '#type' => 'textfield',
      '#title' => 'Qty.',
      '#maxlength' => 10,
      '#element_validate' => array('avtrans_item_list_row_element_validate'),
      '#av_prod_element_validate' => array('element_validate_integer_positive', 'avtrans_element_validate_enough_qty'),
      '#attributes' => array(
        'data-column-name' => 'qty',
        'class' => array('prod-column-qty'),
      ),
    );

    $fields['cost'] = array(
      '#id' => 'product-cost-' . $prod_key,
      '#item_row_key' => $prod_key,
      '#type' => 'textfield',
      '#title' => 'Unit Cost',
      '#maxlength' => 19,
      '#element_validate' => array('avtrans_item_list_row_element_validate'),
      '#av_prod_element_validate' => array('element_validate_number', 'avbase_element_validate_money_length'),
      '#attributes' => array(
        'data-column-name' => 'cost',
        'class' => array('prod-column-cost'),
      ),
    );

    $fields['total'] = array(
      '#id' => 'product-total-' . $prod_key,
      '#item_row_key' => $prod_key,
      '#type' => 'textfield',
      '#title' => 'Total',
      '#maxlength' => 19,
      '#element_validate' => array('avtrans_item_list_row_element_validate'),
      '#av_prod_element_validate' => array('element_validate_number', 'avbase_element_validate_money_length'),
      '#attributes' => array(
        'data-column-name' => 'total',
        'class' => array('prod-column-total'),
      ),
    );

    $fields['prod_delete_btn'] = array(
      '#name' => 'prod_delete_btn_' . $prod_key,
      '#item_row_key' => $prod_key,
      '#type' => 'submit',
      '#value' => t('Delete'),
      '#limit_validation_errors' => array(),
      '#submit' => array('avtrans_item_list_prod_submit'),
      '#attributes' => array(
        'class' => array('prod-column-delete-btn'),
        'tabindex' => -1,
      ),
      '#ajax' => array(
        'callback' => 'avtrans_item_list_prod_js',
        'wrapper' => 'item-list-product-row-' . $prod_key . '-wrapper',
        'effect' => 'none',
        'event' => 'click',
      ),
    );

    if (!empty($_item_rows[$prod_key]['product_id'])) {
      $fields['product_title']['#attributes']['data-selected-product-id'] = $_item_rows[$prod_key]['product_id'];
      $detected_product_ids[] = $_item_rows[$prod_key]['product_id'];
    }

    $form['product_rows'][$prod_key] = $fields;
    $form['product_rows'][$prod_key]['#prod_index'] = $prod_index;
    $form['product_rows'][$prod_key]['#theme'] = 'avbase_nestable_form_row';
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
    '#submit' => array('avtrans_item_list_prod_submit'),
    '#ajax' => array(
      'callback' => 'avtrans_item_list_prod_js',
      'wrapper' => 'item-list-new-product-wrapper',
      'effect' => 'none',
      'event' => 'click',
    ),
  );

  $js_settings['avbase']['uoms'] = avbase_get_categories('uom');
  $form['#attached']['js'][] = AVTRANS_MODULE_PATH . '/js/nestable_product_form.js';
  $form['#attached']['js'][] = array(
    'data' => $js_settings,
    'type' => 'setting',
  );
}