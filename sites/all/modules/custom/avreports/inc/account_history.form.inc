<?php

/**
 * Account history form.
 * @param $form
 * @param $form_state
 *
 * @return array
 */
function avreports_account_history_form($form, &$form_state) {
  ini_set('memory_limit', AVBASE_EXPANDED_MEMORY_LIMIT);
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
    'avachart.title' => array(
      '#title' => t('Account Name'),
    ),
    // 'avtxn.reference_id' => array(
    //   '#title' => t('Reference ID'),
    // ),
    'reset_button' => array('#attributes' => array('style' => 'margin-top: 25px;')),
  );
  $form['transaction_list_filter'] = avtxns_txn_list_filter_form($form, $form_state, $filter_form_settings);
  // Prepare query variables.
  $select = $joins = $where = $args = array();
  $user_filter = empty($form_state['values']['avtxns_tl_filter']) ? array() : $form_state['values']['avtxns_tl_filter'];
  $filter_query = empty($user_filter['filter_query']) ? array() : $user_filter['filter_query'];
  $where = empty($filter_query['where']) ? array() : $filter_query['where'];
  $args = empty($filter_query['args']) ? array() : $filter_query['args'];

  // Header.
  $header = array();
  //$header['product_title'] = array('data' => 'PRODUCT', 'field' => 'avprod.title');
  // $header['transaction_type'] = array('data' => 'TYPE');
  $header['transaction_date'] = array('data' => 'DATE', 'field' => 'avtxn.transaction_date');
  $header['transaction_id'] = array('data' => 'TXN. #', 'field' => 'avtxn.id');
  $header['display_name'] = array('data' => 'CLIENT');
  $header['description'] = array('data' => 'DESCRIPTION');
  $header['debit'] = array('data' => 'DEBIT', 'class' => array('uk-text-right'));
  $header['credit'] = array('data' => 'CREDIT', 'class' => array('uk-text-right'));
  $header['reference_id'] = array('data' => 'REF. ID');

  // Account details.
  if (!empty($user_filter['avachart.title'])) {
    $selected_account = avtxns_account_chart_query_load(array('avachart.title = :title'), array(':title' => $user_filter['avachart.title']));
    $selected_account = empty($selected_account) ? NULL : array_shift($selected_account);
  }

  // Initialize.
  $table_rows = array();
  $debit_total = $credit_total = $account_opening_balance = 0;

  // Only show stuff if selected account exists.
  if (!empty($selected_account)) {
    // No need to filter mysql results by entered Account name.
    unset($where['avachart.title']);
    unset($args[':avacharttitle']);

    // Get opening balance.
    $query = "SELECT amount FROM {avtbl_fs} WHERE account_id = :account_id AND before_this_day <= :today_midnight ORDER BY before_this_day DESC LIMIT 1";
    $account_opening_balance = db_query($query, array(':account_id' => $selected_account->id, ':today_midnight' => $args[':date_from_stamp']))->fetchField();
    $account_opening_balance = empty($account_opening_balance) ? 0 : $account_opening_balance;

    // If no balance, try the very last amount.
    // if (empty($account_opening_balance)) {
    //   $query = "SELECT amount FROM {avtbl_fs} WHERE account_id = :account_id ORDER BY before_this_day DESC LIMIT 1";
    //   $account_opening_balance = db_query($query, array(':account_id' => $selected_account->id))->fetchField();
    //   $account_opening_balance = empty($account_opening_balance) ? 0 : $account_opening_balance;
    // }


    // Balance type.
    $actd = avtxns_account_chart_types($selected_account->account_type);
    $account_balance_type = $actd['balance_type'];
    // $types_with_account_id = avtxns_get_types_with_account_id($selected_account->id);

    // Set default order.
    if (empty($_GET['order'])) {
      $_GET['order'] = $header['transaction_id']['data'];
      $_GET['sort'] = 'ASC';
    }
    $order_by = avbase_build_header_order_array($header);

    // $rs = db_query($query);
    // foreach ($rs as $row) {
    //   $account_details[$row->id] = $row;
    // }

    // Transaction.
    $select[] = 'avtxn.id';
    $select[] = 'avtxn.transaction_date';
    $select[] = 'avtxn.transaction_type';
    $select[] = 'avtxn.reference_id';
    $select[] = 'avtxn.memo';
    $where['transaction_status'] = 'avtxn.status NOT IN (' . AVTXNS_TXN_STATUS_PENDING . ',' . AVTXNS_TXN_STATUS_VOID . ')';
    // $where[] = 'avtxn.id IN(137826, 138004)';
    // $where[] = "avtxn.transaction_type IN('obll')";
    // Client.
    $select[] = 'avclient.display_name';
    $joins[] = 'LEFT JOIN {avtbl_clients} AS avclient ON avclient.id = avtxn.client_id';
    // Transaction details.
    // $account_transaction_types = ;
    $select[] = "IF (avtxn.transaction_type IN ('" . implode("','", avtxns_get_txn_account_types()) . "'), avtxn_detail.id, avtxn.id) AS transaction_key";
    if ($selected_account->id == AVTXN_AC_SPECIFIC_SALES_DISCOUNT_ID) {
      $select[] = "IF (avtxn.transaction_type IN ('" . implode("','", avtxns_get_txn_account_types()) . "'), (" . avtxns_txn_detail_sum_query() . "),(" . avtxns_txn_detail_discount_query(TRUE) . ")) AS grand_total";
    }
    elseif ($selected_account->id == AVTXN_AC_SPECIFIC_SALES_ID) {
      $select[] = avtxns_txn_detail_sum_query() . ' AS grand_total';
    }
    else {
      $select[] = avtxns_txn_detail_sum_query_discount_applied() . ' AS grand_total';
    }
    $select[] = 'avtxn_detail.description';
    $select[] = 'avtxn_detail.debit AS debit';
    $select[] = 'avtxn_detail.credit AS credit';
    $select[] = 'avtxn_detail.ref_txn_type';
    $joins[] = 'INNER JOIN {avtbl_transaction_details} AS avtxn_detail ON avtxn_detail.transaction_id = avtxn.id';
    $account_id_cases = array();
    $transaction_types = avtxns_txn_types();
    // $account_transaction_types = avtxns_get_txn_account_types();
    // $debug_types = array(AVTXNS_TXN_TYPE_PAY_BILL);
    foreach ($transaction_types as $ttype => $tt) {
      // continue;
      if (!empty($debug_types) && !in_array($ttype, $debug_types)) {
        continue;
      }
      if (!empty($tt['account_ids'])) {
        // Regular transactions that must automatically reflect on specific account ids.
        $ref_txn_types = $tt['account_ids'];
        foreach ($ref_txn_types as $rttype => $tt_account_ids) {
          if (empty($tt_account_ids)) {
            continue;
          }
          if (!in_array($selected_account->id, array_keys($tt_account_ids))) {
            continue;
          }

          $ref_txn_type_condition = "avtxn_detail.ref_txn_type = '$rttype'";
          if ($rttype == 'default') {
            $ref_txn_type_condition = "avtxn_detail.ref_txn_type IS NULL OR avtxn_detail.ref_txn_type = ''";
          }
          $account_id_cases[] = "WHEN (avtxn.transaction_type = '$ttype' AND ($ref_txn_type_condition)) THEN 1";
        }
      }
      elseif (!empty($tt['default_account_id'])) {
        // Transactions where users input the account id, but must also automatically reflect on another account id to balance it.
        // e.g.: Credit memo, Charges, etc.
        $default_account_id_match = isset($tt['default_account_id'][$selected_account->id]) ? 1 : 0;
        $account_id_cases[] = "WHEN (avtxn.transaction_type = '$ttype' AND (avtxn_detail.ref_txn_type IS NULL OR avtxn_detail.ref_txn_type = '') AND ($default_account_id_match OR avtxn_detail.account_id = {$selected_account->id})) THEN 1";
      }
    }

    $account_id_cases = empty($account_id_cases) ? array('WHEN NULL THEN NULL') : $account_id_cases;
    $account_id_case_string = '(CASE ' . implode(' ', $account_id_cases) . ' ELSE (avtxn_detail.account_id = ' . $selected_account->id . ') END)';
    // $account_id_case_string = '(CASE ' . implode(' ', $account_id_cases) . ' ELSE (0) END)';
    $where['account_id'] = $account_id_case_string;

    // Build query string.
    $group_by[] = 'transaction_key';
    $query = "SELECT " . implode(', ', $select) . " FROM {avtbl_transactions} AS avtxn " . implode(' ', $joins);
    avbase_query_supplements($query, $where, $order_by, NULL, NULL, $group_by);
    $rs = db_query($query, $args);
    foreach ($rs as $row) {

      $data = array();
      $this_ttd = $transaction_types[$row->transaction_type];
      // Columns.
      $data['date'] = format_date($row->transaction_date, 'custom', AVREPORTS_LIST_DATE_FORMAT);
      $data['id'] = l(avbase_txn_number_format($this_ttd['type'], $row->id), "{$this_ttd['base_path']}/$row->id/view");
      $data['display_name'] = empty($row->display_name) ? NULL : check_plain($row->display_name);
      $data['description'] = empty($row->description) ? (empty($row->memo) ? NULL : check_plain($row->memo)) : check_plain($row->description);
      // For non-journal transactions, use grand_total as debit/credit.
      $this_balance_type = NULL;
      if (!empty($this_ttd['account_ids'])) {
        $ref_txn_types = $this_ttd['account_ids'];
        $this_ref_txn_type = empty($row->ref_txn_type) ? 'default' : $row->ref_txn_type;
        $this_balance_type = isset($ref_txn_types[$this_ref_txn_type][$selected_account->id]) ? $ref_txn_types[$this_ref_txn_type][$selected_account->id] : NULL;
      }
      elseif (!empty($this_ttd['default_account_id'])) {
        // e.g.: Credit memo, Charges, etc.
        if (isset($this_ttd['default_account_id'][$selected_account->id])) {
          $this_balance_type = $this_ttd['default_account_id'][$selected_account->id];
        }
        else {
          $this_balance_type = array_shift($this_ttd['default_account_id']);
          $this_balance_type = $this_balance_type == 'debit' ? 'credit' : 'debit';
        }
      }
      if (!empty($this_balance_type)) {
        $this_balance_type = $this_balance_type == $account_balance_type ? 'debit' : 'credit';
        $row->$this_balance_type = $row->grand_total;
      }
      $data['debit'] = empty((float) $row->debit) ? NULL : '<div class="uk-text-right">' . number_format($row->debit, 2) . '</div>';
      $data['credit'] = empty((float) $row->credit) ? NULL : '<div class="uk-text-right">' . number_format($row->credit, 2) . '</div>';
      $data['reference_id'] = empty($row->reference_id) ? NULL : check_plain($row->reference_id);

      // Totals.
      $debit_total += $row->debit;
      $credit_total += $row->credit;

      // Entire row.
      $table_rows[] = $data;
    }

    // Add "totals" row.
    if (!empty($table_rows)) {
      $totals_table_row = array();
      $colspan = array_search('debit', array_keys($header));
      $totals_table_row[] = array(
        'data' => NULL,
        'colspan' => $colspan
      );
      $totals_table_row[] = '<div class="uk-text-right">' . number_format($debit_total, 2) . '</div>';
      $totals_table_row[] = '<div class="uk-text-right">' . number_format($credit_total, 2) . '</div>';
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
  }

  // Add table rows to form.
  $transaction_list_table['table'] = array(
    '#header' => $header,
    '#empty' => '<div class="uk-text-muted">' . (empty($user_filter['avachart.title']) ? 'Input account name to view its history.' : t('No match found.')) . '</div>',
    '#attributes' => array(
      'id' => 'transaction-list-table',
      'class' => array('uk-table-condensed uk-table-hover uk-text-small'),
    ),
  );
  $transaction_list_table['table']['#theme'] = 'table';
  $transaction_list_table['table']['#rows'] = $table_rows;
  avtxns_txn_list_table_add_date_from($transaction_list_table, $user_filter);
  if (!empty($selected_account->id)) {
    $transaction_list_table['product_name'] = array(
      '#type' => 'item',
      '#title_display' => 'inline-before',
    );
    // $transaction_list_table['product_name']['#markup'] = '<span class="uk-badge uk-text-uppercase uk-badge-warning">' . $actd['name'] . '</span>';
    $transaction_list_table['product_name']['#markup'] = '<h2 class="uk-margin-remove">' . check_plain($user_filter['avachart.title']) . '</h2>';
    // $transaction_list_table['product_name']['#markup'] .= '<div class="uk-text-muted uk-text-small">Opening balance: ' . avbase_accounting_number_format($account_opening_balance) . '</div>';
    $period_balance = ($debit_total - $credit_total) * ($account_balance_type == 'credit' ? -1 : 1);
    // dpm($account_balance_type);
    $transaction_list_table['product_name']['#markup'] .= '<div class="">Period balance: ' . avbase_accounting_number_format($period_balance) . '</div>';
    // $transaction_list_table['product_name']['#markup'] .= '<div class="">Ending balance: ' . avbase_accounting_number_format($account_opening_balance + $period_balance) . '</div>';
    $transaction_list_table['other_info']['#markup'] = '<div class="uk-text-muted uk-text-small uk-text-right">Debits: ' . number_format($debit_total, 2) . '</div>';
    $transaction_list_table['other_info']['#markup'] .= '<div class="uk-text-muted uk-text-small uk-text-right">Credits: ' . number_format($credit_total, 2) . '</div>';
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