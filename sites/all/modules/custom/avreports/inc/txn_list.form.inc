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

  // Extra columns
  $col_client = !empty($ttd['ctd']);
  $col_due_date = !empty($ttd['bill']);
  $col_paid = !empty($ttd['payable']);

  // Custom date optionsdetails.
  $dad = avtxns_txn_list_filter_date_auto_options();
  $dad_default = empty($dad['last_month_to_date']) ? array() : $dad['last_month_to_date'];
  $filter_form_settings = array(
    'date_auto' => array(
      '#title' => 'Dates',
      '#default_value' => 'last_month_to_date',
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
      '#title' => $ttd['name'] . ' Status',
      '#options' => array(
        AVTXNS_TXN_STATUS_LOCKED . '-' . AVTXNS_TXN_STATUS_OPEN => 'Open',
        AVTXNS_TXN_STATUS_CLOSED => 'Closed',
        AVTXNS_TXN_STATUS_VOID => 'Void'
      ),
      '#default_value' => AVTXNS_TXN_STATUS_LOCKED . '-' . AVTXNS_TXN_STATUS_OPEN,
    ),
    'reset_button' => array('#attributes' => array('style' => 'margin-top: 25px;')),
  );
  if ($col_due_date) {
    $filter_form_settings['overdue'] = array('#title' => 'Due status');
  }
  if (!empty($ttd['ctd'])) {
    $filter_form_settings['avcat_agent.title'] = array(
      '#title' => 'Agent',
      '#autocomplete_path' => 'av/categories/agent' . $ttd['ctd']['type'] . '/autocomplete',
    );
    $filter_form_settings['avclients.display_name'] = array(
      '#title' => $ttd['ctd']['name'],
      '#autocomplete_path' => 'av/clients/autocomplete/' . $ttd['ctd']['type'] . '/' . implode('-', array(AVCLIENT_STATUS_ACTIVE, AVCLIENT_STATUS_SUSPENDED)),
      '#attributes' => array(
        'placeholder' => 'search customer',
        'class' => array('trigger-ajax-search'),
      ),
    );
  }
  $form['transaction_list_filter'] = avtxns_txn_list_filter_form($form, $form_state, $filter_form_settings);

  $user_filter = empty($form_state['values']['avtxns_tl_filter']) ? array() : $form_state['values']['avtxns_tl_filter'];
  $filter_query = empty($user_filter['filter_query']) ? array() : $user_filter['filter_query'];
  $where = empty($filter_query['where']) ? array() : $filter_query['where'];
  $args = empty($filter_query['args']) ? array() : $filter_query['args'];
  //$agent_name = empty($user_filter['avcat_agent.title']) ? '' : $user_filter['avcat_agent.title'];




  //if (!empty($agent_name)) {
  //  unset($header['agent']);
  //}



  // Headers.
  $header = array();
  if ($col_client) {
    $header['client'] = array('data' => $ttd['ctd']['name'], 'field' => 'display_name');
  }
  $header['date'] = array('data' => 'Date', 'field' => 'transaction_date');
  $header['id'] = array('data' => strtoupper($transaction_type) . '#', 'field' => 'id');
  $header['term'] = array('data' => 'Terms');
  if ($col_due_date) {
    $header['due_date'] = array('data' => 'Due Date', 'field' => 'due_date');
  }
  $header['total'] = array('data' => 'Total', 'class' => array('uk-text-right'));
  if ($col_paid) {
    $header['paid'] = array('data' => 'Paid', 'class' => array('uk-text-right'));
    $header['balance'] = array('data' => 'Balance', 'class' => array('uk-text-right'));
  }
  $header['agent'] = array('data' => 'Agent', 'field' => 'agent_name');

  // Set table pagination and sort settings.
  if (empty($_GET['order'])) {
    $_GET['order'] = $header['date']['data'];
    $_GET['sort'] = 'desc';
    if (isset($header['client'])) {
      $_GET['order'] = $header['client']['data'];
      $_GET['sort'] = 'asc';
    }
  }
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
  if ($col_due_date && !isset($order_by['due_date'])) {
    $order_by['due_date'] = 'due_date ASC';
  }
  $order_by['transaction_date'] = 'transaction_date ASC';
  $order_by['id'] = 'id DESC';

  $where[] = "avtxn.transaction_type = '" . $transaction_type . "'";
  //dpm($where);
  //$where[] = "avtxn.status NOT IN (" . AVTXNS_TXN_STATUS_VOID . ", " . AVTXNS_TXN_STATUS_PENDING . ")";




  $select = array();
  $select[] = 'avtxn.transaction_date';
  if ($col_due_date) {
    $select[] = 'avtxn.due_date';
    //$col_due_date && $row->due_date <= $now && $row->status != AVTXNS_TXN_STATUS_CLOSED;
    $select[] = avtxns_txn_overdue_query() . ' AS overdue';
    $select[] = 'avtxn.due_date AS gg';
  }
  $select[] = 'avtxn_detail.ref_txn_type';
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
  $joins = array();
  $joins[] = 'LEFT JOIN {avtbl_transaction_details} AS avtxn_detail ON avtxn_detail.transaction_id = avtxn.id';
  if ($col_client) {
    $select[] = 'avtxn_detail.client_id';
    $select[] = 'avclients.display_name';
    $joins[] = 'LEFT JOIN {avtbl_clients} AS avclients ON avclients.id = avtxn.client_id';
  }
  $group_by = array('avtxn.id', 'avtxn_detail.ref_txn_type');
  $transactions = avtxns_txns_query_load($where, $args, NULL, NULL, $order_by, $select, $joins, $group_by);
  $list_total = $paid_total = $balance_total = 0;
  $row_totals = array();
  $table_rows = array();
  //$now = time();
  $last_client_name = '';
  foreach ($transactions as $row) {
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
    if ($col_paid) {
      $row_bal_html = number_format($row_total - $row->paid_amount, 2);
    }

    if ($overdue) {
      $row_bal_html .= ' <em><i class="uk-icon-exclamation-circle" title="Overdue"></i></em>';
    }

    if (!isset($table_rows[$row->id])) {
      if ($col_client) {
        $data['client'] = array('data' => l($row->display_name, $ttd['ctd']['base_path'] . '/' . $row->client_id . '/view'), 'style' => 'background: #fff;');
      }
      $data['date'] = array('data' => l(format_date($row->transaction_date, 'custom', 'Y-m-d'), $view_path));
      $data['id'] = array('data' => l(avbase_txn_number_format($row->transaction_type, $row->id), $view_path));
      $data['term'] = array('data' => check_plain($row->term_name));
      if ($col_due_date) {
        $data['due_date'] = array('data' => format_date($row->due_date, 'custom', 'Y-m-d'));
      }
      $data['total'] = array('data' => $row_total_html, 'class' => array('uk-text-right'));
      if ($col_paid) {
        $paid_amount = empty($row->paid_amount) ? '' : number_format($row->paid_amount, 2);
        $data['paid'] = array('data' => $paid_amount, 'class' => array('uk-text-right'));
        $data['balance'] = array('data' => $row_bal_html, 'class' => array('uk-text-right'));
        $paid_total += $row->paid_amount;
      }
      $data['agent'] = array('data' => check_plain($row->agent_name));
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

      $sorted_by = empty($_GET['order']) ? '' : $_GET['order'];
      if ($col_client && $sorted_by == $header['client']['data'] && $last_client_name == $row->display_name) {
        $table_rows[$row->id]['data']['client']['data'] = '';
      }
      $last_client_name = $row->display_name;
    }
    else {
      $table_rows[$row->id]['data']['total']['data'] =  $row_total_html;
      $table_rows[$row->id]['data']['balance']['data'] =  $row_bal_html;
    }
  }


  if (!empty($table_rows)) {
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

    if ($colspan < count($header)) {
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
      'class' => array('uk-table-condensed '),
    ),
  );

  if (!empty($table_rows)) {
    if (isset($user_filter['simplified_status'])) {
      $transaction_list_table['simplified_status'] = array(
        '#type' => 'item',
        '#markup' => $filter_form_settings['simplified_status']['#options'][$user_filter['simplified_status']] . ' ' . $ttd['plural_name'],
      );
      if (!empty($user_filter['overdue'])) {
        $transaction_list_table['simplified_status']['#markup'] = 'Overdue ' . $ttd['plural_name'];
      }
    }
    if (!empty($user_filter['date_from'])) {
      $date_text = empty($user_filter['date_from']) ? '' : check_plain($user_filter['date_from']);
      if (!empty($user_filter['date_to']) && $user_filter['date_from'] != $user_filter['date_to']) {
        $date_text .= ' - ' . $user_filter['date_to'];
      }
      $transaction_list_table['date_from'] = array(
        '#type' => 'item',
        '#markup' => $date_text,
      );
    }
    //$transaction_list_table['agent'] = array(
    //  '#title' => 'Agent',
    //  '#title_display' => 'inline-before',
    //  '#type' => 'item',
    //  '#markup' => check_plain($agent_name),
    //  '#access' => !empty($agent_name),
    //);
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