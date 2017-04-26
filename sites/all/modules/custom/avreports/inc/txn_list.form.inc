<?php

/**
 * Transaction List report.
 * @param $form
 * @param $form_state
 * @param $transaction_type
 *
 * @return array
 * @throws Exception
 */
function avreports_txn_list_form($form, &$form_state, $transaction_type) {
  global $user;

  //$x = avbase_get_categories('agent1');
  //$x = avbase_get_categories('uom');
  //dpm(implode('|', array_keys($x)));
  //dpm(time());
  ////dpm(strtotime(""));

  // Transaction type details.
  $ttd = avtxns_txn_types($transaction_type);

  // Pending ref types.
  $pending_txns_reference_type = empty($ttd['pending_txns_reference_type']) ? array() : (array) $ttd['pending_txns_reference_type'];

  // Extra columns
  $col_client = !empty($ttd['ctd']);
  $col_area = $col_client && empty($ttd['ctd']['no_area']);
  $col_due_date = !empty($ttd['bill']);
  $col_paid = !empty($ttd['payable']);
  $col_terms = empty($ttd['no_terms']);
  $col_agent = $col_client && empty($ttd['no_agent']);
  $col_memo = $transaction_type == AVTXNS_TXN_TYPE_STOCK_ADJUSTMENT || $ttd['transaction'] == 'accounts';
  //$col_account_name = $ttd['transaction'] == 'accounts';
  $col_total = $transaction_type != AVTXNS_TXN_TYPE_JOURNAL;
  //if (!isset($row->ref_txn_id) && !empty($item->ref_txn_type) && count($pending_txns_reference_type) == 1 && in_array($item->ref_txn_type, $pending_txns_reference_type)) {
  $col_items_ref_txn_id = count($pending_txns_reference_type) == 1;

  // Custom date optionsdetails.
  $dad = avtxns_txn_list_filter_date_auto_options();
  $dad_default_key = empty($ttd['payable']) ? 'today' : 'this_week';
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
      '#title' => $ttd['name'],
      '#options' => array(
        AVTXNS_TXN_STATUS_LOCKED . '-' . AVTXNS_TXN_STATUS_OPEN => 'Open',
      ),
      '#default_value' => AVTXNS_TXN_STATUS_LOCKED . '-' . AVTXNS_TXN_STATUS_OPEN,
    ),
    'reset_button' => array('#attributes' => array('style' => 'margin-top: 25px;')),
  );
  if (in_array($transaction_type, avtxns_get_refable_txn_types()) || in_array($transaction_type, avtxns_get_creditable_txn_types())) {
    $filter_form_settings['simplified_status']['#options'][AVTXNS_TXN_STATUS_CLOSED] = 'Closed';
    $filter_form_settings['simplified_status']['#options'][AVTXNS_TXN_STATUS_LOCKED . '-' . AVTXNS_TXN_STATUS_OPEN . '-' . AVTXNS_TXN_STATUS_CLOSED] = 'Open and Closed';
  }
  if (!empty($ttd['payment']) || $transaction_type == AVTXNS_TXN_TYPE_DELIVERY) {
    $filter_form_settings['simplified_status']['#options'][AVTXNS_TXN_STATUS_PENDING] = 'Pending';
  }
  if (in_array($transaction_type, avtxns_get_creditable_txn_types())) {
    //$filter_form_settings['simplified_status']['#options'][AVTXNS_TXN_STATUS_LOCKED . '-' . AVTXNS_TXN_STATUS_OPEN] = 'With available credits';
  }
  //elseif (in_array($transaction_type, array(AVTXNS_TXN_TYPE_SALES_RETURN, AVTXNS_TXN_TYPE_PURCHASE_RETURN))) {
  //  unset($filter_form_settings['simplified_status']['#options'][AVTXNS_TXN_STATUS_CLOSED]);
  //}
  $filter_form_settings['simplified_status']['#options'][AVTXNS_TXN_STATUS_VOID] = 'Void';
  if ($col_due_date) {
    $filter_form_settings['overdue'] = array('#title' => 'Due status');
  }
  if ($col_client) {
    $filter_form_settings['avclients.display_name'] = array(
      '#title' => $ttd['ctd']['name'],
      '#autocomplete_path' => 'av/clients/autocomplete/' . $ttd['ctd']['type'] . '/' . implode('-', array(AVCLIENT_STATUS_ACTIVE, AVCLIENT_STATUS_SUSPENDED)),
    );
  }
  if ($col_area) {
    $filter_form_settings['avcat_area.title'] = array('#title' => 'Area');
  }
  if ($col_agent) {
    $filter_form_settings['avcat_agent.title'] = array(
      '#title' => 'Agent',
      '#autocomplete_path' => 'av/categories/agent' . $ttd['ctd']['type'] . '/autocomplete',
    );
  }
  $form['transaction_list_filter'] = avtxns_txn_list_filter_form($form, $form_state, $filter_form_settings);

  $user_filter = empty($form_state['values']['avtxns_tl_filter']) ? array() : $form_state['values']['avtxns_tl_filter'];
  $filter_query = empty($user_filter['filter_query']) ? array() : $user_filter['filter_query'];
  $where = empty($filter_query['where']) ? array() : $filter_query['where'];
  $args = empty($filter_query['args']) ? array() : $filter_query['args'];
  //$agent_name = empty($user_filter['avcat_agent.title']) ? '' : $user_filter['avcat_agent.title'];


  // Show hide columns depending on selected filters.
  $col_client_show = $col_client && empty($user_filter['avclients.display_name']);
  $col_agent_show = $col_agent && empty($user_filter['avcat_agent.title']);
  //if (!empty($agent_name)) {
  //  unset($header['agent']);
  //}



  // Headers.
  $header = array();
  if ($col_client_show) {
    $header['client'] = array('data' => $ttd['ctd']['name'], 'field' => 'display_name');
  }
  $header['date'] = array('data' => 'Date', 'field' => 'transaction_date');
  $header['id'] = array('data' => strtoupper($transaction_type) . '#', 'field' => 'avtxn.id');
  if ($col_items_ref_txn_id) {
    $header['items_ref_txn_id'] = array('data' => 'Ref. Txn.#', 'field' => 'avtxn_detail.ref_txn_id');
  }
  if ($col_terms) {
    $header['term'] = array('data' => 'Terms', 'field' => 'term_name');
  }
  if ($col_due_date) {
    $header['due_date'] = array('data' => 'Due Date', 'field' => 'due_date');
  }
  if ($col_memo) {
    $header['memo'] = array('data' => 'Note');
  }
  if ($col_total) {
    $header['total'] = array('data' => 'Total', 'class' => array('uk-text-right'));
  }
  if ($col_paid) {
    $header['paid'] = array('data' => 'Paid', 'class' => array('uk-text-right'));
    $header['balance'] = array('data' => 'Balance', 'class' => array('uk-text-right'));
  }
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

  if (empty($_GET['order'])) {
    // Set default order.
    $_GET['order'] = $header['id']['data'];
    $_GET['sort'] = 'desc';
    //if (isset($header['client'])) {
    //  $_GET['order'] = $header['client']['data'];
    //  $_GET['sort'] = 'asc';
    //}
  }
  $order_by = avbase_build_header_order_array($header);
  if ($col_due_date && isset($order_by['display_name'])) {
    $order_by['avtxn.due_date'] = 'due_date ASC';
    $order_by['avtxn.transaction_date'] = 'transaction_date ASC';
    $order_by['avtxn.id'] = 'id DESC';
  }


  $where[] = "avtxn.transaction_type = '" . $transaction_type . "'";
  //dpm($where);
  //$where[] = "avtxn.status NOT IN (" . AVTXNS_TXN_STATUS_VOID . ", " . AVTXNS_TXN_STATUS_PENDING . ")";




  $select = array();
  $select[] = 'avtxn.id';
  $select[] = 'avtxn.status';
  $select[] = 'avtxn.transaction_type';
  $select[] = 'avtxn.transaction_date';
  if ($col_due_date) {
    $select[] = 'avtxn.due_date';
    $select[] = avtxns_txn_overdue_query() . ' AS overdue';
  }
  $select[] = 'avtxn_detail.ref_txn_type';
  if ($col_items_ref_txn_id) {
    $select[] = 'avtxn_detail.ref_txn_id';
  }
  $select[] = avtxns_txn_detail_sum_query_discount_applied() . " AS grand_total";
  if ($col_paid) {
    $select[] = "(
      IF (
      avtxn.status = " . AVTXNS_TXN_STATUS_VOID . ",
      0,
      (SELECT SUM(pmt_details.cost) FROM {avtbl_transaction_details} pmt_details
      INNER JOIN {avtbl_transactions} pmt ON pmt.id = pmt_details.transaction_id
      WHERE pmt_details.ref_txn_id = avtxn.id AND pmt.status <> " . AVTXNS_TXN_STATUS_VOID . ")
      )
    ) AS paid_amount";
  }
  if ($col_memo) {
    $select[] = 'avtxn.memo';
  }
  $joins = array();
  $joins[] = 'LEFT JOIN {avtbl_transaction_details} AS avtxn_detail ON avtxn_detail.transaction_id = avtxn.id';
  if ($col_client) {
    $select[] = 'avclients.id AS client_id';
    $select[] = 'avclients.display_name';
    $joins[] = 'LEFT JOIN {avtbl_clients} AS avclients ON avclients.id = avtxn.client_id';
  }
  if ($col_area) {
    $joins[] = 'LEFT JOIN {avtbl_categories} AS avcat_area ON avcat_area.id = avclients.area_id';
  }
  if ($col_agent) {
    $select[] = 'avcat_agent.title AS agent_name';
    $joins[] = 'LEFT JOIN {avtbl_categories} AS avcat_agent ON avcat_agent.id = avtxn.agent_id';
  }
  if ($col_terms) {
    $select[] = 'avcat_term.title AS term_name';
    $joins[] = 'LEFT JOIN {avtbl_categories} AS avcat_term ON avcat_term.id = avtxn.term_id';
  }
  $group_by = array('avtxn.id', 'avtxn_detail.ref_txn_type');

  $query = "SELECT " . implode(', ', $select) . " FROM {avtbl_transactions} AS avtxn " . implode(' ', $joins);
  avbase_query_supplements($query, $where, $order_by, NULL, NULL, $group_by);
  $rs = db_query($query, $args);
  //$transactions = avtxns_txns_query_load($where, $args, NULL, NULL, $order_by, $select, $joins, $group_by);
  $list_total = $paid_total = $balance_total = 0;
  $row_totals = array();
  $table_rows = array();
  //$now = time();
  $last_client_name = '';
  foreach ($rs as $row) {
    //$overdue = $col_due_date && $row->due_date <= $now && $row->status != AVTXNS_TXN_STATUS_CLOSED;
    //$overdue = $overdue && $row->status != AVTXNS_TXN_STATUS_VOID;
    $overdue = empty($row->overdue) ? FALSE : $row->overdue;

    $data = array();
    $this_ttd = avtxns_txn_types($row->transaction_type);
    $view_path = "{$this_ttd['base_path']}/$row->id/view";
    if (!empty($this_ttd['credit_txn_types'])) {
      $row_total = empty($row_totals[$row->id]) ? 0 : $row_totals[$row->id];
      if (in_array($row->ref_txn_type, $this_ttd['credit_txn_types'])) {
        $row_total -= $row->grand_total;
        $list_total -= $row->grand_total;
        $balance_total -= $row->grand_total;
      }
      else {
        $row_total += $row->grand_total;
        $list_total += $row->grand_total;
        $balance_total += $row->grand_total - $row->paid_amount;
      }
    }
    else {
      $row_total = $row->grand_total;
      $list_total += $row->grand_total;
    }
    $row_totals[$row->id] = $row_total;
    $row_total_html = l(number_format($row_total, 2), $view_path);
    $row_bal_html = '';
    if ($col_paid) {
      $row_bal_html = number_format(round($row_total - $row->paid_amount, 2), 2);
    }

    if ($overdue) {
      $row_bal_html .= ' <em><i class="uk-icon-exclamation-circle" title="Overdue"></i></em>';
    }

    if (!isset($table_rows[$row->id])) {
      if ($col_client_show) {
        $data['client'] = array('data' => l($row->display_name, $ttd['ctd']['base_path'] . '/' . $row->client_id . '/view'));
      }
      $data['date'] = array('data' => l(format_date($row->transaction_date, 'custom', 'M-d-Y'), $view_path));
      $data['id'] = array('data' => l(avbase_txn_number_format($row->transaction_type, $row->id), $view_path));
      //if (!isset($row->ref_txn_id) && !empty($item->ref_txn_type) && count($pending_txns_reference_type) == 1 && in_array($item->ref_txn_type, $pending_txns_reference_type)) {
      if ($col_items_ref_txn_id) {
        $data['items_ref_txn_id'] = array();
      }
      if ($col_terms) {
        $data['term'] = array('data' => check_plain($row->term_name));
      }
      if ($col_due_date) {
        $data['due_date'] = array('data' => format_date($row->due_date, 'custom', 'Y-M-d'));
      }
      if ($col_memo) {
        $data['memo'] = array('data' => check_plain($row->memo));
      }
      if ($col_total) {
        $data['total'] = array('data' => $row_total_html, 'class' => array('uk-text-right'));
      }

      if ($col_paid) {
        $paid_amount = empty($row->paid_amount) ? '' : number_format($row->paid_amount, 2);
        $data['paid'] = array('data' => $paid_amount, 'class' => array('uk-text-right'));
        $data['balance'] = array('data' => $row_bal_html, 'class' => array('uk-text-right'));
        $paid_total += $row->paid_amount;
      }
      if ($col_agent_show) {
        $data['agent'] = array('data' => check_plain($row->agent_name));
      }
      $table_rows[$row->id] = array('data' => $data);

      if ($overdue) {
        $table_rows[$row->id]['class'] = array('uk-alert-danger');
        if (isset($data['client'])) {
          $table_rows[$row->id]['data']['due_date']['data'] = '<em><i class="uk-icon-exclamation-circle" title="Overdue"></i></em> ' . $table_rows[$row->id]['data']['due_date']['data'];
        }
      }
      elseif ($row->status == AVTXNS_TXN_STATUS_VOID) {
        $table_rows[$row->id]['class'] = array('uk-row-mute');
      }
      //elseif ($row->status == AVTXNS_TXN_STATUS_PENDING) {
      //  $table_rows[$row->id]['class'] = array('uk-alert-warning');
      //}

      //$sorted_by = empty($_GET['order']) ? '' : $_GET['order'];
      //if ($col_client_show) {
      //  if ($sorted_by == $header['client']['data'] && $last_client_name == $row->display_name) {
      //    $table_rows[$row->id]['data']['client']['data'] = '';
      //  }
      //  $last_client_name = $row->display_name;
      //}

    }
    else {
      $table_rows[$row->id]['data']['total']['data'] =  $row_total_html;
      $table_rows[$row->id]['data']['balance']['data'] =  $row_bal_html;
    }

    if ($col_items_ref_txn_id && !empty($row->ref_txn_id) && empty($table_rows[$row->id]['data']['items_ref_txn_id']) && in_array($row->ref_txn_type, $pending_txns_reference_type)) {
      $irtd = avtxns_txn_types($row->ref_txn_type);
      $irti_view_path = "{$irtd['base_path']}/{$row->ref_txn_id}/view";
      $table_rows[$row->id]['data']['items_ref_txn_id'] = array('data' => l(avbase_txn_number_format($row->ref_txn_type, $row->ref_txn_id), $irti_view_path));
    }
  }


  if (!empty($table_rows) && $col_total) {
    //foreach ($row_totals as $rt) {
    //  $list_total += $rt;
    //}
    // Add summary rows at the bottom of the table.
    $table_rows[] = array('data' => array(array('data' => '', 'colspan' => 10)));
    $colspan = array_search('total', array_keys($header)) + 1;
    $totals_table_row = array();
    $totals_table_row[] = array('data' => number_format($list_total, 2), 'class' => array('uk-text-right'), 'colspan' => $colspan);
    if ($col_paid) {
      $totals_table_row[] = array('data' => number_format($paid_total, 2), 'class' => array('uk-text-right'));
      $totals_table_row[] = array('data' => number_format($balance_total, 2), 'class' => array('uk-text-right'));
    }

    if ($colspan + (count($totals_table_row) - 1) < count($header)) {
      $totals_table_row[] = array('data' => '', 'colspan' => 10);
    }
    $table_rows[] = array('data' => $totals_table_row, 'class' => array('suk-text-bold'), 'style' => 'border-top: solid 1px #ddd;');
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

  if (1) {
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

    avbase_user_extra_details($user);
    $transaction_list_table['prepared_by'] = array(
      '#title' => 'Prepared by',
      '#title_display' => 'inline-before',
      '#type' => 'item',
      '#markup' => check_plain($user->avdata->full_name),
    );
    $transaction_list_table['delivered_by'] = array(
      '#title' => 'Received by',
      '#title_display' => 'inline-before',
      '#type' => 'item',
      '#markup' => '_______________________________',
      //'#access' => $transaction_type == AVTXNS_TXN_TYPE_DELIVERY,
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

  //if (empty($rows)) {
  //  $table_markup = '<div class="uk-margin-top uk-text-muted">No match found.</div>';
  //}
  //else {
  //  $table_attributes = array(
  //    'id' => 'transaction-list-table',
  //    'class' => array('uk-table-condensed uk-table-striped'),
  //  );
  //  // Add summary rows at the bottom of the table.
  //  $rows[] = array('data' => array(array('data' => '&nbsp;', 'colspan' => 10)));
  //  $rows[] = array(
  //    'data' => array(
  //      array('data' => number_format($total_receivables, 2), 'class' => array('uk-text-right'), 'colspan' => 3),
  //      array('data' => '', 'colspan' => 2),
  //    ),
  //    'class' => array('uk-table-middle'),
  //  );
  //  $rows[] = array('data' => array(array('data' => '- End of list -', 'colspan' => 10, 'class' => array('uk-text-muted uk-text-center'))));
  //  $table_markup = theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => $table_attributes));
  //}
  //$form['table']['#markup'] = $table_markup;

  //$form['transaction_list_table'] = array(
  //  '#type' => 'container',
  //  '#theme' => 'avreports_txn_list',
  //  '#attributes' => array(
  //    'id' => 'transaction-list-table-wrapper',
  //    'class' => array('printable')
  //  ),
  //);
  //$transaction_list_table['table']['#markup'] = '<!--Do not remove me-->' . $table_markup;
  //if (!empty($rows)) {
  //  if (!empty($user_filter['date_from'])) {
  //    $date_text = empty($user_filter['date_from']) ? '' : check_plain($user_filter['date_from']);
  //    if (!empty($user_filter['date_to']) && $user_filter['date_from'] != $user_filter['date_to']) {
  //      $date_text .= ' - ' . $user_filter['date_to'];
  //    }
  //    $transaction_list_table['date_from'] = array(
  //      '#type' => 'item',
  //      '#markup' => $date_text,
  //    );
  //  }
  //  //$transaction_list_table['agent'] = array(
  //  //  '#title' => 'Agent',
  //  //  '#title_display' => 'inline-before',
  //  //  '#type' => 'item',
  //  //  '#markup' => check_plain($agent_name),
  //  //  '#access' => !empty($agent_name),
  //  //);
  //  avbase_user_extra_details($user);
  //  $transaction_list_table['prepared_by'] = array(
  //    '#title' => 'Prepared by',
  //    '#title_display' => 'inline-before',
  //    '#type' => 'item',
  //    '#markup' => check_plain($user->avdata->full_name),
  //  );
  //  $transaction_list_table['delivered_by'] = array(
  //    '#title' => 'Delivered by',
  //    '#title_display' => 'inline-before',
  //    '#type' => 'item',
  //    '#markup' => '_______________________________',
  //    //'#access' => $transaction_type == AVTXNS_TXN_TYPE_DELIVERY,
  //  );
  //}
  //
  //$form['transaction_list_table'] += $transaction_list_table;
  //$delivery_report

  //$form['delivery_list_printable'] += $delivery_report;


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