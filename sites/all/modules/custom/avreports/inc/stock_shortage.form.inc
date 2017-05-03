<?php

function avreports_stock_shortage_form($form, &$form_state) {
  // Transaction type details.
  $ttd = avtxns_txn_types(AVTXNS_TXN_TYPE_DELIVERY);

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
      '#title' => 'Deliveries',
      '#options' => array(
        AVTXNS_TXN_STATUS_LOCKED . '-' . AVTXNS_TXN_STATUS_OPEN => 'Open',
      ),
      '#default_value' => AVTXNS_TXN_STATUS_LOCKED . '-' . AVTXNS_TXN_STATUS_OPEN . '-' . AVTXNS_TXN_STATUS_CLOSED. '-' . AVTXNS_TXN_STATUS_PENDING,
    ),
    'avprod.title' => array(
      '#title' => t('Product'),
    ),
    'avcat_principal.title' => array(
      '#title' => t('Principal'),
      '#autocomplete_path' => 'av/categories/principal/autocomplete',
    ),
    'reset_button' => array('#attributes' => array('style' => 'margin-top: 25px;')),
  );
  $filter_form_settings['simplified_status']['#options'][AVTXNS_TXN_STATUS_CLOSED] = 'Closed';
  $filter_form_settings['simplified_status']['#options'][AVTXNS_TXN_STATUS_PENDING] = 'Pending';
  $filter_form_settings['simplified_status']['#options'][AVTXNS_TXN_STATUS_LOCKED . '-' . AVTXNS_TXN_STATUS_OPEN . '-' . AVTXNS_TXN_STATUS_CLOSED . '-' . AVTXNS_TXN_STATUS_PENDING] = 'All';
  //$filter_form_settings['simplified_status']['#options'][AVTXNS_TXN_STATUS_VOID] = 'Void';
  $filter_form_settings['avclients.display_name'] = array(
    '#title' => 'Customer',
    '#autocomplete_path' => 'av/clients/autocomplete/' . AVCLIENT_TYPE_CUSTOMER . '/' . implode('-', array(AVCLIENT_STATUS_ACTIVE, AVCLIENT_STATUS_SUSPENDED)),
  );
  $filter_form_settings['avcat_area.title'] = array('#title' => 'Area');
  $filter_form_settings['avcat_agent.title'] = array(
    '#title' => 'Agent',
    '#autocomplete_path' => 'av/categories/agent' .AVCLIENT_TYPE_CUSTOMER . '/autocomplete',
  );
  $filter_form_settings['avclients.category_id'] = array('#title' => 'Channel', '#options' => avbase_get_category_options_array('category' . $ttd['ctd']['type']));
  $filter_form_settings['avcat_prodcat.id'] = array('#options' => avbase_get_category_options_array('product_cat'));
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
  $header['date'] = array('data' => 'Date', 'field' => 'transaction_date');
  $header['id'] = array('data' => 'DR#', 'field' => 'avtxn.id');
  //$header['items_ref_txn_id'] = array('data' => 'Ref. Txn.#', 'field' => 'avtxn_detail.ref_txn_id');
  if ($col_client_show) {
    $header['client'] = array('data' => 'Customer', 'field' => 'display_name');
  }
  if ($col_product_show) {
    $header['product'] = array('data' => 'Product', 'field' => 'avprod.title');
  }
  $header['uom'] = array('data' => 'QTY', 'field' => 'avcat_uom.title');
  //$header['term'] = array('data' => 'Terms', 'field' => 'term_name');
  $header['onhand'] = array('data' => 'ON HAND', 'field' => 'onhand');
  if ($col_agent_show) {
    $header['agent'] = array('data' => 'Agent', 'field' => 'agent_name');
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
    $_GET['order'] = $header['onhand']['data'];
    $_GET['sort'] = 'ASC';
  }
  $order_by = avbase_build_header_order_array($header);
  if (isset($order_by['onhand'])) {
    //$order_by['avtxn.transaction_date'] = 'transaction_date DESC';
    $order_by['avtxn.id'] = 'avtxn.id DESC';
  }

  $select = array();
  $joins = array();
  $group_by = array();
  $select[] = 'avtxn.id';
  $select[] = 'avtxn.status';
  $select[] = 'avtxn.transaction_date';
  $select[] = 'avtxn_detail.item_id';
  //$select[] = 'avtxn_detail.ref_txn_type';
  //$select[] = 'avtxn_detail.ref_txn_id';
  $joins[] = 'INNER JOIN {avtbl_transaction_details} AS avtxn_detail ON avtxn_detail.transaction_id = avtxn.id';
  $select[] = 'avprod.title AS product_title';
  $select[] = 'avprod.qty AS onhand';
  $joins[] = 'INNER JOIN {avtbl_products} AS avprod ON avprod.id = avtxn_detail.item_id AND avtxn_detail.qty < 1';
  $select[] = 'avclients.id AS client_id';
  $select[] = 'avclients.display_name';
  $joins[] = 'INNER JOIN {avtbl_clients} AS avclients ON avclients.id = avtxn.client_id';
  $select[] = 'avcat_uom.title AS uom_title';
  $joins[] = 'LEFT JOIN {avtbl_categories} AS avcat_uom ON avcat_uom.id = avtxn_detail.uom_id';
  $select[] = 'avcat_buom.title AS base_uom_title';
  $joins[] = 'LEFT JOIN {avtbl_categories} AS avcat_buom ON avcat_buom.id = avprod.uom_id';
  $joins[] = 'LEFT JOIN {avtbl_categories} AS avcat_area ON avcat_area.id = avclients.area_id';
  $select[] = 'avcat_agent.title AS agent_name';
  $joins[] = 'LEFT JOIN {avtbl_categories} AS avcat_agent ON avcat_agent.id = avtxn.agent_id';
  //$select[] = 'avcat_term.title AS term_name';
  //$joins[] = 'LEFT JOIN {avtbl_categories} AS avcat_term ON avcat_term.id = avtxn.term_id';
  $joins[] = 'LEFT JOIN {avtbl_categories} AS avcat_prodcat ON avcat_prodcat.id = avprod.category_id';
  $joins[] = 'LEFT JOIN {avtbl_categories} AS avcat_principal ON avcat_principal.id = avprod.principal_id';

  $query = "SELECT " . implode(', ', $select) . " FROM {avtbl_transactions} AS avtxn " . implode(' ', $joins);
  $where[] = 'avtxn.transaction_type = :delivery_type';
  $args[':delivery_type'] = AVTXNS_TXN_TYPE_DELIVERY;

  avbase_query_supplements($query, $where, $order_by, NULL, NULL, $group_by);
  $rs = db_query($query, $args);
  $table_rows = array();
  $irtd = avtxns_txn_types(AVTXNS_TXN_TYPE_SALES_ORDER);
  foreach ($rs as $row) {
    $data = array();
    $data['date'] = array('data' => format_date($row->transaction_date, 'custom', 'M-d-Y'));
    $data['id'] = array('data' => l(avbase_txn_number_format(AVTXNS_TXN_TYPE_DELIVERY, $row->id), "{$ttd['base_path']}/$row->id/view"));
    //$irti_view_path = "{$irtd['base_path']}/{$row->ref_txn_id}/view";
    //$data['items_ref_txn_id'] = array('data' => empty($row->ref_txn_id) ? NULL : l(avbase_txn_number_format($row->ref_txn_type, $row->ref_txn_id), $irti_view_path));

    if ($col_client_show) {
      $data['client'] = array('data' => l($row->display_name, $ttd['ctd']['base_path'] . '/' . $row->client_id . '/view'));
    }
    if ($col_product_show) {
      $data['product'] = array('data' => l($row->product_title, 'av/products/' . $row->item_id . '/view'));
    }
    $data['uom'] = array('data' => empty($row->uom_title) ? NULL : '0 ' . check_plain($row->uom_title));

    //$data['term'] = array('data' => check_plain($row->term_name));
    $data['onhand'] = array('data' => number_format($row->onhand) . ' ' . check_plain($row->base_uom_title));

    if ($col_agent_show) {
      $data['agent'] = array('data' => check_plain($row->agent_name));
    }
    $table_rows[] = array('data' => $data);
  }

  // Build table.
  $transaction_list_table['table'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $table_rows,
    '#empty' => '<div class="uk-text-muted">' . t('No match found.') . '</div>',
    '#attributes' => array(
      'id' => 'transaction-list-table',
      'class' => array('uk-table-condensed uk-table-hover uk-text-small uk-text-uppercase'),
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
  return $form;
}