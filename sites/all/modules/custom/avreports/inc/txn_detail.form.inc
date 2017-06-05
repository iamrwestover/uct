<?php

function avreports_txn_detail_form($form, &$form_state, $transaction_type, $limit = NULL, $offset = NULL) {
  ini_set('memory_limit', AVBASE_EXPANDED_MEMORY_LIMIT);

  // Transaction type details.
  $ttd = avtxns_txn_types($transaction_type);

  // Special flag for sales return.
  $is_sr = $transaction_type == AVTXNS_TXN_TYPE_SALES_RETURN;

  // Extra columns
  $col_return_type = $is_sr;
  $col_reference_id = in_array($transaction_type, avtxns_get_types_with_reference_id());

  $dad = avtxns_txn_list_filter_date_auto_options();
  $dad_default_key = 'this_week';
  $dad_default = empty($dad[$dad_default_key]) ? array() : $dad[$dad_default_key];
  $filter_form_settings = array(
    'date_auto' => array(
      '#title' => 'Dates',
      '#default_value' => $dad_default_key,
    ),
    'date_from' => array(
      '#title' => 'Date from',
      '#default_value' => $dad_default['date_from'],
    ),
    'date_to' => array(
      '#title' => 'Date to',
      '#default_value' => $dad_default['date_to'],
    ),
    'simplified_status' => array(
      '#title' => 'Status',
      '#options' => array(
        AVTXNS_TXN_STATUS_LOCKED . '-' . AVTXNS_TXN_STATUS_OPEN => 'Open',
      ),
      '#default_value' => AVTXNS_TXN_STATUS_LOCKED . '-' . AVTXNS_TXN_STATUS_OPEN . '-' . AVTXNS_TXN_STATUS_CLOSED,
    ),
    'avprod.title' => array(
      '#title' => t('Product'),
    ),
    'avcat_principal.title' => array(
      '#title' => t('Principal'),
      '#autocomplete_path' => 'av/categories/principal/autocomplete',
    ),
    'avtxn.reference_id' => array(
      '#title' => t('Reference ID'),
    ),
    'reset_button' => array('#attributes' => array('style' => 'margin-top: 25px;')),
    'export_button' => array(
      '#func_name' => __FUNCTION__ ,
      '#func_args' => array($transaction_type),
      '#report_name' => 'Stocks Returned - ' . $ttd['transaction'],
      '#report_inc' => 'txn_detail.form',
    ),
  );
  $filter_form_settings['simplified_status']['#options'][AVTXNS_TXN_STATUS_CLOSED] = 'Closed';
  $filter_form_settings['simplified_status']['#options'][AVTXNS_TXN_STATUS_LOCKED . '-' . AVTXNS_TXN_STATUS_OPEN . '-' . AVTXNS_TXN_STATUS_CLOSED] = 'All';
  //$filter_form_settings['simplified_status']['#options'][AVTXNS_TXN_STATUS_VOID] = 'Void';
  $filter_form_settings['avclients.display_name'] = array(
    '#title' => 'Customer',
    '#autocomplete_path' => 'av/clients/autocomplete/' . $ttd['ctd']['type'] . '/' . implode('-', array(AVCLIENT_STATUS_ACTIVE, AVCLIENT_STATUS_SUSPENDED)),
  );
  if ($ttd['ctd']['type'] == AVCLIENT_TYPE_CUSTOMER) {
    $filter_form_settings['avcat_area.title'] = array('#title' => 'Area');
  }
  $filter_form_settings['avcat_agent.title'] = array(
    '#title' => 'Agent',
    '#autocomplete_path' => 'av/categories/agent' .$ttd['ctd']['type'] . '/autocomplete',
  );
  $filter_form_settings['avclients.category_id'] = array('#title' => 'Channel', '#options' => avbase_get_category_options_array('category' . $ttd['ctd']['type']));
  $filter_form_settings['avcat_prodcat.id'] = array('#options' => avbase_get_category_options_array('product_cat'));
  if ($col_return_type) {
    $filter_form_settings['return_type'] = array('#title' => 'Return type');
  }
  $form['transaction_list_filter'] = avtxns_txn_list_filter_form($form, $form_state, $filter_form_settings);

  // User filters.
  $user_filter = empty($form_state['values']['avtxns_tl_filter']) ? array() : $form_state['values']['avtxns_tl_filter'];
  $filter_query = empty($user_filter['filter_query']) ? array() : $user_filter['filter_query'];
  $where = empty($filter_query['where']) ? array() : $filter_query['where'];
  $args = empty($filter_query['args']) ? array() : $filter_query['args'];

  // Show hide columns depending on selected filters.
  $col_client_show = empty($user_filter['avclients.display_name']);
  $col_agent_show = empty($user_filter['avcat_agent.title']);
  $col_product_show = empty($user_filter['avprod.title']);

  // Headers.
  $header = array();
  $header['date'] = array('data' => 'DATE', 'field' => 'transaction_date');
  $header['id'] = array('data' => 'DR#', 'field' => 'avtxn.id');
  if ($col_return_type) {
    $header['return_type'] = array('data' => 'RET. TYPE', 'field' => 'avtxn.return_type');
  }
  if ($col_reference_id) {
    $header['reference_id'] = array('data' => 'REF. ID', 'field' => 'avtxn.reference_id');
  }
  if ($col_client_show) {
    $header['client'] = array('data' => strtoupper($ttd['ctd']['name']), 'field' => 'display_name');
  }
  if ($col_product_show) {
    $header['product'] = array('data' => 'PRODUCT', 'field' => 'avprod.title');
    if ($is_sr) {
      $header['actions'] = NULL;
    }
  }
  $header['row_qty'] = array('data' => 'QTY', 'field' => 'row_qty');
  if ($is_sr) {
    $header['row_qty_good'] = array('data' => 'GOOD', 'field' => 'row_qty_good');
    $header['row_qty_bad'] = array('data' => 'BAD', 'field' => 'row_qty_bad');
    //$header['row_qty_prd'] = array('data' => 'PR QTY', 'field' => 'row_qty_prd');
  }
  $header['uom'] = array('data' => 'UOM', 'field' => 'base_uom_title');
  //$header['term'] = array('data' => 'Terms', 'field' => 'term_name');
  //$header['onhand'] = array('data' => 'ON HAND', 'field' => 'onhand');
  //if ($col_agent_show) {
  //  $header['agent'] = array('data' => 'AGENT', 'field' => 'agent_name');
  //}

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
    $_GET['order'] = $header['id']['data'];
    $_GET['sort'] = 'DESC';
  }
  $order_by = avbase_build_header_order_array($header);

  $select = array();
  $joins = array();
  $group_by = array();
  $select[] = 'avtxn.id';
  $select[] = 'avtxn.status';
  $select[] = 'avtxn.transaction_date';
  if ($col_return_type) {
    $select[] = 'avtxn.return_type';
    $return_types = avtxns_get_return_types();
  }
  if ($col_reference_id) {
    $select[] = 'avtxn.reference_id';
  }
  $select[] = 'avtxn_detail.item_id';
  //$select[] = 'avtxn_detail.ref_txn_type';
  //$select[] = 'avtxn_detail.ref_txn_id';
  $select[] = 'SUM(avtxn_detail.qty * avtxn_detail.qty_per_uom) AS row_qty';
  if ($is_sr) {
    $select[] = 'SUM(IF(avtxn_detail.good_stock = 1, avtxn_detail.qty * avtxn_detail.qty_per_uom, NULL)) AS row_qty_good';
    $select[] = 'SUM(IF(avtxn_detail.good_stock = 0, avtxn_detail.qty * avtxn_detail.qty_per_uom, NULL)) AS row_qty_bad';
    //$select[] = "(
    //  IF (
    //  avtxn.status = " . AVTXNS_TXN_STATUS_VOID . ",
    //  0,
    //  (SELECT SUM(pr_detail.qty * pr_detail.qty_per_uom) FROM {avtbl_transaction_details} pr_detail
    //  INNER JOIN {avtbl_transactions} pr ON pr.id = pr_detail.transaction_id
    //  WHERE pr.transaction_type = '" . AVTXNS_TXN_TYPE_PURCHASE_RETURN . "'
    //    AND pr_detail.item_id = avtxn_detail.item_id
    //    AND pr.status <> " . AVTXNS_TXN_STATUS_VOID . ")
    //  )
    //) AS row_qty_prd";
  }
  $joins[] = 'INNER JOIN {avtbl_transaction_details} AS avtxn_detail ON avtxn_detail.transaction_id = avtxn.id';
  $select[] = 'avprod.title AS product_title';
  //$select[] = 'avprod.qty AS onhand';
  $joins[] = 'INNER JOIN {avtbl_products} AS avprod ON avprod.id = avtxn_detail.item_id';
  $select[] = 'avclients.id AS client_id';
  $select[] = 'avclients.display_name';
  $joins[] = 'INNER JOIN {avtbl_clients} AS avclients ON avclients.id = avtxn.client_id';
  //$select[] = 'avcat_uom.title AS uom_title';
  //$joins[] = 'LEFT JOIN {avtbl_categories} AS avcat_uom ON avcat_uom.id = avtxn_detail.uom_id';
  $select[] = 'avcat_buom.title AS base_uom_title';
  $joins[] = 'LEFT JOIN {avtbl_categories} AS avcat_buom ON avcat_buom.id = avprod.uom_id';
  $joins[] = 'LEFT JOIN {avtbl_categories} AS avcat_area ON avcat_area.id = avclients.area_id';
  $select[] = 'avcat_agent.title AS agent_name';
  $joins[] = 'LEFT JOIN {avtbl_categories} AS avcat_agent ON avcat_agent.id = avtxn.agent_id';
  //$select[] = 'avcat_term.title AS term_name';
  //$joins[] = 'LEFT JOIN {avtbl_categories} AS avcat_term ON avcat_term.id = avtxn.term_id';
  $joins[] = 'LEFT JOIN {avtbl_categories} AS avcat_prodcat ON avcat_prodcat.id = avprod.category_id';
  $joins[] = 'LEFT JOIN {avtbl_categories} AS avcat_principal ON avcat_principal.id = avprod.principal_id';

  $query = "SELECT _expression_ FROM {avtbl_transactions} AS avtxn " . implode(' ', $joins);
  $where[] = 'avtxn.transaction_type = :ttype';
  $args[':ttype'] = $transaction_type;

  $group_by[] = 'avtxn.id';
  $group_by[] = 'avtxn_detail.item_id';
  // Count max rows before applying supplements to $query.
  avreports_count_max_rows($form, $query, $select, $where, $group_by, $args, $offset);
  avbase_query_supplements($query, $where, $order_by, NULL, NULL, $group_by);
  $rs = db_query($query, $args);
  $table_rows = array();
  //$irtd = avtxns_txn_types(AVTXNS_TXN_TYPE_SALES_ORDER);
  $row_qty_total = $row_qty_good_total = $row_qty_bad_total = 0;
  foreach ($rs as $row) {
    $data = array();
    $data['date'] = format_date($row->transaction_date, 'custom', 'M-d-Y');
    $data['id'] = l(avbase_txn_number_format($transaction_type, $row->id), "{$ttd['base_path']}/$row->id/view");
    if ($col_return_type) {
      $data['return_type'] = empty($row->return_type) ? NULL : $return_types[$row->return_type];
    }
    if ($col_reference_id) {
      $data['reference_id'] = empty($row->reference_id) ? NULL : check_plain($row->reference_id);
    }

    if ($col_client_show) {
      $data['client'] = l($row->display_name, $ttd['ctd']['base_path'] . '/' . $row->client_id . '/view');
    }
    if ($col_product_show) {
      $data['product'] = l($row->product_title, 'av/products/' . $row->item_id . '/view');
      if ($is_sr) {
        $data['actions'] = avtxns_txn_list_table_pr_link($row->product_title, $user_filter, AVTXNS_TXN_TYPE_PURCHASE_RETURN);
        $data['actions'] .= ' &nbsp; ' . avtxns_txn_list_table_phl_link($row->product_title, $user_filter, 'hist.');
        $data['actions'] = '<div class="not-printable">' . $data['actions'] . '</div>';
      }
    }
    $data['row_qty'] = number_format($row->row_qty);
    $row_qty_total += $row->row_qty;
    if ($is_sr) {
      $data['row_qty_good'] = number_format($row->row_qty_good);
      $data['row_qty_bad'] = number_format($row->row_qty_bad);
      $row_qty_good_total += $row->row_qty_good;
      $row_qty_bad_total += $row->row_qty_bad;
      //$data['row_qty_prd'] = number_format($row->row_qty_prd);
    }
    $data['uom'] = check_plain($row->base_uom_title);

    //$data['term'] = check_plain($row->term_name);
    //$data['onhand'] = number_format($row->onhand) . ' ' . check_plain($row->base_uom_title);

    //if ($col_agent_show) {
    //  $data['agent'] = check_plain($row->agent_name);
    //}
    $table_rows[] = $data;
  }

  if (!empty($table_rows) && $limit === NULL) {
    $table_rows[] = array(array('data' => NULL, 'colspan' => count($header)));
    $label_colspan = $colspan = array_search('row_qty', array_keys($header));
    $totals_table_row = array();
    $totals_table_row[] = array(
      'data' => '<div class="uk-text-right">Total</div>',
      'colspan' => $label_colspan
    );
    $totals_table_row[] = number_format($row_qty_total);
    if ($is_sr) {
      $totals_table_row[] = number_format($row_qty_good_total);
      $totals_table_row[] = number_format($row_qty_bad_total);
    }
    $remaining_cols = count($header) - ($colspan + (count($totals_table_row) - 1));
    if ($remaining_cols) {
      $totals_table_row[] = array('data' => NULL, 'colspan' => $remaining_cols);
    }
    $table_rows[] = $totals_table_row;
  }

  // Build table.
  $transaction_list_table['table'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $table_rows,
    '#empty' => '<div class="uk-text-muted">' . t('No match found.') . '</div>',
    '#attributes' => array(
      'id' => 'transaction-list-table',
      'class' => array('uk-table-condensed uk-table-hover uk-text-small'),
    ),
  );

  // Show filters used.
  if (isset($user_filter['simplified_status'])) {
    $transaction_list_table['simplified_status'] = array(
      '#type' => 'item',
      '#markup' => check_plain($filter_form_settings['simplified_status']['#options'][$user_filter['simplified_status']]) . ' ' . $ttd['plural_name'],
    );
    if (!empty($user_filter['overdue'])) {
      $transaction_list_table['simplified_status']['#markup'] .= ' (OVERDUE)';
    }
    $transaction_list_table['simplified_status']['#markup'] = strtoupper($transaction_list_table['simplified_status']['#markup']);
  }
  avtxns_txn_list_table_add_date_from($transaction_list_table, $user_filter);
  avtxns_txn_list_table_add_product_title($transaction_list_table, $user_filter, $transaction_type);

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
  if (!empty($user_filter['avcat_area.title'])) {
    $transaction_list_table['area_name'] = array(
      '#type' => 'item',
      '#title' => 'Area',
      '#title_display' => 'inline-before',
      '#markup' => check_plain($user_filter['avcat_area.title']),
    );
  }
  if (!empty($user_filter['avcat_agent.title'])) {
    $transaction_list_table['agent_name'] = array(
      '#type' => 'item',
      '#title' => 'Agent',
      '#title_display' => 'inline-before',
      '#markup' => check_plain($user_filter['avcat_agent.title']),
    );
  }
  if (!empty($user_filter['avclients.category_id'])) {
    $transaction_list_table['avclients.category_id'] = array(
      '#type' => 'item',
      '#title' => 'Channel',
      '#title_display' => 'inline-before',
      '#markup' => check_plain($filter_form_settings['avclients.category_id']['#options'][$user_filter['avclients.category_id']]),
    );
  }
  if (!empty($user_filter['avcat_prodcat.id'])) {
    $transaction_list_table['avcat_prodcat.id'] = array(
      '#type' => 'item',
      '#title' => 'Product Category',
      '#title_display' => 'inline-before',
      '#markup' => check_plain($filter_form_settings['avcat_prodcat.id']['#options'][$user_filter['avcat_prodcat.id']]),
    );
  }
  if (!empty($user_filter['avtxn.reference_id'])) {
    $transaction_list_table['avtxn.reference_id'] = array(
      '#type' => 'item',
      '#title' => 'Reference ID',
      '#title_display' => 'inline-before',
      '#markup' => check_plain($user_filter['avtxn.reference_id']),
    );
  }


  // Add table to form.
  $form['transaction_list_table'] = array(
    '#type' => 'container',
    '#theme' => 'avreports_txn_list',
    '#attributes' => array(
      'id' => 'transaction-list-table-wrapper',
      'class' => array('printable av-sticky-small')
    ),
  );
  $form['transaction_list_table'] += $transaction_list_table;

  // Buttons.
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