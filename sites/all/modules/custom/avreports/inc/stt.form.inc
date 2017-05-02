<?php
function avreports_sales_to_trade_form($form, &$form_state) {
  //ini_set('memory_limit', '512M');
  $client_statuses = array(AVCLIENT_STATUS_ACTIVE, AVCLIENT_STATUS_SUSPENDED);
  $ctd = avclients_client_types(AVCLIENT_TYPE_CUSTOMER);
  $dad = avtxns_txn_list_filter_date_auto_options();
  $dad_default = empty($dad['last_month_to_date']) ? array() : $dad['last_month_to_date'];
  $filter_form_settings = array(
    'date_auto' => array(
      '#title' => t('Dates'),
      '#default_value' => 'last_month_to_date',
    ),
    'date_from' => array(
      '#title' => t('Date from'),
      '#default_value' => $dad_default['date_from'],
    ),
    'date_to' => array(
      '#title' => t('Date to'),
      '#default_value' => $dad_default['date_to'],
    ),
    'avclients.display_name' => array(
      '#title' => $ctd['name'],
      '#autocomplete_path' => 'av/clients/autocomplete/' . $ctd['type'] . '/' . implode('-', $client_statuses),
    ),
    'avcat_agent.title' => array(
      '#title' => t('Agent'),
      '#autocomplete_path' => 'av/categories/agent' . $ctd['type'] . '/autocomplete',
    ),
    'avcat_principal.title' => array(
      '#title' => t('Principal'),
      '#autocomplete_path' => 'av/categories/principal/autocomplete',
    ),
    //'avcat_area.title' => array('#access' => $transaction_kind == 'sales'),
    'table_columns' => array(
      '#options' => array(
        'agent_name' => 'Agent',
        'client_name' => 'Customer',
        'client_address' => 'Address <em>slower</em>',
        'principal_name' => 'Principal',
        'txn_id' => 'INV/SR#  <em>slower</em>',
        'ref_txn_id' => 'DR#  <em>slower</em>',
        'item_name' => 'Product',
        'qty_out' => 'Qty.',
        'uom' => 'UOM',
        'price' => 'Price <em>slower</em>',
        'discount' => 'Discount',
      ),
      '#default_value' => array(
        //'agent_name',
        'client_name',
        //'client_address',
        //'principal_name',
        'txn_id',
        //'ref_txn_id',
        'item_name',
        //'qty_out',
        //'uom',
        //'price',
        //'discount',
      ),
    ),
    'table_group_by' => array(
      '#options' => array(
        '' => 'None',
        'agent_name' => 'Agent',
        'client_name' => 'Customer',
        'principal_name' => 'Principal',
      ),
      '#default_value' => 'agent_name',
    ),
    'reset_button' => array('#attributes' => array('style' => 'margin-top: 25px;')),
  );
  $form['transaction_list_filter'] = avtxns_txn_list_filter_form($form, $form_state, $filter_form_settings);

  // Prepare query variables.
  $select = $joins = $where = $args = $group_by = array();
  $user_filter = empty($form_state['values']['avtxns_tl_filter']) ? array() : $form_state['values']['avtxns_tl_filter'];
  $filter_query = empty($user_filter['filter_query']) ? array() : $user_filter['filter_query'];
  $where = empty($filter_query['where']) ? array() : $filter_query['where'];
  $args = empty($filter_query['args']) ? array() : $filter_query['args'];

  // Headers.
  $header = array(
    'agent_name' => array('data' => 'AGENT', 'field' => 'agent_name'),
    'client_name' => array('data' => 'CUSTOMER', 'field' => 'client_name'),
    'client_address' => array('data' => 'ADDRESS'),
    'principal_name' => array('data' => 'PRINCIPAL', 'field' => 'principal_name'),
    'txn_id' => array('data' => 'TXN. #', 'field' => 'avtxn.id'),
    'ref_txn_id' => array('data' => 'DR #', 'field' => 'avtxn.id'),
    'item_name' => array('data' => 'PRODUCT', 'field' => 'avproducts.title'),
    'qty_out' => array('data' => 'QTY.', 'field' => 'qty_out'),
    'uom' => array('data' => 'UOM', 'field' => 'avcat_uom.title'),
    'price' => array('data' => 'PRICE', 'class' => array('uk-text-right'), 'field' => 'price'),
    'discount' => array('data' => 'DISCOUNT', 'class' => array('uk-text-right'), 'field' => 'discount_amount'),
  );
  $table_columns = empty($user_filter['settings']['table_columns']) ? array() : $user_filter['settings']['table_columns'];
  $table_columns = array_filter($table_columns);
  foreach ($header as $h_k => $h) {
    if (!isset($table_columns[$h_k])) {
      unset($header[$h_k]);
    }
  }
  $header['net_sales'] = array('data' => 'NET SALES', 'field' => 'net_sales', 'class' => array('uk-text-right'));

  // Build query.
  // Add default date from and date to.
  //if (empty($user_filter['date_to'])) {
  //  $today = format_date(time(), 'custom', 'F d, Y');
  //  $today_timestamp = strtotime($today);
  //  $where[] = "avtxn.transaction_date >= $timestamp AND avtxn.transaction_date < $next_day_timestamp";
  //}

  $joins[] = 'INNER JOIN {avtbl_transaction_details} AS avtxn_detail ON avtxn_detail.item_id = avproducts.id';
  $joins[] = 'INNER JOIN {avtbl_transactions} AS avtxn ON avtxn.id = avtxn_detail.transaction_id AND avtxn.status <> ' . AVTXNS_TXN_STATUS_VOID;
  if (isset($table_columns['agent_name']) || !empty($user_filter['avcat_agent.title'])) {
    $select[] = 'avcat_agent.title AS agent_name';
    $joins[] = 'LEFT JOIN {avtbl_categories} AS avcat_agent ON avcat_agent.id = avtxn.agent_id';
  }
  if (isset($table_columns['client_name']) || !empty($user_filter['avclients.display_name'])) {
    $select[] = 'avclients.display_name AS client_name';
    $joins['clients'] = 'LEFT JOIN {avtbl_clients} AS avclients ON avclients.id = avtxn.client_id';
  }
  if (isset($table_columns['client_address'])) {
    $select[] = 'avclients.address';
    $select[] = 'avclients.province';
    $select[] = 'avclients.city';
    $select[] = 'avclients.zip_code';
    $joins['clients'] = 'LEFT JOIN {avtbl_clients} AS avclients ON avclients.id = avtxn.client_id';
  }
  if (isset($table_columns['principal_name']) || !empty($user_filter['avcat_principal.title'])) {
    $select[] = 'avcat_principal.title AS principal_name';
    $joins[] = 'LEFT JOIN {avtbl_categories} AS avcat_principal ON avcat_principal.id = avproducts.principal_id';
  }
  if (isset($table_columns['item_name'])) {
    $select[] = 'avproducts.title';
  }
  if (isset($table_columns['txn_id'])) {
    $select[] = 'avtxn.id';
    $select[] = 'avtxn.transaction_type';
    $group_by['txn_id'] = 'avtxn.id';
  }
  if (isset($table_columns['ref_txn_id'])) {
    $select[] = 'avtxn_detail.ref_txn_id';
    $select[] = 'avtxn_detail.ref_txn_type';
    if (!isset($group_by['txn_id'])) {
      $group_by['ref_txn_id'] = 'avtxn_detail.ref_txn_id';
    }
  }
  if (isset($table_columns['qty_out'])) {
    $select[] = 'SUM(avtxn_detail.qty * avtxn_detail.qty_per_uom * (IF(avtxn.transaction_type = :txn_type_invoice, 1, -1))) AS qty_out';
  }
  if (isset($table_columns['uom'])) {
    $select[] = 'avcat_uom.title AS uom_title';
    $joins[] = 'LEFT JOIN {avtbl_categories} AS avcat_uom ON avcat_uom.id = avproducts.uom_id';
  }
  if (isset($table_columns['price'])) {
    $select[] = 'avtxn_detail.cost / avtxn_detail.qty_per_uom AS price';
    if (!isset($group_by['txn_id'])) {
      $group_by['price'] = 'avtxn_detail.cost';
    }
  }
  //$joins[] = 'LEFT JOIN {avtbl_categories} AS avcat_area ON avcat_area.id = avclients.area_id';

  if (isset($table_columns['discount'])) {
    $select[] = avtxns_txn_detail_sum_query('avtxn.transaction_type = :txn_type_invoice') . ' AS total_sales';
  }
  $select[] = avtxns_txn_detail_sum_query_discount_applied('avtxn.transaction_type = :txn_type_invoice') . ' AS net_sales';
  $where[] = "avtxn.transaction_type IN(:txn_types)";
  $where[] = "IF(avtxn.transaction_type = :txn_type_invoice, avtxn_detail.ref_txn_type = :txn_type_delivery, 1)";
  $args[':txn_types'] = array(AVTXNS_TXN_TYPE_INVOICE, AVTXNS_TXN_TYPE_SALES_RETURN);
  $args[':txn_type_invoice'] = AVTXNS_TXN_TYPE_INVOICE;
  $args[':txn_type_delivery'] = AVTXNS_TXN_TYPE_DELIVERY;




  // Set table pagination and sort settings.
  avbase_pagination_and_sort_settings($user_filter);

  // Set default order.
  if (empty($_GET['order'])) {
    $_GET['order'] = $header['net_sales']['data'];
    $_GET['sort'] = 'desc';
  }

  // Order by.
  $order_by = avbase_build_header_order_array($header);
  $col_group_by = array(
    'agent_name' => 'avtxn.agent_id',
    'client_name' => 'avtxn.client_id',
    'principal_name' => 'avproducts.principal_id',
    'item_name' => 'avproducts.id'
  );
  foreach ($col_group_by as $g_k => $g) {
    if (!isset($table_columns[$g_k])) {
      if ($g_k == 'client_name' && isset($table_columns['client_address'])) {
        continue;
      }
      unset($col_group_by[$g_k]);
    }
  }
  //dpm($where);dpm($args);
  //dpm($col_group_by);
  //$group_by[transaction_type] = 'avtxn.transaction_type';
  $group_by += $col_group_by;
  $query = "SELECT " . implode(', ', $select) . " FROM {avtbl_products} AS avproducts " . implode(' ', $joins);
  avbase_query_supplements($query, $where, $order_by, NULL, NULL, $group_by);
  $rs = db_query($query, $args);
  //$records = avtxns_product_query_load($where, $args, NULL, NULL, $order_by, $select, $joins, $group_by);
  //$records = avtxns_product_query_load($where, $args, NULL, NULL, $order_by, $select, $joins, $group_by);

  // Group result.
  $table_group_by = empty($user_filter['settings']['table_group_by']) ? NULL : $user_filter['settings']['table_group_by'];
  $grouped_records = array();
  foreach ($rs as $row) {
    $group_id = isset($row->$table_group_by) ? $row->$table_group_by : '';
    $grouped_records[check_plain($group_id)][] = $row;
  }

  // List result.
  $rows = array();
  //$index = 0;
  //$total2 = 0;
  foreach ($grouped_records as $group_id => $group) {
    $group_total = 0;
    $group_discount = 0;
    foreach ($group as $group_row_id => $row) {
      $rows[] = avreports_sales_to_trade_table_row($row, $table_columns, $table_group_by, $group_total, $group_discount);
    }


    $group_totals_row =  array(
      'data' => array(
        //'spacer' => array('data' => $group_id . ' - NET SALES', 'class' => array(' uk-text-right uk-text-uppercase')),
        //'discount_total' => array('data' => number_format($group_discount, 2), 'class' => array('uk-text-right uk-text-bold')),
        //'group_discount' => array('data' => strtoupper($group_id) . ' - DISCOUNT TOTAL: <span class="uk-text-bold">' . number_format($group_discount, 2) . '</span>', 'class' => array('uk-text-right')),
        'group_total' => array(
          'data' => strtoupper($group_id) . ' NET SALES<span class="uk-margin-left uk-text-bold">' . number_format($group_total, 2) . '</span>',
          'colspan' => 20, 'class' => array('uk-text-right')),
      ),
      //'class' => array('uk-row-mute'),
    );
    if (isset($table_columns['discount'])) {
      $group_totals_row['data']['group_total']['data'] = 'TOTAL DISCOUNT GIVEN<span class="uk-margin-left uk-text-bold uk-margin-large-right">' . number_format($group_discount, 2) . '</span>' . $group_totals_row['data']['group_total']['data'];
    }
    if (empty($table_columns)) {
      //unset($group_totals_row['data']['spacer']);
    }
    else {
      //$group_totals_row['data']['spacer']['colspan'] = count($table_columns);
    }
    $rows[] = $group_totals_row;
    $rows[] = array('data' => array('-' => array('data' => '&nbsp', 'colspan' => 20)));
    //$rows[] = array('data' => array('-' => array('colspan' => 10)));
  }

  //$rows['spacer'] = array('data' => array('-' => array('colspan' => 10)));
  //$rows['total'] = array(
  //  'data' => array(
  //    'spacer' => array('data' => 'Total', 'colspan' => 1),
  //    //'total1' => array('data' => number_format($total1), 'class' => array('uk-text-right uk-text-bold')),
  //    'total2' => array('data' => number_format($total2, 2), 'class' => array('uk-text-right uk-text-bold')),
  //  ),
  //  //'class' => array(''),
  //);
  //if (empty($table_columns)) {
  //  unset($rows['total']['data']['spacer']);
  //}
  //else {
  //  $rows['total']['data']['spacer']['colspan'] = count($table_columns);
  //}

  $table_attributes = array(
    'id' => 'transaction-list-table',
    'class' => array('uk-table-condensed uk-table-hover uk-text-small'),
  );

  //$pager_html = empty($options['#hide_filter_form']) ? theme('pager', array('tags' => array())) : '';
  $table = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => '<div class="uk-text-muted">' . t('No match found.') . '</div>',
    '#attributes' => $table_attributes,
  );

  $form['transaction_list_table'] = array(
    '#type' => 'container',
    '#theme' => 'avreports_inventory_ranking',
    '#attributes' => array(
      'id' => 'transaction-list-table-wrapper',
      'class' => array('printable av-sticky-small')
    ),
  );
  //$transaction_list_table['table']['#markup'] = '<!--Do not remove me-->' . $table_markup;
  $transaction_list_table['table'] = $table;
  //if (!empty($rows)) {
  $client_name = empty($user_filter['avclients.display_name']) ? '' : $user_filter['avclients.display_name'];
  $client = NULL;
  if (!empty($client_name)) {
    $client = avclient_query_load(array('avclients.display_name = :name'), array(':name' => $client_name));
    $client = array_shift($client);
  }
  avtxns_txn_list_table_add_date_from($transaction_list_table, $user_filter);
  if (!empty($client_name)) {
    $transaction_list_table['client_name'] = array(
      '#title' => $ctd['name'],
      '#title_display' => 'none',
      '#type' => 'item',
      '#markup' => empty($client_name) ? '' : check_plain($client_name),
    );
  }
  if (isset($client->address_string)) {
    $transaction_list_table['address'] = array(
      '#title' => 'Address',
      '#title_display' => 'none',
      '#type' => 'item',
      '#markup' => isset($client->address_string) ? nl2br($client->address_string) : '',
    );
  }
  if (!empty($client->contact_number)) {
    $transaction_list_table['phone'] = array(
      '#title' => 'Contact #',
      '#title_display' => 'inline-before',
      '#type' => 'item',
      '#markup' => empty($client->contact_number) ? '' : check_plain($client->contact_number),
    );
  }
  if (!empty($user_filter['avcat_agent.title'])) {
    $transaction_list_table['agent_name'] = array(
      '#title' => 'Agent',
      '#title_display' => 'inline-before',
      '#type' => 'item',
      '#markup' => empty($user_filter['avcat_agent.title']) ? '' : check_plain($user_filter['avcat_agent.title']),
    );
  }
  if (!empty($user_filter['avcat_principal.title'])) {
    $transaction_list_table['principal_name'] = array(
      '#title' => 'Principal',
      '#title_display' => 'inline-before',
      '#type' => 'item',
      '#markup' => empty($user_filter['avcat_principal.title']) ? '' : check_plain($user_filter['avcat_principal.title']),
    );
  }
  if (!empty($user_filter['avcat_area.title'])) {
    $transaction_list_table['area_name'] = array(
      '#title' => 'Area',
      '#title_display' => 'inline-before',
      '#type' => 'item',
      '#markup' => empty($user_filter['avcat_area.title']) ? '' : check_plain($user_filter['avcat_area.title']),
    );
  }
  //}
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
  return $form;
}

function avreports_sales_to_trade_table_row($row, $table_columns, $table_group_by, &$group_total, &$group_discount) {
  //$row = unserialize($row);
  $row_discount = 0;
  if (isset($table_columns['discount'])) {
    $row_discount = $row->total_sales - $row->net_sales;
    $group_discount += $row_discount;
  }
  $table_row = array(
    'data' => array(
      //'placeholder' => array('data' => $index + 1, 'class' => array('suk-hidden')),
      'agent_name' => isset($table_columns['agent_name']) ? check_plain($row->agent_name) : '',
      'client_name' => isset($table_columns['client_name']) ? check_plain($row->client_name) : '',
      'client_address' => isset($table_columns['client_address']) ? avclients_address_string_format($row) : '',
      'principal_name' => isset($table_columns['principal_name']) ? check_plain($row->principal_name) : '',
      'txn_id' => isset($table_columns['txn_id']) ? avbase_txn_number_format($row->transaction_type, $row->id) : '',
      'ref_txn_id' => isset($table_columns['ref_txn_id']) && !empty($row->ref_txn_id) ? avbase_txn_number_format($row->ref_txn_type, $row->ref_txn_id) : '-',
      'item_name' => isset($table_columns['item_name']) ? check_plain($row->title) : '',
      'qty_out' => isset($table_columns['qty_out']) ? number_format($row->qty_out) : '',
      'uom' => isset($table_columns['uom']) ? check_plain($row->uom_title) : '',
      'price' => array('data' => isset($table_columns['price']) ? number_format($row->price, 2) : '', 'class' => array('uk-text-right')),
      'discount' => array('data' => isset($table_columns['discount']) && $row_discount ? number_format($row_discount, 2) : '', 'class' => array('uk-text-right')),
    ),
    'class' => array('uk-text-uppercase'),
  );

  if (!empty($group_total) && isset($table_row['data'][$table_group_by])) {
    $table_row['data'][$table_group_by] = '';
    if ($table_group_by == 'client_name') {
      $table_row['data']['client_address'] = '';
    }
  }

  foreach ($table_row['data'] as $d_k => $d) {
    if (!isset($table_columns[$d_k])) {
      unset($table_row['data'][$d_k]);
    }
  }
  $table_row['data']['net_sales'] = array('data' => number_format($row->net_sales, 2), 'class' => array('uk-text-right'));

  //$table_row['data']['qty_sold'] = array('data' => number_format($row->qty_sold), 'class' => array('uk-text-right'));
  //$table_row['data']['net_sales'] = array('data' => number_format($row->net_sales, 2), 'class' => array('uk-text-right'));
  //$total2 += $row->net_sales;
  $group_total += $row->net_sales;

  //dpm($row->total_sales);
  //if ($row->transaction_type == AVTXNS_TXN_TYPE_SALES_RETURN) {
  //  $table_row['class'][] = 'uk-alert-danger';
  //}
  //unset($row);
  return $table_row;
}