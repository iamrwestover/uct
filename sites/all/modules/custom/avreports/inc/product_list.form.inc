<?php

function avreports_product_list_form($form, &$form_state, $complete_columns = TRUE, $selectable_list = FALSE, $default_order_key = 'title', $limit = NULL, $offset = NULL) {

    // Filter form.
  $filter_form_settings = array(
    'avclients.display_name' => array(
      '#title' => 'Preferred Vendor',
      '#autocomplete_path' => 'av/clients/autocomplete/' . AVCLIENT_TYPE_VENDOR . '/' . implode('-', array(AVCLIENT_STATUS_ACTIVE, AVCLIENT_STATUS_SUSPENDED)),
    ),
    'avcat_principal.title' => array(
      '#title' => t('Principal'),
      '#autocomplete_path' => 'av/categories/principal/autocomplete',
    ),
    'stock_status' => array('#default_value' => ($complete_columns ? 'all' : 'os')),
    'reset_button' => array('#attributes' => array('style' => 'margin-top: 25px;')),
    'export_button' => array(
      '#func_name' => __FUNCTION__ ,
      '#func_args' => array($complete_columns, $selectable_list, $default_order_key),
      '#report_name' => 'Product List',
      '#report_inc' => 'product_list.form',
      '#access' => $complete_columns,
    ),
  );
  $filter_form_settings['avcat_prodcat.id'] = array('#options' => avbase_get_category_options_array('product_cat'));
  $filter_form_settings['product_status'] = array('#default_value' => AVPRODUCTS_STATUS_ACTIVE);
  $form['transaction_list_filter'] = avtxns_txn_list_filter_form($form, $form_state, $filter_form_settings);

  // Prepare query variables.
  $select = $joins = $where = $args = $group_by = $table_rows = array();
  $user_filter = empty($form_state['values']['avtxns_tl_filter']) ? array() : $form_state['values']['avtxns_tl_filter'];
  $filter_query = empty($user_filter['filter_query']) ? array() : $user_filter['filter_query'];
  $where = empty($filter_query['where']) ? array() : $filter_query['where'];
  $args = empty($filter_query['args']) ? array() : $filter_query['args'];


  // Show hide columns depending on selected filters.
  $col_vendor_show = empty($user_filter['avclients.display_name']);
  $col_principal_show = empty($user_filter['avcat_principal.title']);
  $col_category_show = empty($user_filter['avcat_prodcat.id']);

  $header = array();
  $header['title'] = array('data' => 'TITLE', 'field' => 'title');
  if ($complete_columns) {
    $header['uom'] = array('data' => 'BASE UOM', 'field' => 'uom_title');
  }
  $header['onhand'] = array('data' => 'ON HAND', 'class' => array('uk-text-right'), 'field' => 'qty');
  if ($complete_columns) {
    $header['cost'] = array('data' => 'COST', 'class' => array('uk-text-right'), 'field' => 'cost');
    $header['price'] = array('data' => 'SALES PRICE', 'class' => array('uk-text-right'), 'field' => 'price');
    $header['rop'] = array('data' => 'ROP', 'class' => array('uk-text-right'), 'field' => 'rop');
    $header['eoq'] = array('data' => 'EOQ', 'class' => array('uk-text-right'), 'field' => 'eoq');
    if ($col_category_show) {
      $header['category'] = array('data' => 'CATEGORY', 'field' => 'category_name');
    }
    $header['shelf'] = array('data' => 'SHELF LOCATION', 'field' => 'shelf_name');
    $header['discount'] = array('data' => 'DISCOUNT', 'class' => array('uk-text-center'));
  }

  if ($col_vendor_show) {
    $header['preferred_vendor'] = array('data' => 'PREFERRED VENDOR', 'field' => 'avclients.display_name');
  }

  if ($complete_columns) {
    if ($col_principal_show) {
      $header['principal'] = array('data' => 'PRINCIPAL', 'field' => 'principal_name');
    }
  }



  // Set table pagination and sort settings.
  if (!empty($user_filter['settings']['table_href'])) {
    $table_ajax_href = $user_filter['settings']['table_href'];
    $href_parts = parse_url($table_ajax_href);
    if (!empty($href_parts['query'])) {
      parse_str($href_parts['query'], $href_query);
      foreach ($href_query as $k => $v) {
        $_GET[$k] = $v;
      }
    }
  }

  // Set default order.
  if (empty($_GET['order'])) {
    $_GET['order'] = $header[$default_order_key]['data'];
    $_GET['sort'] = 'ASC';
  }
  $order_by = avbase_build_header_order_array($header);


  // Build query.
  $select[] = 'avprod.*';
  $select[] = 'avcat_uom.title AS uom_title';
  $select[] = 'avcat_prodcat.title AS category_name';
  $select[] = 'avcat_shelf.title AS shelf_name';
  $select[] = 'avcat_principal.title AS principal_name';
  $select[] = 'avclients.display_name AS vendor_name';
  $joins[] = 'LEFT JOIN {avtbl_categories} AS avcat_uom ON avcat_uom.id = avprod.uom_id';
  $joins[] = 'LEFT JOIN {avtbl_categories} AS avcat_prodcat ON avcat_prodcat.id = avprod.category_id';
  $joins[] = 'LEFT JOIN {avtbl_categories} AS avcat_shelf ON avcat_shelf.id = avprod.shelf_id';
  $joins[] = 'LEFT JOIN {avtbl_categories} AS avcat_principal ON avcat_principal.id = avprod.principal_id';
  $joins[] = 'LEFT JOIN {avtbl_clients} AS avclients ON avclients.id = avprod.vendor_id';

  switch ($user_filter['settings']['stock_status']) {
    case 'low':
      $where[] = '(avprod.qty < 1 OR avprod.rop <= avprod.qty)';
      break;
    case 'os':
      $where[] = 'avprod.qty < 1';
      break;
  }

  //$where[] = 'avclients.client_type = :ctype';
  //$args[':ctype'] = $client_type;
  $query = "SELECT _expression_ FROM {avtbl_products} AS avprod " . implode(' ', $joins);
  // Count max rows before applying supplements to $query.
  avreports_count_max_rows($form, $query, $select, $where, $group_by, $args, $offset);
  avbase_query_supplements($query, $where, $order_by, $limit, $offset, $group_by);
  $rs = db_query($query, $args);
  foreach ($rs as $row) {
    $data = array();
    $data['title'] = l($row->title, "av/products/{$row->id}/view");
    if ($complete_columns) {
      $data['uom'] = check_plain($row->uom_title);
    }
    $data['onhand'] = '<div class="uk-text-right">' . number_format($row->qty) . '</div>';
    if ($complete_columns) {
      $data['cost'] = '<div class="uk-text-right">' . number_format($row->cost, 2) . '</div>';
      $data['price'] = '<div class="uk-text-right">' . number_format($row->price, 2) . '</div>';
      $data['rop'] = '<div class="uk-text-right">' . number_format($row->rop) . '</div>';
      $data['eoq'] = '<div class="uk-text-right">' . number_format($row->eoq) . '</div>';
      if ($col_category_show) {
        $data['category'] = check_plain($row->category_name);
      }
      $data['shelf'] = check_plain($row->shelf_name);

      $discount = array();
      for ($x = 1; $x <= 4; $x++) {
        $_discount_name = "discount$x";
        if (isset($row->$_discount_name)) {
          $discount[] = round($row->$_discount_name, 2);
        }
      }
      $discount = array_filter($discount);
      $data['discount'] = '<div class="uk-text-center">' . implode('/', $discount) . '</div>';
    }

    if ($col_vendor_show) {
      $data['preferred_vendor'] = l($row->vendor_name, 'av/vendors/' . $row->vendor_id . '/view');
    }

    if ($complete_columns) {
      if ($col_principal_show) {
        $data['principal'] = check_plain($row->principal_name);
      }
    }


    // Careful when adding classes on rows, check if tableselect mode is ON.
    $row_class = array();
    $psd = avproducts_product_status_details($row->status);
    if (!empty($psd['color_class'])) {
      $row_class[] = $psd['color_class'];
    }
    if ($selectable_list) {
      $data['#attributes']['class'] = $row_class;
      $table_rows[$row->id] = $data;
    }
    else {
      $data['#avclass'] = implode(' ', $row_class);
      $table_rows[] = $data;
    }
  }

  $transaction_list_table['table'] = array(
    '#header' => $header,
    '#empty' => '<div class="uk-text-muted">' . t('No match found.') . '</div>',
    '#attributes' => array(
      'id' => 'transaction-list-table',
      'class' => array('uk-table-condensed uk-table-hover uk-text-small'),
    ),
  );
  if ($selectable_list) {
    $transaction_list_table['table']['#type'] = 'tableselect';
    $transaction_list_table['table']['#options'] = $table_rows;
    $transaction_list_table['table']['#attributes']['class'][] = 'av-frs';
  }
  else {
    $transaction_list_table['table']['#theme'] = 'table';
    $transaction_list_table['table']['#rows'] = $table_rows;
  }


  if (!empty($user_filter['avclients.display_name'])) {
    $transaction_list_table['client_name'] = array(
      '#type' => 'item',
      '#title' => 'Preferred vendor',
      '#title_display' => 'inline-before',
      '#markup' => check_plain($user_filter['avclients.display_name']),
    );
  }
  if (!empty($user_filter['avcat_principal.title'])) {
    $transaction_list_table['principal_name'] = array(
      '#type' => 'item',
      '#title' => 'Principal',
      '#title_display' => 'inline-before',
      '#markup' => check_plain($user_filter['avcat_principal.title']),
    );
  }
  if (!empty($user_filter['avcat_prodcat.id'])) {
    $transaction_list_table['avcat_prodcat.id'] = array(
      '#type' => 'item',
      '#title' => 'Category',
      '#title_display' => 'inline-before',
      '#markup' => check_plain($filter_form_settings['avcat_prodcat.id']['#options'][$user_filter['avcat_prodcat.id']]),
    );
  }


  $form['transaction_list_table'] = array(
    '#type' => 'container',
    '#theme' => 'avreports_txn_list',
    '#attributes' => array(
      'id' => 'transaction-list-table-wrapper',
      'class' => array('printable av-sticky-small')
    ),
  );
  $form['transaction_list_table'] += $transaction_list_table;

  $form['buttons']['#theme'] = 'avbase_crud_button_group';
  $form['buttons']['print'] = array(
    '#id' => 'av-print-btn',
    '#type' => 'button',
    '#value' => t('Print...'),
    '#visible_in_view_mode' => TRUE,
    '#icon_key' => 'print',
  );

  // Hack to minimize caching size.
  if (empty($transaction_list_table['table']['#type']) || $transaction_list_table['table']['#type'] != 'tableselect') {
    $form['#form_keys_no_cache'] = array('transaction_list_table');
  }

  // Show / hide preview?
  avreports_report_live_preview($form);
  return $form;
}
