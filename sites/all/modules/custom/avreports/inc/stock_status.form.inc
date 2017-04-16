<?php

function avreports_stock_status_form($form, &$form_state) {
  $dad = avtxns_txn_list_filter_date_auto_options();
  $dad_default = empty($dad['last_month_to_date']) ? array() : $dad['last_month_to_date'];
  $filter_form_settings = array(
    //'date_auto' => array(
    //  '#title' => t('Dates'),
    //  '#default_value' => 'last_month_to_date',
    //),
    //'date_from' => array(
    //  '#title' => t('Date from'),
    //  '#default_value' => $dad_default['date_from'],
    //),
    //'date_to' => array(
    //  '#title' => t('Date to'),
    //  '#default_value' => $dad_default['date_to'],
    //),
    //'reset_button' => array('#attributes' => array('style' => 'margin-top: 25px;')),
  );
  $form['transaction_list_filter'] = avtxns_txn_list_filter_form($form, $form_state, $filter_form_settings);
  $form['transaction_list_filter']['avtxns_tl_filter']['#attributes']['class'][] = 'uk-hidden';

  // Prepare query variables.
  $select = $joins = $where = $args = array();
  $user_filter = empty($form_state['values']['avtxns_tl_filter']) ? array() : $form_state['values']['avtxns_tl_filter'];
  $filter_query = empty($user_filter['filter_query']) ? array() : $user_filter['filter_query'];
  //$where = empty($filter_query['where']) ? array() : $filter_query['where'];
  //$args = empty($filter_query['args']) ? array() : $filter_query['args'];

  $header = array();
  $header['title'] = array('data' => 'Product', 'field' => 'title');
  $header['vendor'] = array('data' => 'Preferred Vendor');
  $header['on_hand'] = array('data' => 'On Hand', 'field' => 'qty', 'class' => array('uk-text-right'));
  $header['uom'] = array('data' => 'UOM');
  $header['on_so'] = array('data' => 'On SO', 'class' => array('uk-text-right'));
  $header['on_po'] = array('data' => 'On PO', 'class' => array('uk-text-right'));
  $header['rop'] = array('data' => 'Reorder Pt.', 'field' => 'rop', 'class' => array('uk-text-right'));
  $header['cost'] = array('data' => 'Cost', 'field' => 'cost', 'class' => array('uk-text-right'));
  $header['price'] = array('data' => 'Price', 'field' => 'price', 'class' => array('uk-text-right'));


  if (empty($_GET['order'])) {
    $_GET['order'] = 'On Hand';
    $_GET['sort'] = 'asc';
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
  $order_by = avbase_build_header_order_array($header);

  //$select[] = 'avprod.*';
  //$select[] = 'avtxn.transaction_type';
  //$select[] = '(IF(avtxn.transaction_type = :po, 1, 2)) AS on_so';
  //$select[] = '(IF(avtxn.transaction_type = :so, 1, 2)) AS on_po';
  //$joins[] = 'LEFT JOIN {avtbl_transaction_details} AS avtxn_detail ON avtxn_detail.item_id = avprod.id';
  //$joins[] = 'LEFT JOIN {avtbl_transactions} AS avtxn ON avtxn.id = avtxn_detail.transaction_id';
  $where[] = 'avprod.status = :active';
  //$where[] = 'avprod.id < 10';
  //$where[] = 'avtxn.status IS NULL OR avtxn.status IN (:status)';
  $args[':active'] = AVPRODUCTS_STATUS_ACTIVE;
  //$args[':po'] = AVTXNS_TXN_TYPE_PURCHASE_ORDER;
  //$args[':so'] = AVTXNS_TXN_TYPE_SALES_ORDER;
  //$args[':status'] = array(AVTXNS_TXN_STATUS_OPEN);
  //$query = "SELECT " . implode(', ', $select) . " FROM {avtbl_products} AS avprod " . implode(' ', $joins);
  //$group_by = array('avprod.id');
  //if (!empty($where)) {
  //  $query .= " WHERE " . implode(' AND ', $where);
  //}
  //if (!empty($group_by)) {
  //  $query .= " GROUP BY " . implode(', ', $group_by);
  //}
  //if (!empty($order_by)) {
  //  $query .= " ORDER BY " . implode(', ', $order_by);
  //}
  //$rs = db_query($query, $args);
  $product_rows = avproduct_query_load($where, $args, '', $order_by);
  $pr_ids = array();
  foreach ($product_rows as $pr) {
    $pr_ids[] = $pr->id;
  }
  $query = "SELECT
    avtxn.transaction_type,
    avtxn_detail.item_id,
    SUM(avtxn_detail.qty * avtxn_detail.qty_per_uom) as txn_qty
    FROM {avtbl_transaction_details} AS avtxn_detail
    INNER JOIN {avtbl_transactions} AS avtxn ON avtxn.id = avtxn_detail.transaction_id
    WHERE avtxn.status IN (:status) AND avtxn.transaction_type IN (:types)
    AND avtxn_detail.item_id IN (:ids)
    GROUP BY avtxn_detail.item_id, avtxn.transaction_type
    ";
  $targs = array();
  $targs[':status'] = array(AVTXNS_TXN_STATUS_OPEN);
  $targs[':ids'] = $pr_ids;
  $targs[':types'] = array(AVTXNS_TXN_TYPE_PURCHASE_ORDER, AVTXNS_TXN_TYPE_SALES_ORDER);
  $rs = db_query($query, $targs);
  $trows = array();
  foreach ($rs as $row) {
    $trows[$row->item_id][$row->transaction_type] = $row;
  }

  $table_rows = array();
  foreach ($product_rows as $p_id => $row) {
    $data = array();
    $data['title'] = array('data' => check_plain($row->title));
    $data['vendor'] = array('data' => check_plain($row->vendor_name));
    $data['on_hand'] = array('data' => $row->qty, 'class' => array('uk-text-right'));
    $data['uom'] = array('data' => check_plain($row->uom_title));
    $on_so = empty($trows[$p_id][AVTXNS_TXN_TYPE_SALES_ORDER]->txn_qty) ? 0 : $trows[$p_id][AVTXNS_TXN_TYPE_SALES_ORDER]->txn_qty;
    $on_po = empty($trows[$p_id][AVTXNS_TXN_TYPE_PURCHASE_ORDER]->txn_qty) ? 0 : $trows[$p_id][AVTXNS_TXN_TYPE_PURCHASE_ORDER]->txn_qty;
    $data['on_so'] = array('data' => $on_so, 'class' => array('uk-text-right'));
    $data['on_po'] = array('data' => $on_po, 'class' => array('uk-text-right'));
    $data['rop'] = array('data' => $row->rop, 'class' => array('uk-text-right'));
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

  $form['transaction_list_table'] = array(
    '#type' => 'container',
    '#theme' => 'avreports_inventory_ranking',
    '#attributes' => array(
      'id' => 'transaction-list-table-wrapper',
      'class' => array('printable')
    ),
  );

  avtxns_txn_list_table_add_date_from($transaction_list_table, $user_filter);

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
