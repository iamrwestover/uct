<?php
function avreports_sales_to_trade_form($form, &$form_state, $dr_register = FALSE, $limit = NULL, $offset = NULL) {
  ini_set('memory_limit', AVBASE_EXPANDED_MEMORY_LIMIT);

  $client_statuses = array(AVCLIENT_STATUS_ACTIVE, AVCLIENT_STATUS_SUSPENDED);
  $ctd = avclients_client_types(AVCLIENT_TYPE_CUSTOMER);
  $dad = avtxns_txn_list_filter_date_auto_options();
  $dad_default = empty($dad['this_week']) ? array() : $dad['this_week'];
  $filter_form_settings = array(
    'avtxn.id' => array('#title' => $dr_register ? 'DR#' : 'INV/SR#'),
    'avtxn_detail.ref_txn_id' => array('#title' => 'DR#'),
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
    'avprod.title' => array(
      '#title' => t('Product'),
    ),
    'avcat_area.title' => array('#title' => 'Area'),
    'avclients.category_id' => array('#title' => 'Channel', '#options' => avbase_get_category_options_array('category' . AVCLIENT_TYPE_CUSTOMER)),
    'table_columns' => array(
      '#options' => array(
        'agent_name' => 'Agent',
        'client_name' => 'Customer',
        'category_name' => 'Channel',
        'address' => '<em title="May cause slower response time">Address</em>',
        'city' => '<em title="May cause slower response time">City</em>',
        'province' => '<em title="May cause slower response time">Province</em>',
        'zip_code' => '<em title="May cause slower response time">Zip Code</em>',
        'principal_name' => 'Principal',
        'transaction_date' => '<em title="May cause slower response time">Date</em>',
        'txn_id' => '<em title="May cause slower response time">' . ($dr_register ? 'DR#' : 'INV/SR#') . '</em>',
        'ref_txn_id' => '<em title="May cause slower response time">DR#</em>',
        'item_name' => 'Product',
        'qty_out' => 'Qty.',
        'uom' => 'UOM',
        'price' => '<em title="May cause slower response time">Price</em>',
        'discount' => 'Discount',
      ),
      '#default_value' => array(
        'agent_name',
        //'client_name',
        //'client_address',
        'principal_name',
        //'txn_id',
        //'ref_txn_id',
        //'item_name',
        //'qty_out',
        //'uom',
        //'price',
        //'discount',
      ),
    ),
    'table_group_by' => array(
      '#options' => array(
        '' => 'Overall',
        'agent_name' => 'Agent',
        'client_name' => 'Customer',
        'principal_name' => 'Principal',
      ),
      '#default_value' => 'agent_name',
    ),
    'reset_button' => array('#attributes' => array('style' => 'margin-top: 25px;')),
    'export_button' => array(
      '#func_name' => __FUNCTION__ ,
      '#func_args' => array($dr_register),
      '#report_name' => $dr_register ? 'Delivery Receipt Register' : 'Sales to Trade',
      '#report_inc' => 'sales_to_trade.form',
    ),
  );
  if ($dr_register) {
    unset($filter_form_settings['avtxn_detail.ref_txn_id']);
    unset($filter_form_settings['table_columns']['#options']['ref_txn_id']);
    $filter_form_settings['table_group_by']['#default_value'] = '';
    $filter_form_settings['table_columns']['#default_value'] = array('principal_name');
  }
  $form['transaction_list_filter'] = avtxns_txn_list_filter_form($form, $form_state, $filter_form_settings);

  $user_filter = empty($form_state['values']['avtxns_tl_filter']) ? array() : $form_state['values']['avtxns_tl_filter'];
  $table = avreports_sales_to_trade_table($form, $form_state, $dr_register, $limit, $offset);

  $form['transaction_list_table'] = array(
    '#type' => 'container',
    '#theme' => 'avreports_txn_list',
    '#attributes' => array(
      'id' => 'transaction-list-table-wrapper',
      'class' => array('printable av-sticky-small')
    ),
  );
  //$transaction_list_table['table']['#markup'] = '<!--Do not remove me-->' . $table_markup;
  $transaction_list_table['table'] = $table;
  //if (!empty($rows)) {
  //$client_name = empty($user_filter['avclients.display_name']) ? '' : $user_filter['avclients.display_name'];
  //$client = NULL;
  //if (!empty($client_name)) {
  //  $client = avclient_query_load(array('avclients.display_name = :name'), array(':name' => $client_name));
  //  $client = array_shift($client);
  //}
  avtxns_txn_list_table_add_date_from($transaction_list_table, $user_filter);
  avtxns_txn_list_table_add_product_title($transaction_list_table, $user_filter);
  if (!empty($user_filter['avclients.display_name'])) {
    $client = avclient_query_load(array('avclients.display_name = :name'), array(':name' => $user_filter['avclients.display_name']));
    $client = array_shift($client);
    if (!empty($client->id)) {
      $transaction_list_table['client_name'] = array(
        '#type' => 'item',
        '#markup' => '<div class="uk-text-bold">' . check_plain($client->display_name) . '</div>' . nl2br($client->address_string),
      );
      if (!empty($client->contact_number)) {
        $transaction_list_table['client_name']['#markup'] .= '<br />Contact #: ' . check_plain($client->contact_number);
      }
    }
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
  if (!empty($user_filter['avtxn.id'])) {
    $transaction_list_table['avtxn.id'] = array(
      '#title' => 'INV/SR#',
      '#title_display' => 'inline-before',
      '#type' => 'item',
      '#markup' => empty($user_filter['avtxn.id']) ? '' : check_plain($user_filter['avtxn.id']),
    );
  }
  if (!empty($user_filter['avtxn_detail.ref_txn_id'])) {
    $transaction_list_table['avtxn_detail.ref_txn_id'] = array(
      '#title' => 'DR#',
      '#title_display' => 'inline-before',
      '#type' => 'item',
      '#markup' => empty($user_filter['avtxn_detail.ref_txn_id']) ? '' : check_plain($user_filter['avtxn_detail.ref_txn_id']),
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

  // Show / hide preview?
  avreports_report_live_preview($form);
  return $form;
}

function avreports_sales_to_trade_table_row($row, $table_columns, $table_group_by, $group_total, $row_discount, $hide_cols) {

  $data = array();
  //'placeholder' => array('data' => $index + 1, 'class' => array('suk-hidden')),
  avreports_sales_to_trade_table_row_col($data, 'agent_name', $row, $table_columns, $table_group_by, $group_total, $hide_cols);
  avreports_sales_to_trade_table_row_col($data, 'client_name', $row, $table_columns, $table_group_by, $group_total, $hide_cols);
  avreports_sales_to_trade_table_row_col($data, 'category_name', $row, $table_columns, $table_group_by, $group_total, $hide_cols, NULL, 'client_name');
  avreports_sales_to_trade_table_row_col($data, 'address', $row, $table_columns, $table_group_by, $group_total, $hide_cols, NULL, 'client_name');
  avreports_sales_to_trade_table_row_col($data, 'city', $row, $table_columns, $table_group_by, $group_total, $hide_cols, NULL, 'client_name');
  avreports_sales_to_trade_table_row_col($data, 'province', $row, $table_columns, $table_group_by, $group_total, $hide_cols, NULL, 'client_name');
  avreports_sales_to_trade_table_row_col($data, 'zip_code', $row, $table_columns, $table_group_by, $group_total, $hide_cols, NULL, 'client_name');
  avreports_sales_to_trade_table_row_col($data, 'principal_name', $row, $table_columns, $table_group_by, $group_total, $hide_cols);

  $col_data = isset($table_columns['transaction_date']) ? format_date($row->transaction_date, 'custom', AVREPORTS_LIST_DATE_FORMAT) : NULL;
  avreports_sales_to_trade_table_row_col($data, 'transaction_date', $row, $table_columns, $table_group_by, $group_total, $hide_cols, $col_data);

  $col_data = isset($table_columns['txn_id']) ? avbase_txn_number_format($row->transaction_type, $row->id) : NULL;
  avreports_sales_to_trade_table_row_col($data, 'txn_id', $row, $table_columns, $table_group_by, $group_total, $hide_cols, $col_data);

  $col_data = isset($table_columns['ref_txn_id']) && !empty($row->ref_txn_id) ? avbase_txn_number_format($row->ref_txn_type, $row->ref_txn_id) : '-';
  avreports_sales_to_trade_table_row_col($data, 'ref_txn_id', $row, $table_columns, $table_group_by, $group_total, $hide_cols, $col_data);

  $col_data = isset($table_columns['item_name']) ? check_plain($row->title) : NULL;
  avreports_sales_to_trade_table_row_col($data, 'item_name', $row, $table_columns, $table_group_by, $group_total, $hide_cols, $col_data);

  avreports_sales_to_trade_table_row_col($data, 'qty_out', $row, $table_columns, $table_group_by, $group_total, $hide_cols);

  $col_data = isset($table_columns['uom']) ? check_plain($row->uom_title) : NULL;
  avreports_sales_to_trade_table_row_col($data, 'uom', $row, $table_columns, $table_group_by, $group_total, $hide_cols, $col_data);

  $col_data = isset($table_columns['price']) ? '<div class="uk-text-right">' . number_format($row->price, 2) . '</div>' : NULL;
  avreports_sales_to_trade_table_row_col($data, 'price', $row, $table_columns, $table_group_by, $group_total, $hide_cols, $col_data);

  $col_data = isset($table_columns['discount']) && $row_discount ? '<div class="uk-text-right">' . number_format($row_discount, 2) . '</div>' : NULL;
  avreports_sales_to_trade_table_row_col($data, 'discount', $row, $table_columns, $table_group_by, $group_total, $hide_cols, $col_data);

  $table_row = $data;

  //$table_row = array(
  //  'data' => array(
  //    //'placeholder' => array('data' => $index + 1, 'class' => array('suk-hidden')),
  //    'agent_name' => isset($table_columns['agent_name']) ? check_plain($row->agent_name) : '',
  //    'client_name' => isset($table_columns['client_name']) ? check_plain($row->client_name) : '',
  //    'client_address' => isset($table_columns['client_address']) ? avclients_address_string_format($row) : '',
  //    'principal_name' => isset($table_columns['principal_name']) ? check_plain($row->principal_name) : '',
  //    'txn_id' => isset($table_columns['txn_id']) ? avbase_txn_number_format($row->transaction_type, $row->id) : '',
  //    'ref_txn_id' => isset($table_columns['ref_txn_id']) && !empty($row->ref_txn_id) ? avbase_txn_number_format($row->ref_txn_type, $row->ref_txn_id) : '-',
  //    'item_name' => isset($table_columns['item_name']) ? check_plain($row->title) : '',
  //    'qty_out' => isset($table_columns['qty_out']) ? number_format($row->qty_out) : '',
  //    'uom' => isset($table_columns['uom']) ? check_plain($row->uom_title) : '',
  //    'price' => array('data' => isset($table_columns['price']) ? number_format($row->price, 2) : '', 'class' => array('uk-text-right')),
  //    'discount' => array('data' => isset($table_columns['discount']) && $row_discount ? number_format($row_discount, 2) : ''),
  //  ),
  //  //'class' => array('uk-text-uppercase'),
  //);
  //
  //if (!empty($group_total) && isset($table_row['data'][$table_group_by])) {
  //  $table_row['data'][$table_group_by] = '';
  //  if ($table_group_by == 'client_name') {
  //    $table_row['data']['client_address'] = '';
  //  }
  //}
  //
  //foreach ($table_row['data'] as $d_k => $d) {
  //  if (!isset($table_columns[$d_k])) {
  //    unset($table_row['data'][$d_k]);
  //  }
  //}

  $table_row['net_sales'] = '<div class="uk-text-right">' . number_format($row->net_sales, 2) . '</div>';
  //$table_row['data']['#net_sales'] = round($row->net_sales, 4);

  //$table_row['#test'] = 3;
  //$table_row['data']['qty_sold'] = array('data' => number_format($row->qty_sold), 'class' => array('uk-text-right'));
  //$table_row['data']['net_sales'] = array('data' => number_format($row->net_sales, 2), 'class' => array('uk-text-right'));
  //$total2 += $row->net_sales;
  //$group_total += $row->net_sales;

  //dpm($row->total_sales);
  //if ($row->transaction_type == AVTXNS_TXN_TYPE_SALES_RETURN) {
  //  $table_row['class'][] = 'uk-alert-danger';
  //}
  //unset($row);
  return $table_row;
}

function avreports_sales_to_trade_table_row_col(&$data, $col_name, $row, $table_columns, $table_group_by, $group_total, $hide_cols, $value = NULL, $tgcn = NULL) {
  if (in_array($col_name, $hide_cols)) {
    return;
  }
  if (isset($table_columns[$col_name])) {
    if (!empty($group_total) && !empty($table_group_by) && ($table_group_by == $col_name || $table_group_by == $tgcn)) {
      $data[$col_name] = NULL;
    }
    else {
      $data[$col_name] = is_null($value) && isset($row->$col_name) ? check_plain($row->$col_name) : $value;
    }
  }
}

function avreports_sales_to_trade_table(&$form, $form_state, $dr_register = FALSE, $limit = NULL, $offset = NULL) {
  // Prepare query variables.
  $select = $joins = $where = $args = $group_by = array();
  $user_filter = empty($form_state['values']['avtxns_tl_filter']) ? array() : $form_state['values']['avtxns_tl_filter'];
  $filter_query = empty($user_filter['filter_query']) ? array() : $user_filter['filter_query'];
  $where = empty($filter_query['where']) ? array() : $filter_query['where'];
  $args = empty($filter_query['args']) ? array() : $filter_query['args'];

  $hide_cols = array();
  if (!empty($user_filter['avcat_principal.title'])) {
    $hide_cols[] = 'principal_name';
  }
  if (!empty($user_filter['avcat_agent.title'])) {
    $hide_cols[] = 'agent_name';
  }
  if (!empty($user_filter['avclients.display_name'])) {
    $hide_cols[] = 'client_name';
  }
  if (!empty($user_filter['avtxn.id'])) {
    $hide_cols[] = 'txn_id';
  }
  if (!empty($user_filter['avtxn_detail.ref_txn_id'])) {
    $hide_cols[] = 'ref_txn_id';
  }
  if (!empty($user_filter['avprod.title'])) {
    $hide_cols[] = 'item_name';
  }
  if (!empty($user_filter['avclients.category_id'])) {
    $hide_cols[] = 'category_name';
  }

  // Headers.
  $header = array(
    'agent_name' => array('data' => 'AGENT', 'field' => 'agent_name'),
    'client_name' => array('data' => 'CUSTOMER', 'field' => 'client_name'),
    'category_name' => array('data' => 'CHANNEL', 'field' => 'avcat_cat.title'),
    'address' => array('data' => 'ADDRESS'),
    'city' => array('data' => 'CITY', 'field' => 'city'),
    'province' => array('data' => 'PROVINCE', 'field' => 'province'),
    'zip_code' => array('data' => 'ZIP CODE', 'field' => 'zip_code'),
    'principal_name' => array('data' => 'PRINCIPAL', 'field' => 'principal_name'),
    'transaction_date' => array('data' => 'DATE', 'field' => 'avtxn.transaction_date'),
    'txn_id' => array('data' => 'TXN. #', 'field' => 'avtxn.id'),
    'ref_txn_id' => array('data' => 'DR #', 'field' => 'avtxn.id'),
    'item_name' => array('data' => 'PRODUCT', 'field' => 'avprod.title'),
    'qty_out' => array('data' => 'QTY.', 'field' => 'qty_out'),
    'uom' => array('data' => 'UOM', 'field' => 'avcat_uom.title'),
    'price' => array('data' => 'PRICE', 'class' => array('uk-text-right'), 'field' => 'price'),
    'discount' => array('data' => 'DISCOUNT', 'class' => array('uk-text-right'), 'field' => 'total_sales'),
  );
  $table_columns = empty($user_filter['settings']['table_columns']) ? array() : $user_filter['settings']['table_columns'];
  $table_columns = array_filter($table_columns);
  foreach ($header as $h_k => $h) {
    if (!isset($table_columns[$h_k])) {
      unset($header[$h_k]);
    }
  }
  foreach ($header as $h_k => $h) {
    if (in_array($h_k, $hide_cols)) {
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

  $joins[] = 'INNER JOIN {avtbl_transaction_details} AS avtxn_detail ON avtxn_detail.item_id = avprod.id';
  $joins[] = 'INNER JOIN {avtbl_transactions} AS avtxn ON avtxn.id = avtxn_detail.transaction_id AND avtxn.status <> ' . AVTXNS_TXN_STATUS_VOID;
  if (isset($table_columns['agent_name']) || !empty($user_filter['avcat_agent.title'])) {
    $select[] = 'avcat_agent.title AS agent_name';
    $joins[] = 'LEFT JOIN {avtbl_categories} AS avcat_agent ON avcat_agent.id = avtxn.agent_id';
  }
  if (isset($table_columns['client_name']) || !empty($user_filter['avclients.display_name'])) {
    $select[] = 'avclients.display_name AS client_name';
    $joins['clients'] = 'LEFT JOIN {avtbl_clients} AS avclients ON avclients.id = avtxn.client_id';
  }
  if (isset($table_columns['category_name']) || !empty($user_filter['avclients.category_id'])) {
    $select[] = 'avcat_cat.title AS category_name';
    $joins['clients'] = 'LEFT JOIN {avtbl_clients} AS avclients ON avclients.id = avtxn.client_id';
    $joins[] = 'LEFT JOIN {avtbl_categories} AS avcat_cat ON avcat_cat.id = avclients.category_id';
    if (!isset($table_columns['client_name'])) {
      $group_by['category_id'] = 'category_name';
    }
  }
  if (isset($table_columns['address'])) {
    $select[] = 'avclients.address';
    $joins['clients'] = 'LEFT JOIN {avtbl_clients} AS avclients ON avclients.id = avtxn.client_id';
    if (!isset($table_columns['client_name'])) {
      $group_by['address'] = 'avclients.address';
    }
  }
  if (isset($table_columns['province'])) {
    $select[] = 'avclients.province';
    $joins['clients'] = 'LEFT JOIN {avtbl_clients} AS avclients ON avclients.id = avtxn.client_id';
    if (!isset($table_columns['client_name'])) {
      $group_by['province'] = 'avclients.province';
    }
  }
  if (isset($table_columns['city'])) {
    $select[] = 'avclients.city';
    $joins['clients'] = 'LEFT JOIN {avtbl_clients} AS avclients ON avclients.id = avtxn.client_id';
    if (!isset($table_columns['client_name'])) {
      $group_by['city'] = 'avclients.city';
    }
  }
  if (isset($table_columns['code'])) {
    $select[] = 'avclients.zip_code';
    $joins['clients'] = 'LEFT JOIN {avtbl_clients} AS avclients ON avclients.id = avtxn.client_id';
  }
  if (isset($table_columns['principal_name']) || !empty($user_filter['avcat_principal.title'])) {
    $select[] = 'avcat_principal.title AS principal_name';
    $joins[] = 'LEFT JOIN {avtbl_categories} AS avcat_principal ON avcat_principal.id = avprod.principal_id';
  }
  if (isset($table_columns['item_name'])) {
    $select[] = 'avprod.title';
  }
  if (isset($table_columns['transaction_date'])) {
    $select[] = 'avtxn.transaction_date';
    $group_by['txn_id'] = 'avtxn.transaction_date';
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
    if ($dr_register) {
      $select[] = 'SUM(avtxn_detail.qty) AS qty_out';
    }
    else {
      $select[] = 'SUM(avtxn_detail.qty * avtxn_detail.qty_per_uom * (IF(avtxn.transaction_type = :txn_type_invoice, 1, -1))) AS qty_out';
    }
  }
  if (isset($table_columns['uom'])) {
    if ($dr_register) {
      $select[] = 'avcat_uom.title AS uom_title';
      $joins[] = 'LEFT JOIN {avtbl_categories} AS avcat_uom ON avcat_uom.id = avtxn_detail.uom_id';
      $group_by['uom'] = 'avcat_uom.title';
    }
    else {
      $select[] = 'avcat_uom.title AS uom_title';
      $joins[] = 'LEFT JOIN {avtbl_categories} AS avcat_uom ON avcat_uom.id = avprod.uom_id';
    }
  }
  if (isset($table_columns['price'])) {
    if ($dr_register) {
      $select[] = 'avtxn_detail.cost AS price';
      $group_by['price'] = 'avtxn_detail.cost';
    }
    else {
      $select[] = 'avtxn_detail.cost / avtxn_detail.qty_per_uom AS price';
      $group_by['price'] = 'avtxn_detail.cost / avtxn_detail.qty_per_uom';
    }
  }
  if (!empty($user_filter['avcat_area.title'])) {
    $joins['clients'] = 'LEFT JOIN {avtbl_clients} AS avclients ON avclients.id = avtxn.client_id';
    $joins[] = 'LEFT JOIN {avtbl_categories} AS avcat_area ON avcat_area.id = avclients.area_id';
  }

  $sum_query_increment_string = $dr_register ? '1' : 'avtxn.transaction_type = :txn_type_invoice';
  if (isset($table_columns['discount'])) {
    $select[] = avtxns_txn_detail_sum_query($sum_query_increment_string) . ' AS total_sales';
  }
  $select[] = avtxns_txn_detail_sum_query_discount_applied($sum_query_increment_string) . ' AS net_sales';
  $where[] = "avtxn.transaction_type IN(:txn_types)";
  $where[] = "IF(avtxn.transaction_type = :txn_type_invoice, avtxn_detail.ref_txn_type = :txn_type_delivery, 1)";
  // Only RUDs from sales returns, RPDs shouldn't be included in stt.
  $where[] = "IF(avtxn.transaction_type = :txn_type_sr, avtxn.return_type = :rud, 1)";
  $args[':txn_types'] = $dr_register ? array(AVTXNS_TXN_TYPE_DELIVERY) : array(AVTXNS_TXN_TYPE_INVOICE, AVTXNS_TXN_TYPE_SALES_RETURN);
  $args[':txn_type_invoice'] = AVTXNS_TXN_TYPE_INVOICE;
  $args[':txn_type_delivery'] = AVTXNS_TXN_TYPE_DELIVERY;
  $args[':txn_type_sr'] = AVTXNS_TXN_TYPE_SALES_RETURN;
  $args[':rud'] = AVTXNS_RETURN_TYPE_RUD;




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
    'principal_name' => 'avprod.principal_id',
    'item_name' => 'avprod.id'
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
  //$count_query = "SELECT 1 AS expression FROM {avtbl_products} AS avprod " . implode(' ', $joins);
  //$query = str_replace('1 AS expression', implode(', ', $select), $count_query);
  $query = "SELECT _expression_ FROM {avtbl_products} AS avprod " . implode(' ', $joins);
  // Count max rows before applying supplements to $query.
  avreports_count_max_rows($form, $query, $select, $where, $group_by, $args, $offset);
  avbase_query_supplements($query, $where, $order_by, $limit, $offset, $group_by);
  $rs = db_query($query, $args);

  //$records = avtxns_product_query_load($where, $args, NULL, NULL, $order_by, $select, $joins, $group_by);
  //$records = avtxns_product_query_load($where, $args, NULL, NULL, $order_by, $select, $joins, $group_by);

  // Group result.
  $table_group_by = empty($user_filter['settings']['table_group_by']) ? NULL : $user_filter['settings']['table_group_by'];
  //$grouped_records = array();
  //foreach ($rs as $row) {
  //  $group_id = isset($row->$table_group_by) ? $row->$table_group_by : '';
  //  $grouped_records[check_plain($group_id)][] = $row;
  //}

  $group_totals = $group_discounts = array();
  // List result.
  $rows = array();
  //$index = 0;
  //$total2 = 0;
  //foreach ($grouped_records as $group_id => $group) {
  //  $group_total = 0;
  $group_discount = 0;
  $row_index = 0;
  //$group_index = 0;
  foreach ($rs as $group_row_id => $row) {
    // Get group id.
    $group_id = isset($row->$table_group_by) ? $row->$table_group_by : '';
    // Sum group totals.
    $group_total = empty($group_totals[$group_id]) ? 0 : $group_totals[$group_id];
    $group_totals[$group_id] = $group_total + $row->net_sales;

    // Sum group discounts.
    $row_discount = 0;
    if (isset($table_columns['discount'])) {
      $row_discount = $row->total_sales - $row->net_sales;
      $group_discount = empty($group_discounts[$group_id]) ? 0 : $group_discounts[$group_id];
      $group_discounts[$group_id] = $group_discount + $row_discount;
    }
    // Get group index.
    $group_index = array_search($group_id, array_keys($group_totals));
    $rows[sprintf('%010d', $group_index) . $group_id . '*-' . sprintf('%010d', $row_index++)] = avreports_sales_to_trade_table_row($row, $table_columns, $table_group_by, $group_total, $row_discount, $hide_cols);
  }


  //  $group_totals_row =  array(
  //    'data' => array(
  //      //'spacer' => array('data' => $group_id . ' - NET SALES', 'class' => array(' uk-text-right uk-text-uppercase')),
  //      //'discount_total' => array('data' => number_format($group_discount, 2), 'class' => array('uk-text-right uk-text-bold')),
  //      //'group_discount' => array('data' => strtoupper($group_id) . ' - DISCOUNT TOTAL: <span class="uk-text-bold">' . number_format($group_discount, 2) . '</span>', 'class' => array('uk-text-right')),
  //      'group_total' => array(
  //        'data' => strtoupper($group_id) . ' NET SALES<span class="uk-margin-left uk-text-bold">' . number_format($group_total, 2) . '</span>',
  //        'colspan' => 20, 'class' => array('uk-text-right')),
  //    ),
  //    //'class' => array('uk-row-mute'),
  //  );
  //  if (isset($table_columns['discount'])) {
  //    $group_totals_row['data']['group_total']['data'] = 'TOTAL DISCOUNT GIVEN<span class="uk-margin-left uk-text-bold uk-margin-large-right">' . number_format($group_discount, 2) . '</span>' . $group_totals_row['data']['group_total']['data'];
  //  }
  //  if (empty($table_columns)) {
  //    //unset($group_totals_row['data']['spacer']);
  //  }
  //  else {
  //    //$group_totals_row['data']['spacer']['colspan'] = count($table_columns);
  //  }
  //  $rows[] = $group_totals_row;
  //  $rows[] = array('data' => array('-' => array('data' => '&nbsp', 'colspan' => 20)));
  //  //$rows[] = array('data' => array('-' => array('colspan' => 10)));
  //}

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

  if ($limit === NULL) {
    $group_index = 0;
    $totals_colspan = count($header);
    foreach ($group_totals as $group_id => $net_sales) {
      $group_totals_row =  array(
        'group_total' => array(
          'data' => strtoupper($group_id) . ' NET SALES <span class="uk-margin-left uk-text-bold">' . number_format((empty($group_totals[$group_id]) ? 0 : $group_totals[$group_id]), 2) . '</span>',
          'colspan' => $totals_colspan,
        ),
      );

      if (isset($table_columns['discount'])) {
        $group_discount = empty($group_discounts[$group_id]) ? 0 : $group_discounts[$group_id];
        $group_totals_row['group_total']['data'] = 'TOTAL DISCOUNT USED <span class="uk-margin-left uk-text-bold uk-margin-large-right">' . number_format($group_discount, 2) . '</span> ' . $group_totals_row['group_total']['data'];
      }
      $group_totals_row['group_total']['data'] = '<div class="uk-text-right">' . $group_totals_row['group_total']['data'] . '</div>';
      $row_wtf_id = sprintf('%010d', $group_index++) . $group_id;
      $rows[$row_wtf_id . '_TOTAL'] = $group_totals_row;
      $rows[$row_wtf_id . '_z'] = array(array('data' => NULL, 'colspan' => $totals_colspan));
    }

  }

  if (!empty($table_group_by)) {
    ksort($rows);
  }



  $table_attributes = array(
    'id' => 'transaction-list-table',
    'class' => array('uk-table-condensed uk-table-hover uk-text-small uk-text-uppercase'),
  );

  //$pager_html = empty($options['#hide_filter_form']) ? theme('pager', array('tags' => array())) : '';
  $table = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => '<div class="uk-text-muted">' . t('No match found.') . '</div>',
    '#attributes' => $table_attributes,
  );


  return $table;
}