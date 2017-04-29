<?php
function avreports_sales_to_trade_form($form, &$form_state) {
  $client_statuses = array(AVCLIENT_STATUS_ACTIVE, AVCLIENT_STATUS_SUSPENDED);
  $ctd = avclients_client_types(AVCLIENT_TYPE_CUSTOMER);
  $dad = avtxns_txn_list_filter_date_auto_options();
  $dad_default = empty($dad['this_month']) ? array() : $dad['this_month'];
  $filter_form_settings = array(
    'date_auto' => array(
      '#title' => t('Dates'),
      '#default_value' => 'this_month',
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
        'client_address' => 'Address',
        'principal_name' => 'Principal',
        'item_name' => 'Product',
        'qty_out' => 'Qty.',
        'uom' => 'UOM',
      ),
      '#default_value' => array(
        'agent_name',
        'principal_name',
        'client_name',
        'client_address',
        'item_name',
        'qty_out',
        'uom',
      ),
    ),
    'table_group_by' => array(
      '#options' => array(
        '' => 'No grouping',
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
  $select = $joins = $where = $args = array();
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
    'item_name' => array('data' => 'PRODUCT', 'field' => 'avproducts.title'),
    'qty_out' => array('data' => 'QTY.', 'field' => 'qty_out'),
    'uom' => array('data' => 'UOM', 'field' => 'avcat_uom.title'),
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
  $joins[] = 'INNER JOIN {avtbl_transactions} AS avtxn ON avtxn.id = avtxn_detail.transaction_id';
  $where[] = "avtxn.status <> " . AVTXNS_TXN_STATUS_VOID;
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
  if (isset($table_columns['qty_out'])) {
    $select[] = 'SUM(avtxn_detail.qty * avtxn_detail.qty_per_uom) AS qty_out';
  }
  if (isset($table_columns['uom'])) {
    $select[] = 'avcat_uom.title AS uom_title';
    $joins[] = 'LEFT JOIN {avtbl_categories} AS avcat_uom ON avcat_uom.id = avproducts.uom_id';
  }
  //$joins[] = 'LEFT JOIN {avtbl_categories} AS avcat_area ON avcat_area.id = avclients.area_id';

  $select[] = avtxns_txn_detail_sum_query_discount_applied() . ' AS net_sales';
  $where[] = "avtxn.transaction_type = :txn_type_invoice";
  $where[] = "avtxn_detail.ref_txn_type = :txn_type_delivery";
  $args[':txn_type_invoice'] = AVTXNS_TXN_TYPE_INVOICE;
  $args[':txn_type_delivery'] = AVTXNS_TXN_TYPE_DELIVERY;

  if (empty($_GET['order'])) {
    $_GET['order'] = 'Total Sales';
    $_GET['sort'] = 'desc';
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
  $group_by = array();
  $group_by = array(
    'agent_name' => 'avtxn.agent_id',
    'client_name' => 'avtxn.client_id',
    'principal_name' => 'avproducts.principal_id',
    'item_name' => 'avproducts.id'
  );
  foreach ($group_by as $g_k => $g) {
    if (!isset($table_columns[$g_k])) {
      if ($g_k == 'client_name' && isset($table_columns['client_address'])) {
        continue;
      }
      unset($group_by[$g_k]);
    }
  }
  $records = avtxns_product_query_load($where, $args, NULL, NULL, $order_by, $select, $joins, $group_by);

  // Group result.
  $table_group_by = empty($user_filter['settings']['table_group_by']) ? NULL : $user_filter['settings']['table_group_by'];
  $grouped_records = array();
  foreach ($records as $row) {
    $group_id = isset($row->$table_group_by) ? $row->$table_group_by : '';
    $grouped_records[check_plain($group_id)][] = $row;
  }

  // List result.
  $rows = array();
  $index = 0;
  $total2 = 0;
  foreach ($grouped_records as $group_id => $group) {
    $group_total = 0;
    foreach ($group as $row) {
      $table_row = array(
        'data' => array(
          //'placeholder' => array('data' => $index + 1, 'class' => array('suk-hidden')),
          'agent_name' => isset($table_columns['agent_name']) ? check_plain($row->agent_name) : '',
          'client_name' => isset($table_columns['client_name']) ? check_plain($row->client_name) : '',
          'client_address' => isset($table_columns['client_address']) ? avclients_address_string_format($row) : '',
          'principal_name' => isset($table_columns['principal_name']) ? check_plain($row->principal_name) : '',
          'item_name' => isset($table_columns['item_name']) ? check_plain($row->title) : '',
          'qty_out' => isset($table_columns['qty_out']) ? number_format($row->qty_out) : '',
          'uom' => isset($table_columns['uom']) ? check_plain($row->uom_title) : '',
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
      $total2 += $row->net_sales;
      $group_total += $row->net_sales;

      $rows[] = $table_row;
      $index++;
    }

    $group_totals_row =  array(
      'data' => array(
        'spacer' => array('data' => $group_id . ' - Total', 'colspan' => 1, 'class' => array(' uk-text-right uk-text-uppercase')),
        'total2' => array('data' => number_format($group_total, 2), 'class' => array('uk-text-right uk-text-bold')),
      ),
      //'class' => array('uk-row-mute'),
    );
    if (empty($table_columns)) {
      unset($group_totals_row['data']['spacer']);
    }
    else {
      $group_totals_row['data']['spacer']['colspan'] = count($table_columns);
    }
    $rows[] = $group_totals_row;
    $rows[] = array('data' => array('-' => array('data' => '&nbsp', 'colspan' => 10)));
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
