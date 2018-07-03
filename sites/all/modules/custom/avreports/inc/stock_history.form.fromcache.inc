<?php

function avreports_stock_history_form($form, &$form_state) {
  ini_set('memory_limit', AVBASE_EXPANDED_MEMORY_LIMIT);
  $dad = avtxns_txn_list_filter_date_auto_options(AVBASE_MONTH_ONLY_FORMAT);
  $dad_default = empty($dad['this_month']) ? array() : $dad['this_month'];
  $filter_form_settings = array(
    'date_auto' => array(
      '#title' => t('Dates'),
      '#default_value' => 'this_month',
    ),
    'date_from' => array(
      '#title' => t('Date from'),
      '#default_value' => $dad_default['date_from'],
      '#av_date_format' => AVBASE_MONTH_ONLY_FORMAT,
    ),
    'date_to' => array(
      '#title' => t('Date to'),
      '#default_value' => $dad_default['date_to'],
      '#av_date_format' => AVBASE_MONTH_ONLY_FORMAT,
    ),
    'avprod.title' => array(
      '#title' => t('Product'),
    ),
    'avtxn.transaction_type' => array('#title' => 'Type', '#options' => avtxns_txn_types_options_array(FALSE, avtxns_get_qty_change_txn_types())),
    'reset_button' => array('#attributes' => array('style' => 'margin-top: 25px;')),
  );
  $form['transaction_list_filter'] = avtxns_txn_list_filter_form($form, $form_state, $filter_form_settings);
  // Prepare query variables.
  $select = $joins = $where = $args = array();
  $user_filter = empty($form_state['values']['avtxns_tl_filter']) ? array() : $form_state['values']['avtxns_tl_filter'];
  $filter_query = empty($user_filter['filter_query']) ? array() : $user_filter['filter_query'];
  $where = empty($filter_query['where']) ? array() : $filter_query['where'];
  $args = empty($filter_query['args']) ? array() : $filter_query['args'];
  // Group by.
  $group_by = array();
  $group_by[] = 'avtxn.id';
  // $group_by[] = 'avtxn_detail.item_id';
  // Order by.
  $order_by = array();
  $order_by[] = 'avtxn.transaction_date';
  $order_by[] = 'avtxn.id';

  // Header.
  $header = array();
  $header['transaction_type'] = array('data' => 'TYPE');
  $header['transaction_date'] = array('data' => 'DATE');
  $header['transaction_id'] = array('data' => 'TXN. #');
  $header['reference_id'] = array('data' => 'REF. ID');
  $header['client_name'] = array('data' => 'CLIENT');
  $header['qty_in'] = array('data' => 'QTY. IN', 'class' => array('uk-text-right'));
  $header['qty_out'] = array('data' => 'QTY. OUT', 'class' => array('uk-text-right'));
  $header['onhand'] = array('data' => 'ON HAND', 'class' => array('uk-text-right'));
  $header['purchase_cost'] = array('data' => 'TOTAL PURCHASE COST', 'class' => array('uk-text-right'));
  // $header['sales_cost'] = array('data' => 'COST OF SALES', 'class' => array('uk-text-right'));
  // $header['ave_cost'] = array('data' => 'UNIT COST', 'class' => array('uk-text-right'));
  // $header['asset_value'] = array('data' => 'ASSET VALUE', 'class' => array('uk-text-right'));

  // Build table rows.
  $table_rows = array();
  $selected_product = NULL;
  $date_start = $args[':date_from_stamp'] = strtotime('first day of this month', $args[':date_from_stamp']);
  $date_end = $args[':date_to_stamp'] = strtotime('first day of next month', $args[':date_to_stamp']);
  $increment_types = array(AVTXNS_TXN_TYPE_RECEIVE, AVTXNS_TXN_TYPE_STOCK_TRANSFER_IN);
  $decrement_types = array(AVTXNS_TXN_TYPE_PURCHASE_RETURN, AVTXNS_TXN_TYPE_STOCK_TRANSFER_OUT);
  $fixed_cost_types = array_merge($increment_types, $decrement_types);
  if (!empty($user_filter['avprod.title'])) {
    $query = "SELECT avprod.id, avprod.title,
      initial_qty AS onhand,
      initial_value AS item_cost,
      (initial_value * initial_qty) AS opening_inventory,
      (initial_value * initial_qty) AS total_purchase_cost,
      initial_qty_date, avcat_uom.title AS uom_title, avclients.display_name AS vendor_name
      FROM {avtbl_products} avprod
      LEFT JOIN avtbl_clients AS avclients ON avclients.id = avprod.vendor_id
      LEFT JOIN avtbl_categories AS avcat_uom ON avcat_uom.id = avprod.uom_id
      WHERE avprod.title = :title";
    $selected_product = db_query($query, array(':title' => $user_filter['avprod.title']))->fetchObject();
  }
  // Only do stuff if selected product exists.
  $do_stuff = !empty($selected_product->id) && $date_start < $date_end;
  if ($do_stuff) {
    // Get product initial values.
    if ($date_start <= strtotime('first day of this month', $selected_product->initial_qty_date)) {
      $row = $selected_product;
      $initial_qty_date = strtotime('today', $selected_product->initial_qty_date);
    }
    else {
      // Get data from cache.
      $query = "SELECT item_cost, (qty_in - qty_out) onhand, SUM(item_cost * (qty_in - qty_out)) AS opening_inventory,
      SUM(qty_in_amount) AS total_purchase_cost
      FROM {avtbl_inventory_cache}
      WHERE product_id = {$selected_product->id} AND before_this_day = $date_start";
      $row = db_query($query)->fetchObject();
      $initial_qty_date = $date_start;
    }

    $item_cost = empty($row->item_cost) ? 0 : $row->item_cost;
    $onhand = empty($row->onhand) ? 0 : $row->onhand;
    $opening_inventory = empty($row->opening_inventory) ? 0 : $row->opening_inventory;
    $total_purchase_cost = empty($row->total_purchase_cost) ? 0 : $row->total_purchase_cost;

    // Add initial product row.
    $data = array();
    $beg_inventory_label = t('Beginning inventory<br />%date', array('%date' => format_date($initial_qty_date, 'custom', 'F Y')));
    $data['transaction_type'] = $beg_inventory_label . (empty($item_cost) ? '<br /><em class="">data not yet available</em>' : '');
    $data['transaction_date'] = empty($item_cost) ? '-' : format_date($initial_qty_date, 'custom', AVREPORTS_LIST_DATE_FORMAT);
    $data['transaction_id'] = NULL;
    $data['reference_id'] = NULL;
    $data['client_name'] = NULL;
    $data['qty_in'] = NULL;
    $data['qty_out'] = NULL;
    $data['onhand'] =  empty($item_cost) ? '-' : ('<div class="uk-text-right">' . avbase_accounting_number_format($onhand, 0) . '</div>');
    $data['purchase_cost'] = empty($item_cost) ? '-' : ('<div class="uk-text-right">' . avbase_accounting_number_format($total_purchase_cost) . '</div>');
    // $data['sales_cost'] = NULL;
    // $data['purchase_cost'] = NULL;
    // Careful when adding classes on rows, check if tableselect mode is ON.
    $table_rows[$selected_product->id . '_init'] = $data;

    // Get most recent product cost.
    $last_cost_query = "SELECT (avtxn_detail.cost / avtxn_detail.qty_per_uom) last_cost
      FROM {avtbl_transaction_details} avtxn_detail
      INNER JOIN {avtbl_transactions} avtxn ON avtxn.id = avtxn_detail.transaction_id
      WHERE avtxn_detail.item_id = :item_id 
      AND avtxn.status <> :void_status AND avtxn.transaction_type IN (:fixed_cost_types)
      AND avtxn.transaction_date >= :init_qty_date
      AND avtxn.transaction_date < :date_to
      ORDER BY avtxn.transaction_date DESC, avtxn.id DESC LIMIT 1";
    $last_cost_args = array(
      ':item_id' => $selected_product->id,
      ':void_status' => AVTXNS_TXN_STATUS_VOID,
      ':fixed_cost_types' => $fixed_cost_types,
      ':init_qty_date' => $initial_qty_date,
      ':date_to' => $date_end,
    );
    $last_cost = db_query($last_cost_query, $last_cost_args)->fetchField();
    $item_cost = empty($last_cost) ? $item_cost : $last_cost;
    // Get transactions made on selected date ranges.
    // Transaction details.
    $select[] = avtxns_txn_detail_qty_query('avtxn.transaction_type IN (:pos_types)') . ' as row_qty';
    $args[':pos_types'] = avtxns_get_qty_change_txn_types('increment');
    // $select[] = avtxns_txn_detail_sum_query_discount_applied("avtxn.transaction_type IN (:inc_types)") . ' AS total_purchase_cost';
    $select[] = 'SUM(
      IF(
        avtxn.transaction_type IN (:fixed_cost_types),
        ' . avtxns_txn_detail_sum_query_discount_applied('avtxn.transaction_type IN (:inc_types)', FALSE) . ',
        NULL
      )
    ) AS purchase_cost';
    // $select[] = 'SUM(IF(
    //   avtxn.transaction_type IN (:fixed_cost_types),
    //   ' . avtxns_txn_detail_sum_query_discount_applied('avtxn.transaction_type IN (:inc_types)', FALSE) . ',
    //   ' . avtxns_txn_detail_sum_query("avtxn.transaction_type IN (:pos_types)", FALSE, "($item_cost * avtxn_detail.qty_per_uom)") . '
    //   )) AS row_cost';
    // dpm(avtxns_txn_detail_sum_query("avtxn.transaction_type IN (:pos_types)", TRUE, "($item_cost * avtxn_detail.qty_per_uom)"));
    $args[':fixed_cost_types'] = $fixed_cost_types;
    $args[':inc_types'] = $increment_types;
    // $args[':pos_types'] = avtxns_get_qty_change_txn_types('increment');
    $where[] = 'avtxn_detail.item_id = :item_id';
    $args[':item_id'] = $selected_product->id;
    // Transaction.
    $select[] = 'avtxn.id AS transaction_id';
    $select[] = 'avtxn.transaction_date';
    $select[] = 'avtxn.transaction_type';
    $select[] = 'avtxn.client_id';
    $select[] = 'avtxn.reference_id';
    $joins[] = "INNER JOIN {avtbl_transactions} AS avtxn ON avtxn.id = avtxn_detail.transaction_id AND avtxn.transaction_type IN (:ttypes)";
    $args[':ttypes'] = avtxns_get_qty_change_txn_types();
    $where[] = 'avtxn.status <> ' . AVTXNS_TXN_STATUS_VOID;
    // Client.
    $select[] = 'avclient.display_name AS client_name';
    $joins[] = 'LEFT JOIN {avtbl_clients} AS avclient ON avclient.id = avtxn.client_id';



    // Clear panggulong variables.
    // unset($where['date_from']);
    // unset($args[':date_from_stamp']);
    unset($where['avtxn.transaction_type']);
    unset($args[':avtxntransaction_type']);
    unset($where['avprod.title']);
    unset($args[':avprodtitle']);

    // Build query.
    $query = "SELECT " . implode(', ', $select) . " FROM {avtbl_transaction_details} AS avtxn_detail " . implode(' ', $joins);
    avbase_query_supplements($query, $where, $order_by, NULL, NULL, $group_by);
    // dpm($query);
    // dpm($args);
    $rs = db_query($query, $args);
    $beg_onhand = $onhand;
    $beg_purchase_cost = $total_purchase_cost;
    $qty_in = $qty_out = 0;
    foreach ($rs as $row) {
      // Only show selected transaction type.
      // if (!empty($user_filter['avtxn.transaction_type']) && $row->transaction_type != $user_filter['avtxn.transaction_type']) {
      //   continue;
      // }
      $this_qty_in = in_array($row->transaction_type, $args[':pos_types']) ? $row->row_qty : 0;
      $this_qty_out = !in_array($row->transaction_type, $args[':pos_types']) ? $row->row_qty : 0;
      $purchase_cost = in_array($row->transaction_type, $fixed_cost_types) ? $row->purchase_cost : 0;
      $qty_in += $this_qty_in;
      $qty_out += $this_qty_out;
      $onhand += empty($row->row_qty) ? 0 : $row->row_qty;
      $total_purchase_cost += $purchase_cost;


      $ttd = avtxns_txn_types($row->transaction_type);
      $txn_view_path = $ttd['base_path'] . "/{$row->transaction_id}/view";
      $data = array();
      $data['transaction_type'] = $ttd['name'];
      $data['transaction_date'] = format_date($row->transaction_date, 'custom', AVREPORTS_LIST_DATE_FORMAT);
      $data['transaction_id'] = l(avbase_txn_number_format($row->transaction_type, $row->transaction_id), $txn_view_path);
      $data['reference_id'] = empty($row->reference_id) ? NULL : check_plain($row->reference_id);
      $data['client_name'] = empty($ttd['ctd']) ? NULL : l($row->client_name, $ttd['ctd']['base_path'] . '/' . $row->client_id . '/view');
      $data['qty_in'] = !empty($this_qty_in) ? ('<div class="uk-text-right">' . number_format($this_qty_in) . '</div>') : NULL;
      $data['qty_out'] = !empty($this_qty_out) ? ('<div class="uk-text-right">' . number_format($this_qty_out) . '</div>') : NULL;
      // $data['purchase_cost'] = in_array($row->transaction_type, $fixed_cost_types) ? ('<div class="uk-text-right">' . avbase_accounting_number_format($total_cost) . '</div>') : NULL;
      $data['onhand'] = '<div class="uk-text-right">' . $onhand . '</div>';
      $data['purchase_cost'] = !empty($purchase_cost) ? ('<div class="uk-text-right">' . avbase_accounting_number_format($row->purchase_cost) . '</div>') : NULL;
      // $data['sales_cost'] = in_array($row->transaction_type, $fixed_cost_types) ? NULL : '<div class="uk-text-right">' . avbase_accounting_number_format($total_cost) . '</div>';
      // $data['ave_cost'] = in_array($row->transaction_type, $fixed_cost_types) ? NULL : '<div class="uk-text-right">' . number_format($item_cost, 2) . '</div>';
      // $data['asset_value'] = '<div class="uk-text-right">' . avbase_accounting_number_format($last_value) . '</div>';

      // Careful when adding classes on rows, check if tableselect mode is ON.
      $table_rows[$row->transaction_id] = $data;
    }

    // Add "totals" row.
    if (!empty($table_rows)) {
      $totals_table_row = array();
      $colspan = array_search('qty_in', array_keys($header));
      if (!empty($colspan)) {
        $totals_table_row[] = array('data' => NULL, 'colspan' => $colspan);
        $totals_table_row[] = '<div class="uk-text-right">' . number_format($qty_in) . '</div>';
        $totals_table_row[] = '<div class="uk-text-right">' . number_format($qty_out) . '</div>';
        $totals_table_row[] = '<div class="uk-text-right">' . number_format($onhand) . '</div>';
        $totals_table_row[] = '<div class="uk-text-right">' . avbase_accounting_number_format($total_purchase_cost) . '</div>';
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



  $transaction_list_table['table'] = array(
    '#header' => $header,
    '#empty' => '<div class="uk-text-muted">' . (empty($user_filter['avprod.title']) ? 'Input product name to view its history.' : t('No match found.')) . '</div>',
    '#attributes' => array(
      'id' => 'transaction-list-table',
      'class' => array('uk-table-condensed uk-table-hover uk-text-small'),
    ),
  );
  $transaction_list_table['table']['#theme'] = 'table';
  $transaction_list_table['table']['#rows'] = $table_rows;
  avtxns_txn_list_table_add_date_from($transaction_list_table, $user_filter);
  if ($do_stuff) {
    $transaction_list_table['product_name'] = array(
      '#type' => 'item',
      '#title_display' => 'inline-before',
      '#markup' => '<span class="uk-text-bold">' . l($user_filter['avprod.title'], "av/products/{$selected_product->id}/view") . '</span>',
    );
    $uom_title = empty($selected_product->uom_title) ? '' : check_plain($selected_product->uom_title);
    $transaction_list_table['product_name']['#markup'] .= '<br />Preferred vendor: ' . check_plain($selected_product->vendor_name);
    $transaction_list_table['product_name']['#markup'] .= "<div class=\"\">On hand: " . number_format($onhand) . " ($uom_title)</div>";
    $transaction_list_table['product_name']['#markup'] .= '<div class="">Recent purchase cost: ' . avbase_accounting_number_format($item_cost) . "</div>";
    $transaction_list_table['other_info']['#markup'] = '<div class=""><span class="uk-margin-right uk-text-muted">Cost of goods available for sale:</span>' . avbase_accounting_number_format($total_purchase_cost) . '</div>';
    // $ending_inventory = $onhand * round($item_cost, 2);
    $ending_inventory = $onhand * $item_cost;
    $transaction_list_table['other_info']['#markup'] .= '<div class=""><span class="uk-margin-right uk-text-muted">Ending inventory:</span>' . avbase_accounting_number_format($ending_inventory) . '</div>';
    $cogs = $total_purchase_cost - $ending_inventory;
    $transaction_list_table['other_info']['#markup'] .= '<div class=""><span class="uk-margin-right uk-text-muted">Cost of sales:</span>' . avbase_accounting_number_format($cogs) . '</div>';
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
