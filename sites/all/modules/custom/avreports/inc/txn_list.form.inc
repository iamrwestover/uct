<?php

function avreports_client_balances_form($form, &$form_state, $client_type) {
  if ($client_type == AVCLIENT_TYPE_CUSTOMER) {
    $form = avreports_txn_list_form($form, $form_state, AVTXNS_TXN_TYPE_INVOICE, AVTXNS_TXN_TYPE_CUSTOMER_CHARGES);
  }
  elseif ($client_type == AVCLIENT_TYPE_VENDOR) {
    $form = avreports_txn_list_form($form, $form_state, AVTXNS_TXN_TYPE_BILL, AVTXNS_TXN_TYPE_OTHER_BILLS);
  }
  return $form;
}

/**
 * Transaction List report.
 * @param $form
 * @param $form_state
 * @param $transaction_type
 * @param $secondary_transaction_type
 *
 * @return array
 * @throws Exception
 */
function avreports_txn_list_form($form, &$form_state, $transaction_type, $secondary_transaction_type = NULL, $limit = NULL, $offset = NULL) {
  ini_set('memory_limit', AVBASE_EXPANDED_MEMORY_LIMIT);


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
  $col_due_date = !empty($ttd['payable']);
  $col_paid = !empty($ttd['payable']);
  $col_terms = empty($ttd['no_terms']);
  $col_agent = $col_client && empty($ttd['no_agent']);
  $col_memo = $transaction_type == AVTXNS_TXN_TYPE_STOCK_ADJUSTMENT || $ttd['transaction'] == 'accounts';
  //$col_account_name = $ttd['transaction'] == 'accounts';
  $col_total = $transaction_type != AVTXNS_TXN_TYPE_JOURNAL;
  //if (!isset($row->ref_txn_id) && !empty($item->ref_txn_type) && count($pending_txns_reference_type) == 1 && in_array($item->ref_txn_type, $pending_txns_reference_type)) {
  $col_items_ref_txn_id = count($pending_txns_reference_type) == 1;
  $col_return_type = $transaction_type == AVTXNS_TXN_TYPE_SALES_RETURN;
  $col_reference_id = in_array($transaction_type, avtxns_get_types_with_reference_id()) || in_array($secondary_transaction_type, avtxns_get_types_with_reference_id());
  $col_bal_age = !empty($secondary_transaction_type);

  // Custom date options details.
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
      '#title' => 'Status',
      '#options' => array(
        AVTXNS_TXN_STATUS_LOCKED . '-' . AVTXNS_TXN_STATUS_OPEN => 'Open',
      ),
      '#default_value' => AVTXNS_TXN_STATUS_LOCKED . '-' . AVTXNS_TXN_STATUS_OPEN,
    ),
    'reset_button' => array('#attributes' => array('style' => 'margin-top: 25px;')),
    'export_button' => array(
      '#func_name' => __FUNCTION__ ,
      '#func_args' => array($transaction_type, $secondary_transaction_type),
      '#report_name' => $ttd['name'] . ' List',
      '#report_inc' => 'txn_list.form',
    ),
  );

  // Override report name.
  if ($transaction_type == AVTXNS_TXN_TYPE_INVOICE && $secondary_transaction_type == AVTXNS_TXN_TYPE_CUSTOMER_CHARGES) {
    $filter_form_settings['export_button']['#report_name'] = 'Customer Balances';
  }
  elseif ($transaction_type == AVTXNS_TXN_TYPE_BILL && $secondary_transaction_type == AVTXNS_TXN_TYPE_OTHER_BILLS) {
    $filter_form_settings['export_button']['#report_name'] = 'Vendor Balances';
  }
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
  if ($col_return_type) {
    $filter_form_settings['return_type'] = array('#title' => 'Return type');
  }
  if ($col_reference_id) {
    $filter_form_settings['avtxn.reference_id'] = array('#title' => 'Reference ID');
  }
  if (!empty($ttd['ctd']) && empty($ttd['ctd']['no_category'])) {
    $filter_form_settings['avclients.category_id'] = array('#title' => 'Category', '#options' => avbase_get_category_options_array('category' . $ttd['ctd']['type']));
  }
  if ($col_paid) {
    $filter_form_settings['payment_date_to'] = array('#title' => 'Payments until', '#default_value' => $dad['today']['date_to']);
  }
  $form['transaction_list_filter'] = avtxns_txn_list_filter_form($form, $form_state, $filter_form_settings);

  $user_filter = empty($form_state['values']['avtxns_tl_filter']) ? array() : $form_state['values']['avtxns_tl_filter'];
  $filter_query = empty($user_filter['filter_query']) ? array() : $user_filter['filter_query'];
  $where = empty($filter_query['where']) ? array() : $filter_query['where'];
  $args = empty($filter_query['args']) ? array() : $filter_query['args'];
  //$agent_name = empty($user_filter['avcat_agent.title']) ? '' : $user_filter['avcat_agent.title'];


  // Show hide columns depending on selected filters.
  $col_client_show = $col_client && empty($user_filter['avclients.display_name']);
  $col_agent_show = $col_agent && empty($user_filter['avcat_agent.title']) && empty($secondary_transaction_type);
  //if (!empty($agent_name)) {
  //  unset($header['agent']);
  //}



  // Headers.
  $header = array();
  if ($col_client_show) {
    $header['client'] = array('data' => $ttd['ctd']['name'], 'field' => 'display_name');
  }
  $header['date'] = array('data' => 'DATE', 'field' => 'transaction_date');
  $header['id'] = array('data' => 'TXN#', 'field' => 'avtxn.id');
  if ($col_items_ref_txn_id) {
    $header['items_ref_txn_id'] = array('data' => 'REF. TXN.#', 'field' => 'avtxn_detail.ref_txn_id');
  }
  if ($col_return_type) {
    $header['return_type'] = array('data' => 'RER. TYPE', 'field' => 'avtxn.return_type');
  }
  if ($col_reference_id) {
    $header['reference_id'] = array('data' => 'REF. ID');
    if (empty($secondary_transaction_type)) {
      $header['reference_id']['field'] = 'avtxn.reference_id';
    }
  }
  if ($col_terms) {
    $header['term'] = array('data' => 'TERMS', 'field' => 'term_name');
  }
  if ($col_due_date) {
    $header['due_date'] = array('data' => 'DUE DATE', 'field' => 'due_date');
  }
  if ($col_memo) {
    $header['memo'] = array('data' => 'NOTE');
  }
  if ($col_total) {
    $header['total'] = array('data' => 'TOTAL', 'class' => array('uk-text-right'));
  }
  if ($col_paid) {
    $header['paid'] = array('data' => 'PAID', 'class' => array('uk-text-right'));
    $header['balance'] = array('data' => 'BALANCE', 'class' => array('uk-text-right'));
  }
  if ($col_agent_show) {
    $header['agent'] = array('data' => 'AGENT', 'field' => 'agent_name');
  }
  if ($col_bal_age) {
    $header['bal_age'] = array('data' => 'AGE', 'field' => 'bal_age', 'class' => array('uk-text-right'));
    $header['bal_age']['data-uk-tooltip'] = '{cls: "tt-blue"}';
    $header['bal_age']['title'] = 'Age in Days';
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
    $_GET['sort'] = 'DESC';
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


  if (empty($secondary_transaction_type)) {
    $where[] = "avtxn.transaction_type = '" . $transaction_type . "'";
  }
  else {
    $where[] = "avtxn.transaction_type IN ('$transaction_type', '$secondary_transaction_type')";
  }
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
  if ($col_bal_age) {
    $select[] = '(UNIX_TIMESTAMP() - avtxn.transaction_date) / 86400 AS bal_age';
  }
  $select[] = 'avtxn_detail.ref_txn_type';
  if ($col_items_ref_txn_id) {
    $select[] = 'avtxn_detail.ref_txn_id';
  }
  if ($col_return_type) {
    $select[] = 'avtxn.return_type';
    $return_types = avtxns_get_return_types();
  }
  if ($col_reference_id) {
    $select['reference_id'] = 'avtxn.reference_id';
  }
  $select['grand_total'] = avtxns_txn_detail_sum_query_discount_applied();
  $select['grand_total'] .= empty($ttd['payment']) ? '' : " + IFNULL(avtxn.amount_to_credit, 0)";
  $select['grand_total'] .= " AS grand_total";
  if ($col_paid) {
    $payment_date_to_where = '';
    if (!empty($where['payment_date_to']) && !empty($args[':payment_date_to_stamp'])) {
      $payment_date_to_where = ' AND pmt.transaction_date < ' . $args[':payment_date_to_stamp'];
      unset($where['payment_date_to']);
      unset($args[':payment_date_to_stamp']);
    }
    $select[] = "(
      IF (
      avtxn.status = " . AVTXNS_TXN_STATUS_VOID . ",
      0,
        (
        SELECT SUM(pmt_details.cost) FROM {avtbl_transaction_details} pmt_details
        INNER JOIN {avtbl_transactions} pmt ON pmt.id = pmt_details.transaction_id
        WHERE pmt_details.ref_txn_id = avtxn.id AND pmt.status <> " . AVTXNS_TXN_STATUS_VOID . $payment_date_to_where . "
        )
      )
    ) AS paid_amount";

    //unset($args[':payment_date_to_stamp']);
  }
  if ($col_memo) {
    $select[] = 'avtxn.memo';
  }
  $joins = array();
  $joins[] = 'INNER JOIN {avtbl_transaction_details} AS avtxn_detail ON avtxn_detail.transaction_id = avtxn.id';
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
  if ($col_reference_id && !empty($secondary_transaction_type) && $transaction_type == AVTXNS_TXN_TYPE_BILL) {
    $select['reference_id'] = 'IF(avtxn.transaction_type = \'' . AVTXNS_TXN_TYPE_BILL . '\', avtxn2.reference_id, avtxn.reference_id) AS reference_id';
    $joins[] = 'LEFT JOIN {avtbl_transactions} AS avtxn2 ON avtxn2.id = avtxn_detail.ref_txn_id';
  }
  $group_by = array('avtxn.id', 'avtxn_detail.ref_txn_type');

  $query = "SELECT _expression_ FROM {avtbl_transactions} AS avtxn " . implode(' ', $joins);
  // Count max rows before applying supplements to $query.
  avreports_count_max_rows($form, $query, $select, $where, $group_by, $args, $offset);
  avbase_query_supplements($query, $where, $order_by, $limit, $offset, $group_by);
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
    $row_total_html = '<div class="uk-text-right">' . number_format($row_total, 2) . '</div>';
    $row_bal_html = NULL;
    if ($col_paid) {
      $row_bal = round($row_total - $row->paid_amount, 2);
      $row_bal_html = empty($row_bal) ? NULL : number_format($row_bal, 2);
    }

    if (!empty($row_bal_html) && $overdue) {
      $row_bal_html .= ' <em><i class="uk-icon-exclamation-circle" title="Overdue"></i></em>';
    }

    if (!empty($row_bal_html)) {
      $row_bal_html = '<div class="uk-text-right">' . $row_bal_html . '</div>';
    }

    if (!isset($table_rows[$row->id])) {
      if ($col_client_show) {
        $data['client'] = l($row->display_name, $ttd['ctd']['base_path'] . '/' . $row->client_id . '/view');
      }
      $data['date'] = format_date($row->transaction_date, 'custom', AVREPORTS_LIST_DATE_FORMAT);
      $data['id'] = l(avbase_txn_number_format($row->transaction_type, $row->id), $view_path);
      //if (!isset($row->ref_txn_id) && !empty($item->ref_txn_type) && count($pending_txns_reference_type) == 1 && in_array($item->ref_txn_type, $pending_txns_reference_type)) {
      if ($col_items_ref_txn_id) {
        $data['items_ref_txn_id'] = NULL;
      }
      if ($col_return_type) {
        $data['return_type'] = empty($row->return_type) ? NULL : $return_types[$row->return_type];
      }
      if ($col_reference_id) {
        $data['reference_id'] = empty($row->reference_id) ? NULL : check_plain($row->reference_id);
      }
      if ($col_terms) {
        $data['term'] = empty($row->term_name) ? NULL : check_plain($row->term_name);
      }
      if ($col_due_date) {
        $data['due_date'] = empty($row->due_date) ? NULL : format_date($row->due_date, 'custom', AVREPORTS_LIST_DATE_FORMAT);
      }
      if ($col_memo) {
        $data['memo'] = empty($row->memo) ? NULL : check_plain($row->memo);
      }
      if ($col_total) {
        $data['total'] = $row_total_html;
      }
      if ($col_paid) {
        $paid_amount = empty($row->paid_amount) ? NULL : number_format($row->paid_amount, 2);
        $data['paid'] = empty($paid_amount) ? NULL : '<div class="uk-text-right">' . $paid_amount . '</div>';
        $data['balance'] = $row_bal_html;
        $paid_total += $row->paid_amount;
      }
      if ($col_agent_show) {
        $data['agent'] = check_plain($row->agent_name);
      }
      if ($col_bal_age) {
        $data['bal_age'] = empty($row->bal_age) ? NULL : '<div class="uk-text-right">' . number_format($row->bal_age) . '</div>';
      }


      if ($overdue) {
        //if (isset($data['due_date'])) {
        //  $data['due_date'] = '<em><i class="uk-icon-exclamation-circle" title="Overdue"></i></em> ' . $data['due_date'];
        //}
        $data['#avclass'] = 'uk-alert-danger';
      }
      elseif ($row->status == AVTXNS_TXN_STATUS_VOID) {
        $data['#avclass'] = 'uk-row-mute';
      }

      //$data['#avclass'] = 'uk-alert-danger';
      $table_rows[$row->id] = $data;

      //if ($overdue) {
      //  if (isset($data['client'])) {
      //    $table_rows[$row->id]['due_date'] = '<em><i class="uk-icon-exclamation-circle" title="Overdue"></i></em> ' . $table_rows[$row->id]['due_date'];
      //  }
      //  //$table_rows[$row->id]['class'] = array('uk-alert-danger');
      //}
      //elseif ($row->status == AVTXNS_TXN_STATUS_VOID) {
      //  //$table_rows[$row->id]['class'] = array('uk-row-mute');
      //}
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
      //if ($overdue) {
      //  $row_total_html = empty($row_total_html) ? $row_total_html : '<em>' . $row_total_html . '</em>';
      //  $row_bal_html = empty($row_bal_html) ? $row_bal_html : '<em>' . $row_bal_html . '</em>';
      //}
      //elseif ($row->status == AVTXNS_TXN_STATUS_VOID) {
      //  $row_total_html = empty($row_total_html) ? $row_total_html : '<span class="uk-text-muted uk-text-italic">' . $row_total_html . '</span>';
      //  $row_bal_html = empty($row_bal_html) ? $row_bal_html : '<span class="uk-text-muted uk-text-italic">' . $row_bal_html . '</span>';
      //}
      $table_rows[$row->id]['total'] = $row_total_html;
      $table_rows[$row->id]['balance'] =  $row_bal_html;
    }

    if ($col_items_ref_txn_id && !empty($row->ref_txn_id) && empty($table_rows[$row->id]['items_ref_txn_id']) && in_array($row->ref_txn_type, $pending_txns_reference_type)) {
      $irtd = avtxns_txn_types($row->ref_txn_type);
      $irti_view_path = "{$irtd['base_path']}/{$row->ref_txn_id}/view";
      $table_rows[$row->id]['items_ref_txn_id'] = l(avbase_txn_number_format($row->ref_txn_type, $row->ref_txn_id), $irti_view_path);
    }

    // Color them red and gray.
    //if ($overdue) {
    //  foreach ($table_rows[$row->id] as $d_k => $d) {
    //    if (!empty($table_rows[$row->id][$d_k])) {
    //      $table_rows[$row->id][$d_k] = '<em>' . $table_rows[$row->id][$d_k] . '</em>';
    //    }
    //  }
    //  //$table_rows[$row->id]['class'] = array('uk-alert-danger');
    //}
    //elseif ($row->status == AVTXNS_TXN_STATUS_VOID) {
    //  //$table_rows[$row->id]['class'] = array('uk-row-mute');
    //  foreach ($table_rows[$row->id] as $d_k => $d) {
    //    if (!empty($table_rows[$row->id][$d_k])) {
    //      $table_rows[$row->id][$d_k] = '<span class="uk-text-muted uk-text-italic">' . $table_rows[$row->id][$d_k] . '</span>';
    //    }
    //  }
    //}
  }


  if (!empty($table_rows) && $col_total && $limit === NULL) {
    //foreach ($row_totals as $rt) {
    //  $list_total += $rt;
    //}
    // Add summary rows at the bottom of the table.
    $table_rows[] = array(array('data' => NULL, 'colspan' => count($header)));
    $colspan = array_search('total', array_keys($header));
    $totals_table_row = array();
    $totals_table_row[] = array('data' => '<div class="uk-text-right">TOTAL</div>', 'colspan' => $colspan);
    $totals_table_row[] = '<div class="uk-text-right">' . number_format($list_total, 2) . '</div>';
    if ($col_paid) {
      $totals_table_row[] = '<div class="uk-text-right">' . number_format($paid_total, 2) . '</div>';
      $totals_table_row[] = '<div class="uk-text-right">' . number_format($balance_total, 2) . '</div>';
    }

    $remaining_cols = count($header) - ($colspan + (count($totals_table_row) - 1));
    if ($remaining_cols) {
      $totals_table_row[] = array('data' => NULL, 'colspan' => $remaining_cols);
    }
    $table_rows[] = $totals_table_row;
  }



  $transaction_list_table['table'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $table_rows,
    '#empty' => '<div class="uk-text-muted">' . t('No match found') . '</div>',
    '#attributes' => array(
      'id' => 'transaction-list-table',
      'class' => array('uk-table-condensed uk-table-hover uk-text-small uk-text-uppercase'),
    ),
  );

  if (1) {
    if (isset($user_filter['simplified_status'])) {
      $transaction_list_table['simplified_status'] = array(
        '#type' => 'item',
        '#markup' => check_plain($filter_form_settings['simplified_status']['#options'][$user_filter['simplified_status']]) . ' ' . (empty($secondary_transaction_type) ? $ttd['plural_name'] : 'Balances'),
      );
      if (!empty($user_filter['overdue'])) {
        $transaction_list_table['simplified_status']['#markup'] .= ' (OVERDUE)';
      }
      $transaction_list_table['simplified_status']['#markup'] = strtoupper($transaction_list_table['simplified_status']['#markup']);
    }
    avtxns_txn_list_table_add_date_from($transaction_list_table, $user_filter);
    if (!empty($user_filter['payment_date_to'])) {
      $transaction_list_table['payment_date_to'] = array(
        '#type' => 'item',
        //'#title' => 'Payments made until',
        //'#title_display' => 'inline-before',
        '#markup' => 'Showing payments made until ' . check_plain($user_filter['payment_date_to']),
      );
    }
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

    if (!empty($user_filter['return_type'])) {
      $transaction_list_table['return_type'] = array(
        '#type' => 'item',
        '#title' => 'Return type',
        '#title_display' => 'inline-before',
        '#markup' => $return_types[$user_filter['return_type']],
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

    if (!empty($user_filter['avtxn.reference_id'])) {
      $transaction_list_table['avtxn.reference_id'] = array(
        '#type' => 'item',
        '#title' => 'Reference ID',
        '#title_display' => 'inline-before',
        '#markup' => check_plain($user_filter['avtxn.reference_id']),
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
      'class' => array('printable av-sticky-small')
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

  // Hack to minimize caching size.
  if (empty($transaction_list_table['table']['#type']) || $transaction_list_table['table']['#type'] != 'tableselect') {
    $form['#form_keys_no_cache'] = array('transaction_list_table');
  }
  // Show / hide preview?
  avreports_report_live_preview($form);
  return $form;
}