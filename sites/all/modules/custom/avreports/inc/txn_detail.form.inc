<?php

function avreports_txn_detail_form($form, &$form_state, $transaction_type, $limit = NULL, $offset = NULL) {
  ini_set('memory_limit', AVBASE_EXPANDED_MEMORY_LIMIT);

  // Transaction type details.
  $ttd = avtxns_txn_types($transaction_type);

  // Special flag for sales return.
  $is_sr = $transaction_type == AVTXNS_TXN_TYPE_SALES_RETURN;

  // Pending ref types.
  $pending_txns_reference_type = empty($ttd['pending_txns_reference_type']) ? array() : (array) $ttd['pending_txns_reference_type'];

  // Extra columns
  $col_client = !empty($ttd['ctd']);
  $col_accounts = $ttd['transaction'] == 'accounts';
  //$col_accounts = FALSE;
  $col_return_type = $is_sr;
  $col_reference_id = in_array($transaction_type, avtxns_get_types_with_reference_id());
  $col_product = empty($ttd['non_product']);
  $col_uom = $col_product;
  $col_ref_txn_id = !empty($pending_txns_reference_type);
  $col_total = !empty($ttd['payment']);

  $dad = avtxns_txn_list_filter_date_auto_options();
  $dad_default_key = 'this_week';
  $dad_default = empty($dad[$dad_default_key]) ? array() : $dad[$dad_default_key];
  $filter_form_settings = array(
    'avtxn.id' => array('#title' => 'Txn. #'),
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
      '#default_value' => AVTXNS_TXN_STATUS_LOCKED . '-' . AVTXNS_TXN_STATUS_OPEN . '-' . AVTXNS_TXN_STATUS_CLOSED . '-' . AVTXNS_TXN_STATUS_PENDING,
    ),
    //'avprod.title' => array(
    //  '#title' => t('Product'),
    //),
    //'avcat_principal.title' => array(
    //  '#title' => t('Principal'),
    //  '#autocomplete_path' => 'av/categories/principal/autocomplete',
    //),
    'avtxn.reference_id' => array(
      '#title' => t('Reference ID'),
    ),
    'avachart.title' => array(
      '#title' => t('Account Name'),
    ),
    'reset_button' => array('#attributes' => array('style' => 'margin-top: 25px;')),
    'export_button' => array(
      '#func_name' => __FUNCTION__ ,
      '#func_args' => array($transaction_type),
      '#report_name' => 'Transaction Details - ' . $ttd['name'],
      '#report_inc' => 'txn_detail.form',
    ),
  );
  if ($col_product) {
    $filter_form_settings['avprod.title'] = array('#title' => t('Product'));
  }
  $filter_form_settings['simplified_status']['#options'][AVTXNS_TXN_STATUS_CLOSED] = 'Closed';
  $filter_form_settings['simplified_status']['#options'][AVTXNS_TXN_STATUS_LOCKED . '-' . AVTXNS_TXN_STATUS_OPEN . '-' . AVTXNS_TXN_STATUS_CLOSED . '-' . AVTXNS_TXN_STATUS_PENDING] = 'All';
  //$filter_form_settings['simplified_status']['#options'][AVTXNS_TXN_STATUS_VOID] = 'Void';

  if ($col_client) {
    $filter_form_settings['avclients.display_name'] = array(
      '#title' => 'Customer',
      '#autocomplete_path' => 'av/clients/autocomplete/' . $ttd['ctd']['type'] . '/' . implode('-', array(AVCLIENT_STATUS_ACTIVE, AVCLIENT_STATUS_SUSPENDED)),
    );
    $filter_form_settings['avcat_agent.title'] = array(
      '#title' => 'Agent',
      '#autocomplete_path' => 'av/categories/agent' .$ttd['ctd']['type'] . '/autocomplete',
    );
    $filter_form_settings['avclients.category_id'] = array('#title' => 'Channel', '#options' => avbase_get_category_options_array('category' . $ttd['ctd']['type']));
  }
  if ($col_client && $ttd['ctd']['type'] == AVCLIENT_TYPE_CUSTOMER) {
    $filter_form_settings['avcat_area.title'] = array('#title' => 'Area');
  }
  if ($col_product) {
    $filter_form_settings['avcat_prodcat.id'] = array('#options' => avbase_get_category_options_array('product_cat'));
    $filter_form_settings['avcat_principal.title'] = array('#title' => t('Principal'), '#autocomplete_path' => 'av/categories/principal/autocomplete');
  }
  if ($col_return_type) {
    $filter_form_settings['return_type'] = array('#title' => 'Return type');
  }
  if ($col_ref_txn_id) {
    $filter_form_settings['avtxn_detail.ref_txn_id'] = array('#title' => 'Ref. Txn. #');
  }
  $form['transaction_list_filter'] = avtxns_txn_list_filter_form($form, $form_state, $filter_form_settings);

  // User filters.
  $user_filter = empty($form_state['values']['avtxns_tl_filter']) ? array() : $form_state['values']['avtxns_tl_filter'];
  $filter_query = empty($user_filter['filter_query']) ? array() : $user_filter['filter_query'];
  $where = empty($filter_query['where']) ? array() : $filter_query['where'];
  $args = empty($filter_query['args']) ? array() : $filter_query['args'];

  // Show hide columns depending on selected filters.
  $col_client_show = $col_client && empty($user_filter['avclients.display_name']);
  $col_agent_show = $col_client && empty($user_filter['avcat_agent.title']);
  $col_product_show = $col_product && empty($user_filter['avprod.title']);

  // Headers.
  $header = array();
  $header['date'] = array('data' => 'DATE', 'field' => 'transaction_date');
  $header['id'] = array('data' => 'TXN.#', 'field' => 'avtxn.id');
  if ($col_accounts) {
    $header['account_title'] = array('data' => 'ACCOUNT NAME', 'field' => 'avachart.title');
    $header['description'] = array('data' => 'DESCRIPTION');
    $header['debit'] = array('data' => 'DEBIT', 'field' => 'avtxn_detail.debit', 'class' => array('uk-text-right'));
    $header['credit'] = array('data' => 'CREDIT', 'field' => 'avtxn_detail.credit', 'class' => array('uk-text-right'));
  }
  if ($col_ref_txn_id) {
    $header['items_ref_txn_id'] = array('data' => 'REF. TXN.#', 'field' => 'avtxn_detail.ref_txn_id');
  }
  if ($col_total) {
    $header['grand_total'] = array('data' => 'PAID', 'class' => array('uk-text-right'));
  }
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
    $header['vendor_name'] = array('data' => 'PREFERRED VENDOR', 'field' => 'avprod.title');
  }
  if ($col_product) {
    $header['row_qty'] = array('data' => 'QTY', 'field' => 'row_qty');
  }
  if ($is_sr) {
    $header['row_qty_good'] = array('data' => 'GOOD', 'field' => 'row_qty_good');
    $header['row_qty_bad'] = array('data' => 'BAD', 'field' => 'row_qty_bad');
    //$header['row_qty_prd'] = array('data' => 'PR QTY', 'field' => 'row_qty_prd');
  }
  if ($col_uom) {
    $header['uom'] = array('data' => 'UOM', 'field' => 'base_uom_title');
  }
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
  if ($col_ref_txn_id) {
    $select[] = 'avtxn_detail.ref_txn_id';
    $select[] = 'avtxn_detail.ref_txn_type';
  }
  if ($col_total) {
    $select[] = avtxns_txn_detail_sum_query_discount_applied() . " + IFNULL(avtxn.amount_to_credit, 0) AS grand_total";
  }
  if ($col_reference_id) {
    $select[] = 'avtxn.reference_id';
  }
  if ($col_product) {
    $select[] = 'avtxn_detail.item_id';
  }
  //$select[] = 'avtxn_detail.ref_txn_type';
  //$select[] = 'avtxn_detail.ref_txn_id';
  if ($col_product) {
    $select[] = 'SUM(avtxn_detail.qty * avtxn_detail.qty_per_uom) AS row_qty';
  }
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
  if ($col_product) {
    $select[] = 'avprod.title AS product_title';
    $joins[] = 'INNER JOIN {avtbl_products} AS avprod ON avprod.id = avtxn_detail.item_id';
    $select[] = 'avvendors.display_name AS vendor_name';
    $joins[] = 'LEFT JOIN {avtbl_clients} AS avvendors ON avvendors.id = avprod.vendor_id';
  }
  if ($col_client) {
    $select[] = 'avclients.id AS client_id';
    $select[] = 'avclients.display_name';
    $joins[] = 'INNER JOIN {avtbl_clients} AS avclients ON avclients.id = avtxn.client_id';
  }
  if ($col_accounts) {
    $select[] = 'avtxn_detail.description';
    $select[] = 'avtxn_detail.debit';
    $select[] = 'avtxn_detail.credit';
    $select[] = 'avachart.title AS account_title';
    $joins[] = 'LEFT JOIN {avtbl_account_chart} AS avachart ON avachart.id = avtxn_detail.account_id';
  }

  //$select[] = 'avcat_uom.title AS uom_title';
  //$joins[] = 'LEFT JOIN {avtbl_categories} AS avcat_uom ON avcat_uom.id = avtxn_detail.uom_id';
  if ($col_uom) {
    $select[] = 'avcat_buom.title AS base_uom_title';
    $joins[] = 'LEFT JOIN {avtbl_categories} AS avcat_buom ON avcat_buom.id = avprod.uom_id';
  }
  if ($col_client) {
    $joins[] = 'LEFT JOIN {avtbl_categories} AS avcat_area ON avcat_area.id = avclients.area_id';
    $select[] = 'avcat_agent.title AS agent_name';
    $joins[] = 'LEFT JOIN {avtbl_categories} AS avcat_agent ON avcat_agent.id = avtxn.agent_id';
  }


  //$select[] = 'avcat_term.title AS term_name';
  //$joins[] = 'LEFT JOIN {avtbl_categories} AS avcat_term ON avcat_term.id = avtxn.term_id';
  if ($col_product) {
    $joins[] = 'LEFT JOIN {avtbl_categories} AS avcat_prodcat ON avcat_prodcat.id = avprod.category_id';
    $joins[] = 'LEFT JOIN {avtbl_categories} AS avcat_principal ON avcat_principal.id = avprod.principal_id';
  }

  $ttypes = (array) $transaction_type;
  $default_account_ids = array();
  $default_account_details = array();
  if ($transaction_type == AVTXNS_TXN_TYPE_JOURNAL) {
    $ttypes = avtxns_get_accounts_txn_types();
    $select[] = 'avtxn.transaction_type';
    $select[] = 'avtxn_detail.cost';
    $where[] = 'avtxn_detail.ref_txn_id IS NULL';

    // Load details of txn default accounts.
    foreach ($ttypes as $tt) {
      if ($tt == AVTXNS_TXN_TYPE_JOURNAL) {
        continue;
      }
      $this_ttd = avtxns_txn_types($tt);
      $account_id = key($this_ttd['default_account_id']);
      $default_account_ids[$account_id] = $account_id;
    }
    $default_account_details = avtxns_account_chart_load_ids($default_account_ids);
  }

  $query = "SELECT _expression_ FROM {avtbl_transactions} AS avtxn " . implode(' ', $joins);
  $where[] = 'avtxn.transaction_type IN (:ttypes)';
  $args[':ttypes'] = $ttypes;

  $group_by[] = 'avtxn.id';
  if ($col_product) {
    $group_by[] = 'avtxn_detail.item_id';
  }
  if ($col_ref_txn_id) {
    $group_by[] = 'avtxn_detail.ref_txn_id';
  }
  if ($col_accounts) {
    $group_by[] = 'avtxn_detail.id';
    $order_by['avtxn_detail.id'] = 'avtxn_detail.id ASC';
  }
  // Count max rows before applying supplements to $query.
  avreports_count_max_rows($form, $query, $select, $where, $group_by, $args, $offset);
  avbase_query_supplements($query, $where, $order_by, $limit, $offset, $group_by);
  $rs = db_query($query, $args);
  $table_rows = array();
  //$irtd = avtxns_txn_types(AVTXNS_TXN_TYPE_SALES_ORDER);
  $row_qty_total = $row_qty_good_total = $row_qty_bad_total = 0;
  $debit_total = $credit_total = 0;
  foreach ($rs as $row) {
    $data = array();
    $data['date'] = format_date($row->transaction_date, 'custom', AVREPORTS_LIST_DATE_FORMAT);
    $this_ttype = $transaction_type;
    $this_ttd = $ttd;
    if ($transaction_type == AVTXNS_TXN_TYPE_JOURNAL) {
      $this_ttype = $row->transaction_type;
      $this_ttd = avtxns_txn_types($this_ttype);
    }
    $data['id'] = l(avbase_txn_number_format($this_ttype, $row->id), "{$this_ttd['base_path']}/$row->id/view");
    if ($col_accounts) {
      $data['account_title'] = check_plain($row->account_title);
      $data['description'] = check_plain($row->description);
      if ($transaction_type == AVTXNS_TXN_TYPE_JOURNAL && $this_ttype != AVTXNS_TXN_TYPE_JOURNAL) {
        $default_account = $this_ttd['default_account_id'];
        $amt_type = array_shift($default_account);
        $amt_type = $amt_type == 'credit' ? 'debit' : 'credit';
        $row->$amt_type = $row->cost;
      }
      $data['debit'] = '<div class="uk-text-right">' . (empty((float) $row->debit) ? NULL : number_format($row->debit, 2)) . '</div>';
      $data['credit'] = '<div class="uk-text-right">' . (empty((float) $row->credit) ? NULL : number_format($row->credit, 2)) . '</div>';
      $debit_total += $row->debit;
      $credit_total += $row->credit;
    }
    if ($col_ref_txn_id && !empty($row->ref_txn_id) && in_array($row->ref_txn_type, $pending_txns_reference_type)) {
      $irtd = avtxns_txn_types($row->ref_txn_type);
      $irti_view_path = "{$irtd['base_path']}/{$row->ref_txn_id}/view";
      $data['items_ref_txn_id'] = l(avbase_txn_number_format($row->ref_txn_type, $row->ref_txn_id), $irti_view_path);
    }
    if ($col_total) {
      $data['grand_total'] = '<div class="uk-text-right">' . number_format($row->grand_total, 2) . '</div>';
    }
    if ($col_return_type) {
      $data['return_type'] = empty($row->return_type) ? NULL : $return_types[$row->return_type];
    }
    if ($col_reference_id) {
      $data['reference_id'] = empty($row->reference_id) ? NULL : check_plain($row->reference_id);
    }

    if ($col_client_show) {
      $data['client'] = l($row->display_name, $this_ttd['ctd']['base_path'] . '/' . $row->client_id . '/view');
    }
    if ($col_product_show) {
      $data['product'] = l($row->product_title, 'av/products/' . $row->item_id . '/view');
      if ($is_sr) {
        $data['actions'] = avtxns_txn_list_table_pr_link($row->product_title, $user_filter, AVTXNS_TXN_TYPE_PURCHASE_RETURN);
        $data['actions'] .= ' &nbsp; ' . avtxns_txn_list_table_phl_link($row->product_title, $user_filter, 'hist.');
        $data['actions'] = '<div class="not-printable">' . $data['actions'] . '</div>';
      }
      $data['vendor_name'] = check_plain($row->vendor_name);
    }
    if ($col_product) {
      $data['row_qty'] = number_format($row->row_qty);
      $row_qty_total += $row->row_qty;
    }
    if ($is_sr) {
      $data['row_qty_good'] = number_format($row->row_qty_good);
      $data['row_qty_bad'] = number_format($row->row_qty_bad);
      $row_qty_good_total += $row->row_qty_good;
      $row_qty_bad_total += $row->row_qty_bad;
      //$data['row_qty_prd'] = number_format($row->row_qty_prd);
    }

    if ($col_uom) {
      $data['uom'] = check_plain($row->base_uom_title);
    }

    //$data['term'] = check_plain($row->term_name);
    //$data['onhand'] = number_format($row->onhand) . ' ' . check_plain($row->base_uom_title);

    //if ($col_agent_show) {
    //  $data['agent'] = check_plain($row->agent_name);
    //}
    $table_rows[] = $data;

    // if ($transaction_type == AVTXNS_TXN_TYPE_JOURNAL && $this_ttype != AVTXNS_TXN_TYPE_JOURNAL) {
    //   // Repeat data to balance the debit/credit amounts for non-journal transactions.
    //   $default_account = $this_ttd['default_account_id'];
    //   $account_id = key($default_account);
    //   $data['account_title'] = check_plain($default_account_details[$account_id]->title);
    //
    //   if (!empty($user_filter['avachart.title']) && $user_filter['avachart.title'] != $data['account_title']) {
    //     continue;
    //   }
    //
    //   // Switch debit/credit amounts.
    //   $amt_type_of_default_account = $default_account[$account_id];
    //   $data[$amt_type_of_default_account] =  $data[$amt_type];
    //   $data[$amt_type] = NULL;
    //
    //   // Update debit/credit total.
    //   $amt_type_of_default_account_total = "{$amt_type_of_default_account}_total";
    //   $$amt_type_of_default_account_total += $row->$amt_type;
    //
    //   // Do duplicate row.
    //   $table_rows[] = $data;
    // }
  }

  if (!empty($table_rows) && $limit === NULL) {
    $colspan = 0;
    $totals_table_row = array();
    if ($col_product) {
      $label_colspan = $colspan = array_search('row_qty', array_keys($header));
      $totals_table_row[] = array(
        'data' => '<div class="uk-text-right">Total</div>',
        'colspan' => $label_colspan
      );
      $totals_table_row[] = number_format($row_qty_total);
      if ($is_sr) {
        $totals_table_row[] = number_format($row_qty_good_total);
        $totals_table_row[] = number_format($row_qty_bad_total);
      }
    }
    elseif ($col_accounts) {
      $label_colspan = $colspan = array_search('debit', array_keys($header));
      $totals_table_row[] = array(
        'data' => NULL,
        'colspan' => $label_colspan
      );
      $totals_table_row[] = '<div class="uk-text-right">' . number_format($debit_total, 2) . '</div>';
      $totals_table_row[] = '<div class="uk-text-right">' . number_format($credit_total, 2) . '</div>';
    }

    if (!empty($colspan)) {
      $remaining_cols = count($header) - ($colspan + (count($totals_table_row) - 1));
      if ($remaining_cols) {
        $totals_table_row[] = array('data' => NULL, 'colspan' => $remaining_cols);
      }

      if (!empty($totals_table_row)) {
        $table_rows[] = array(array('data' => NULL, 'colspan' => count($header)));
        $table_rows[] = $totals_table_row;
      }
    }
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
    $transaction_type_name = $transaction_type == AVTXNS_TXN_TYPE_JOURNAL ? 'ACCOUNT ENTRIES' : $ttd['plural_name'];
    $transaction_list_table['simplified_status'] = array(
      '#type' => 'item',
      '#markup' => check_plain($filter_form_settings['simplified_status']['#options'][$user_filter['simplified_status']]) . ' ' . $transaction_type_name,
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
  if (!empty($user_filter['avachart.title'])) {
    $transaction_list_table['principal_name'] = array(
      '#title' => 'Account name',
      '#title_display' => 'inline-before',
      '#type' => 'item',
      '#markup' => empty($user_filter['avachart.title']) ? '' : check_plain($user_filter['avachart.title']),
    );
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