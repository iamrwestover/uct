<?php
function avreports_stock_status_form($form, &$form_state) {
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
    'all_products' => array('#default_value' => 0),
    'stock_status' => array('#default_value' => 'all'),
    'reset_button' => array('#attributes' => array('style' => 'margin-top: 25px;')),
  );
  $form['transaction_list_filter'] = avtxns_txn_list_filter_form($form, $form_state, $filter_form_settings);
  // Prepare query variables.
  $select = $joins = $where = $args = array();
  $user_filter = empty($form_state['values']['avtxns_tl_filter']) ? array() : $form_state['values']['avtxns_tl_filter'];
  $filter_query = empty($user_filter['filter_query']) ? array() : $user_filter['filter_query'];
  $where = empty($filter_query['where']) ? array() : $filter_query['where'];
  $args = empty($filter_query['args']) ? array() : $filter_query['args'];

  $all_products = !empty($user_filter['settings']['all_products']);
  //if ($all_products) {
  //  $where = $args = array();
  //}

  $header = array();
  $header['product_title'] = array('data' => 'Product', 'field' => 'title');
  //$header['principal'] = array('data' => 'Principal', 'field' => 'principal_title');
  $header['vendor'] = array('data' => 'Preferred Vendor', 'field' => 'vendor_name');
  $header['onhand'] = array('data' => 'On Hand', 'field' => 'onhand', 'class' => array('uk-text-right'));
  $header['uom'] = array('data' => 'UOM', 'field' => 'uom_title');
  $header['dr_total'] = array('data' => 'Delivered', 'field' => 'dr_total', 'class' => array('uk-text-right'));
  $header['on_so'] = array('data' => 'On SO', 'field' => 'so_total', 'class' => array('uk-text-right'));
  $header['on_po'] = array('data' => 'On PO', 'field' => 'po_total', 'class' => array('uk-text-right'));
  $header['rop'] = array('data' => 'ROP', 'field' => 'rop', 'class' => array('uk-text-right'));
  $header['eoq'] = array('data' => 'EOQ', 'field' => 'eoq', 'class' => array('uk-text-right'));
  $header['cost'] = array('data' => 'Cost', 'field' => 'cost', 'class' => array('uk-text-right'));
  $header['price'] = array('data' => 'Price', 'field' => 'price', 'class' => array('uk-text-right'));


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
  if (isset($order_by['onhand'])) {
    $order_by['dr_total'] = 'dr_total DESC';
    $order_by['so_total'] = 'so_total DESC';
    $order_by['po_total'] = 'po_total ASC';
  }


  $select = array();
  $products_join = $all_products ? 'LEFT JOIN' : 'INNER JOIN';
  $select[] = 'avprod.id AS product_id';
  $select[] = 'avprod.title AS product_title';
  $select[] = 'avprod.qty AS onhand';
  $select[] = 'avprod.rop';
  $select[] = 'avprod.eoq';
  $select[] = 'avprod.cost';
  $select[] = 'avprod.price';
  $select[] = 'SUM(IF(avtxn.transaction_type = :po_type && avtxn.status = :open_status, (avtxn_detail.qty * avtxn_detail.qty_per_uom), NULL)) AS po_total';
  $select[] = 'SUM(IF(avtxn.transaction_type = :so_type && avtxn.status = :open_status, (avtxn_detail.qty * avtxn_detail.qty_per_uom), NULL)) AS so_total';
  $select[] = 'SUM(IF(avtxn.transaction_type = :dr_type, (avtxn_detail.qty * avtxn_detail.qty_per_uom), NULL)) AS dr_total';
  $select[] = 'avvendor.display_name AS vendor_name';
  $select[] = 'avcat_uom.title AS uom_title';
  //$select[] = 'avcat_principal.title AS principal_title';
  $joins[] = $products_join . ' {avtbl_transaction_details} AS avtxn_detail ON avtxn_detail.item_id = avprod.id';
  $joins['avtxn'] = $products_join . ' {avtbl_transactions} AS avtxn ON avtxn.id = avtxn_detail.transaction_id AND avtxn.status <> :void_type AND avtxn.transaction_type IN (:ttypes)';
  if (!empty($where)) {
    $joins['avtxn'] .= ' AND ' . implode(' AND ', $where);
    $where = array();
  }
  $joins[] = 'LEFT JOIN {avtbl_clients} AS avvendor ON avvendor.id = avprod.vendor_id';
  $joins[] = 'LEFT JOIN {avtbl_categories} AS avcat_uom ON avcat_uom.id = avprod.uom_id';

  $args[':po_type'] = AVTXNS_TXN_TYPE_PURCHASE_ORDER;
  $args[':so_type'] = AVTXNS_TXN_TYPE_SALES_ORDER;
  $args[':dr_type'] = AVTXNS_TXN_TYPE_DELIVERY;
  $args[':open_status'] = AVTXNS_TXN_STATUS_OPEN;
  $args[':void_type'] = AVTXNS_TXN_STATUS_VOID;
  $args[':ttypes'] = array(AVTXNS_TXN_TYPE_PURCHASE_ORDER, AVTXNS_TXN_TYPE_SALES_ORDER, AVTXNS_TXN_TYPE_DELIVERY);
  $group_by = array();
  $group_by[] = 'avprod.id';

  switch ($user_filter['settings']['stock_status']) {
    case 'low':
      $where[] = '(avprod.qty < 1 OR avprod.rop <= avprod.qty)';
      break;
    case 'os':
      $where[] = 'avprod.qty < 1';
      break;
  }
  if (!empty($user_filter['settings']['principal_name'])) {
    $joins[] = 'INNER JOIN {avtbl_categories} AS avcat_principal ON avcat_principal.id = avprod.principal_id AND avcat_principal.title = :principal_name';
    $args[':principal_name'] = $user_filter['settings']['principal_name'];
  }
  if (!empty($user_filter['settings']['preferred_vendor'])) {
    $where[] = 'avvendor.display_name = :preferred_vendor';
    $args[':preferred_vendor'] = $user_filter['settings']['preferred_vendor'];
  }
  $query = "SELECT " . implode(', ', $select) . " FROM {avtbl_products} AS avprod " . implode(' ', $joins);

  avbase_query_supplements($query, $where, $order_by, NULL, NULL, $group_by);
  $rs = db_query($query, $args);

  $table_rows = array();
  foreach ($rs as $row) {
    $data = array();
    $data['product_title'] = array('data' => l($row->product_title, 'av/products/' . $row->product_id . '/view'));
    //$data['principal'] = array('data' => check_plain($row->principal_title));
    $data['vendor'] = array('data' => check_plain($row->vendor_name));
    $data['onhand'] = array('data' => number_format($row->onhand), 'class' => array('uk-text-right'));
    $data['uom'] = array('data' => check_plain($row->uom_title));
    $data['dr_total'] = array('data' => number_format($row->dr_total), 'class' => array('uk-text-right'));
    $data['on_so'] = array('data' => number_format($row->so_total), 'class' => array('uk-text-right'));
    $data['on_po'] = array('data' => number_format($row->po_total), 'class' => array('uk-text-right'));
    $data['rop'] = array('data' => number_format($row->rop), 'class' => array('uk-text-right'));
    $data['eoq'] = array('data' => number_format($row->eoq), 'class' => array('uk-text-right'));
    $data['cost'] = array('data' => number_format($row->cost, 2), 'class' => array('uk-text-right'));
    $data['price'] = array('data' => number_format($row->price, 2), 'class' => array('uk-text-right'));
    $table_rows[] = array('data' => $data);
  }


  $transaction_list_table['table'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $table_rows,
    '#empty' => '<div class="uk-text-muted">' . t('No match found.') . '</div>',
    '#attributes' => array(
      'id' => 'transaction-list-table',
      'class' => array('uk-table-condensed uk-table-hover'),
    ),
  );
  avtxns_txn_list_table_add_date_from($transaction_list_table, $user_filter);
  if (!empty($user_filter['avcat_principal.title'])) {
    $transaction_list_table['principal_name'] = array(
      '#type' => 'item',
      '#title' => 'Principal',
      '#title_display' => 'inline-before',
      '#markup' => check_plain($user_filter['avcat_principal.title']),
    );
  }

  $form['transaction_list_table'] = array(
    '#type' => 'container',
    '#theme' => 'avreports_txn_list',
    '#attributes' => array(
      'id' => 'transaction-list-table-wrapper',
      'class' => array('printable')
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
  return $form;
}
