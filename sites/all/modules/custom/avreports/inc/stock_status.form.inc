<?php
function avreports_stock_status_form($form, &$form_state, $limit = NULL, $offset = NULL) {
  $dad = avtxns_txn_list_filter_date_auto_options();
  $dad_default = empty($dad['this_week']) ? array() : $dad['this_week'];
  $filter_form_settings = array(
    'date_auto' => array(
      '#title' => t('Dates'),
      '#default_value' => 'this_week',
    ),
    'date_from' => array(
      '#title' => t('Date from'),
      '#default_value' => $dad_default['date_from'],
    ),
    'date_to' => array(
      '#title' => t('Date to'),
      '#default_value' => $dad_default['date_to'],
    ),
    'preferred_vendor' => array(
      '#title' => 'Preferred Vendor',
      '#autocomplete_path' => 'av/clients/autocomplete/' . AVCLIENT_TYPE_VENDOR . '/' . implode('-', array(AVCLIENT_STATUS_ACTIVE, AVCLIENT_STATUS_SUSPENDED)),
    ),
    'principal_name' => array(
      '#title' => t('Principal'),
      '#autocomplete_path' => 'av/categories/principal/autocomplete',
    ),
    // 'all_products' => array('#default_value' => 0),
    'stock_status' => array('#default_value' => 'all'),
    'selectable' => array('#default_value' => 0),
    'reset_button' => array('#attributes' => array('style' => 'margin-top: 25px;')),
    'export_button' => array(
      '#func_name' => __FUNCTION__ ,
      '#func_args' => array(),
      '#report_name' => 'Stock Status',
      '#report_inc' => 'stock_status.form',
    ),
  );
  // $filter_form_settings['product_status'] = array('#default_value' => AVPRODUCTS_STATUS_ACTIVE);
  $form['transaction_list_filter'] = avtxns_txn_list_filter_form($form, $form_state, $filter_form_settings);
  // Prepare query variables.
  $select = $joins = $where = $args = array();
  $user_filter = empty($form_state['values']['avtxns_tl_filter']) ? array() : $form_state['values']['avtxns_tl_filter'];
  $filter_query = empty($user_filter['filter_query']) ? array() : $user_filter['filter_query'];
  $where = empty($filter_query['where']) ? array() : $filter_query['where'];
  $args = empty($filter_query['args']) ? array() : $filter_query['args'];

  // $all_products = !empty($user_filter['settings']['all_products']);
  $selectable_list = !empty($user_filter['settings']['selectable']);

  // if ($all_products) {
  //   unset($where['product_statuses']);
  //   unset($args[':product_statuses']);
  // }

  unset($where['date_to']);
  unset($where['date_from']);
  unset($args[':date_to_stamp']);
  unset($args[':date_from_stamp']);

  $header = array();
  $header['product_title'] = array('data' => 'PRODUCT', 'field' => 'product_title');
  $header['vendor'] = array('data' => 'PREFERRED VENDOR', 'field' => 'vendor_name');
  $header['principal'] = array('data' => 'PRINCIPAL', 'field' => 'principal_name');
  $header['onhand'] = array('data' => 'OHQ', 'field' => 'onhand', 'class' => array('uk-text-right'));
  $header['onhand']['data-uk-tooltip'] = '{cls: "tt-blue"}';
  $header['onhand']['title'] = 'On Hand Quantity';

  $header['uom'] = array('data' => 'U/M', 'field' => 'uom_title');
  $header['uom']['data-uk-tooltip'] = '{cls: "tt-blue"}';
  $header['uom']['title'] = 'Unit of Measure';

  $header['day_ave'] = array('data' => 'ASPD', 'class' => array('uk-text-right'));
  $header['day_ave']['data-uk-tooltip'] = '{cls: "tt-blue"}';
  $header['day_ave']['title'] = 'Average Sales Per Day';

  $header['dr_total'] = array('data' => 'OUT(D)', 'class' => array('uk-text-right'));
  $header['dr_total']['data-uk-tooltip'] = '{cls: "tt-blue"}';
  $header['dr_total']['title'] = 'Out (Deliveries)';

  $header['stou_total'] = array('data' => 'OUT(T)', 'class' => array('uk-text-right'));
  $header['stou_total']['data-uk-tooltip'] = '{cls: "tt-blue"}';
  $header['stou_total']['title'] = 'Out (Transfers)';

  $header['so_total'] = array('data' => 'SO', 'class' => array('uk-text-right'));
  $header['so_total']['data-uk-tooltip'] = '{cls: "tt-blue"}';
  $header['so_total']['title'] = 'On Sales Order';

  $header['po_total'] = array('data' => 'PO', 'class' => array('uk-text-right'));
  $header['po_total']['data-uk-tooltip'] = '{cls: "tt-blue"}';
  $header['po_total']['title'] = 'On Purchase Order';

  $header['rop'] = array('data' => 'ROP', 'field' => 'rop', 'class' => array('uk-text-right'));
  $header['rop']['data-uk-tooltip'] = '{cls: "tt-blue"}';
  $header['rop']['title'] = 'Reorder Point';

  $header['eoq'] = array('data' => 'EOQ', 'field' => 'eoq', 'class' => array('uk-text-right'));
  $header['eoq']['data-uk-tooltip'] = '{cls: "tt-blue"}';
  $header['eoq']['title'] = 'Economic Ordery Quantity';

  $header['cost'] = array('data' => 'COST', 'field' => 'cost', 'class' => array('uk-text-right'));
  $header['price'] = array('data' => 'PRICE', 'field' => 'price', 'class' => array('uk-text-right'));


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
    $_GET['order'] = $header['onhand']['data'];
    $_GET['sort'] = 'ASC';
  }
  $order_by = avbase_build_header_order_array($header);

  // Supporting order by if ordered by On Hand.
  // if (isset($order_by['onhand'])) {
  //   $order_by['day_ave'] = 'day_ave DESC';
  //   $order_by['so_total'] = 'so_total DESC';
  //   $order_by['po_total'] = 'po_total ASC';
  // }

  //$now = time(); // or your date as well
  //$your_date = strtotime("2010-01-01");
  //$datediff = $now - $your_date;
  $date_from = strtotime($user_filter['date_from']);
  $date_to = strtotime($user_filter['date_to']);
  $date_diff = $date_to - $date_from;
  $day_diff = floor($date_diff / (60 * 60 * 24));
  $day_diff = $day_diff < 1 ? 1 : $day_diff;

  // $products_join = $all_products ? 'LEFT JOIN' : 'INNER JOIN';
  $products_join = 'LEFT JOIN';
  $dr_total_query = 'SUM(IF(avtxn.transaction_type = :dr_type, (avtxn_detail.qty * avtxn_detail.qty_per_uom), NULL))';
  $select[] = 'avprod.id AS product_id';
  $select[] = 'avprod.title AS product_title';
  $select[] = 'avprod.qty AS onhand';
  $select[] = 'avprod.rop';
  $select[] = 'avprod.eoq';
  $select[] = 'avprod.cost';
  $select[] = 'avprod.price';
  $select[] = 'avprod.status';
  // $select[] = 'SUM(IF(avtxn.transaction_type = :po_type && avtxn.status = :open_status, (avtxn_detail.qty * avtxn_detail.qty_per_uom), NULL)) AS po_total';
  $select[] = '11 AS po_total';
  // $select[] = 'SUM(IF(avtxn.transaction_type = :so_type && avtxn.status = :open_status, (avtxn_detail.qty * avtxn_detail.qty_per_uom), NULL)) AS so_total';
  $select[] = '22 AS so_total';
  // $select[] = $dr_total_query . ' AS dr_total';
  $select[] = '33 AS dr_total';
  // $select[] = 'SUM(IF(avtxn.transaction_type = :stou_type, (avtxn_detail.qty * avtxn_detail.qty_per_uom), NULL)) AS stou_total';
  $select[] = '44 AS stou_total';
  // $select[] = "($dr_total_query / $day_diff) AS day_ave";
  $select[] = "55 AS day_ave";
  $select[] = 'avvendor.display_name AS vendor_name';
  $select[] = 'avcat_uom.title AS uom_title';
  $select[] = 'avcat_principal.title AS principal_name';
  // $joins['avtxn_detail'] = $products_join . ' {avtbl_transaction_details} AS avtxn_detail ON avtxn_detail.item_id = avprod.id';
  // $joins['avtxn'] = $products_join . ' {avtbl_transactions} AS avtxn ON avtxn.id = avtxn_detail.transaction_id AND avtxn.status <> :void_type AND avtxn.transaction_type IN (:ttypes)';
  // if (!empty($where)) {
  //   $joins['avtxn'] .= ' AND ' . implode(' AND ', $where);
  //   $where = array();
  // }
  $joins[] = 'LEFT JOIN {avtbl_clients} AS avvendor ON avvendor.id = avprod.vendor_id';
  $joins[] = 'LEFT JOIN {avtbl_categories} AS avcat_uom ON avcat_uom.id = avprod.uom_id';
  $joins[] = 'LEFT JOIN {avtbl_categories} AS avcat_principal ON avcat_principal.id = avprod.principal_id';

  $select_args[':po_type'] = AVTXNS_TXN_TYPE_PURCHASE_ORDER;
  $select_args[':so_type'] = AVTXNS_TXN_TYPE_SALES_ORDER;
  $select_args[':dr_type'] = AVTXNS_TXN_TYPE_DELIVERY;
  $select_args[':stou_type'] = AVTXNS_TXN_TYPE_STOCK_TRANSFER_OUT;
  $select_args[':open_status'] = AVTXNS_TXN_STATUS_OPEN;
  $select_args = array();
  // $args[':void_type'] = AVTXNS_TXN_STATUS_VOID;
  // $args[':ttypes'] = array(AVTXNS_TXN_TYPE_PURCHASE_ORDER, AVTXNS_TXN_TYPE_SALES_ORDER, AVTXNS_TXN_TYPE_DELIVERY, AVTXNS_TXN_TYPE_STOCK_TRANSFER_OUT);
  $group_by = array();
  // $group_by[] = 'avprod.id';

  switch ($user_filter['settings']['stock_status']) {
    case 'low':
      $where[] = '(avprod.qty < 1 OR avprod.rop >= avprod.qty)';
      break;
    case 'os':
      $where[] = 'avprod.qty < 1';
      break;
  }
  if (!empty($user_filter['settings']['principal_name'])) {
    // $joins[] = 'INNER JOIN {avtbl_categories} AS avcat_principal ON avcat_principal.id = avprod.principal_id AND avcat_principal.title = :principal_name';
    $where[] = 'avcat_principal.title = :principal_name';
    $args[':principal_name'] = $user_filter['settings']['principal_name'];
  }
  if (!empty($user_filter['settings']['preferred_vendor'])) {
    $where[] = 'avvendor.display_name = :preferred_vendor';
    $args[':preferred_vendor'] = $user_filter['settings']['preferred_vendor'];
  }
  $where[] = 'avprod.status = :active_prod';
  $args[':active_prod'] = AVPRODUCTS_STATUS_ACTIVE;
  $query = "SELECT _expression_ FROM {avtbl_products} AS avprod " . implode(' ', $joins);
  // Count max rows before applying supplements to $query.
  avreports_count_max_rows($form, $query, $select, $where, $group_by, $args, $offset);
  avbase_query_supplements($query, $where, $order_by, $limit, $offset, $group_by);
  // avreports_count_max_rows_stock_status($form, $query, $select, $where, $group_by, $args, $offset, $joins, $all_products);
  // avbase_query_supplements_stock_status($query, $where, $order_by, $limit, $offset, $group_by, $joins, $all_products, $args);

  $args = array_merge($args, $select_args);
  $rs = db_query($query, $args);

  $table_rows = array();
  foreach ($rs as $row) {
    $data = array();
    // $data['product_title'] = l($row->product_title, 'av/products/' . $row->product_id . '/view');
    $data['product_title'] = check_plain($row->product_title);
    $data['vendor'] = check_plain($row->vendor_name);
    $data['principal'] = check_plain($row->principal_name);
    $data['onhand'] = '<div class="uk-text-right">' . number_format($row->onhand) . '</div>';
    $data['uom'] = check_plain($row->uom_title);
    $data['day_ave'] = '<div class="uk-text-right">0</div>';
    $data['dr_total'] = '<div class="uk-text-right">0</div>';
    $data['stou_total'] = '<div class="uk-text-right">0</div>';
    $data['so_total'] = '<div class="uk-text-right">0</div>';
    $data['po_total'] = '<div class="uk-text-right">0</div>';
    $data['rop'] = '<div class="uk-text-right">' . number_format($row->rop) . '</div>';
    $data['eoq'] = '<div class="uk-text-right">' . number_format($row->eoq) . '</div>';
    $data['cost'] = '<div class="uk-text-right">' . number_format($row->cost, 2) . '</div>';
    $data['price'] = '<div class="uk-text-right">' . number_format($row->price, 2) . '</div>';

    // Careful when adding classes on rows, check if tableselect mode is ON.
    $row_class = array();
    $psd = avproducts_product_status_details($row->status);
    if (!empty($psd['color_class'])) {
      $row_class[] = $psd['color_class'];
    }
    if ($selectable_list) {
      $data['#attributes']['class'] = $row_class;
      // $table_rows[$row->id] = $data;
    }
    else {
      $data['#avclass'] = implode(' ', $row_class);
      // $table_rows[] = $data;
    }

    $table_rows[$row->product_id] = $data;
  }

  avreports_stock_status_txn_total($table_rows, AVTXNS_TXN_TYPE_DELIVERY, $filter_query);
  avreports_stock_status_txn_total($table_rows, AVTXNS_TXN_TYPE_STOCK_TRANSFER_OUT, $filter_query);
  avreports_stock_status_txn_total($table_rows, AVTXNS_TXN_TYPE_PURCHASE_ORDER, $filter_query);
  avreports_stock_status_txn_total($table_rows, AVTXNS_TXN_TYPE_SALES_ORDER, $filter_query);


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
  avtxns_txn_list_table_add_date_from($transaction_list_table, $user_filter);
  if (!empty($user_filter['settings']['principal_name'])) {
    $transaction_list_table['principal_name'] = array(
      '#type' => 'item',
      '#title' => 'Principal',
      '#title_display' => 'inline-before',
      '#markup' => check_plain($user_filter['settings']['principal_name']),
    );
  }

  // Button.
  if ($selectable_list) {
    $transaction_list_table['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Order selected products'),
      '#attributes' => array(
        'class' => array('uk-button-primary uk-margin-top'),
        'data-ks' => 'CTRL+D',
      ),
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

function avreports_stock_status_form_validate($form, &$form_state) {
  $ids = empty($form_state['values']['table']) ? array() : $form_state['values']['table'];

  $selected = array();
  foreach ($ids as $k => $v) {
    if (empty($v)) {
      continue;
    }
    $selected[] = $v;
  }

  if (empty($selected)) {
    form_set_error('', t('Please select at least 1 product.'));
    return;
  }

  if (count($selected) > AVTXNS_TXN_ROW_LIMIT) {
    form_set_error('', t('Maximum number of items allowed per transaction is %max.', array('%max' => AVTXNS_TXN_ROW_LIMIT)));
    return;
  }
}

function avreports_stock_status_form_submit($form, &$form_state) {
  $ids = empty($form_state['values']['table']) ? array() : $form_state['values']['table'];


  $product_rows = avproduct_load_ids($ids);
  $item_rows = array();
  $i = 0;
  foreach ($product_rows as $v) {
    $item_rows['new_' . $i++] = array(
      'item_id' => $v->id,
      'product_title' => $v->title,
      'uom_title' => $v->uom_title,
      'qty_per_uom' => 1,
      'qty' => $v->eoq,
      'cost' => $v->cost,
      'total' => $v->eoq * $v->cost,
    );
  }
  $_SESSION['preload']['products'] = $item_rows;
  $form_state['redirect'] = 'av/po/new';
}

function avreports_stock_status_txn_total(&$table_rows, $txn_type, $filter_query) {
  $product_ids = array_keys($table_rows);
  if (empty($product_ids)) {
    return;
  }

  $args = empty($filter_query['args']) ? array() : $filter_query['args'];
  $query = "SELECT avprod.id, SUM(avtxn_detail.qty * avtxn_detail.qty_per_uom) AS total
FROM avtbl_transactions AS avtxn
INNER JOIN avtbl_transaction_details AS avtxn_detail ON avtxn_detail.transaction_id = avtxn.id
INNER JOIN avtbl_products AS avprod ON avprod.id = avtxn_detail.item_id
WHERE avtxn.status <> :void_type AND avtxn.status <> :cancelled_type AND avtxn.transaction_type = :txn_type AND avtxn.transaction_date < :date_to_stamp AND avtxn.transaction_date >= :date_from_stamp
AND avprod.id IN (:product_ids)
GROUP BY avtxn_detail.item_id";
  $args[':void_type'] = AVTXNS_TXN_STATUS_VOID;
  $args[':cancelled_type'] = AVTXNS_TXN_STATUS_CANCELLED;
  $args[':txn_type'] = $txn_type;
  $args[':product_ids'] = $product_ids;
  $totals = db_query($query, $args)->fetchAllKeyed();
  if (!empty($totals)) {
    foreach ($table_rows as $item_id => $table_row) {
      if (isset($totals[$item_id])) {
        $table_rows[$item_id]["{$txn_type}_total"] = '<div class="uk-text-right">' . number_format($totals[$item_id]) . '</div>';

        // Set average dr per day.
        if ($txn_type == AVTXNS_TXN_TYPE_DELIVERY) {
          $date_from = $args[':date_from_stamp'];
          $date_to = $args[':date_to_stamp'];
          $date_diff = $date_to - $date_from;
          $day_diff = floor($date_diff / (60 * 60 * 24));
          $day_diff = $day_diff < 1 ? 1 : $day_diff;
          $table_rows[$item_id]["day_ave"] = '<div class="uk-text-right">' . number_format($totals[$item_id] / $day_diff) . '</div>';
        }
      }
    }
  }
}
