<?php

function avreports_stock_history_form3($form, &$form_state) {
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
  $header['row_qty'] = array('data' => 'QTY.', 'class' => array('uk-text-right'));
  $header['purchase_cost'] = array('data' => 'PURCHASE COST', 'class' => array('uk-text-right'));
  $header['onhand'] = array('data' => 'ON HAND', 'class' => array('uk-text-right'));
  // $header['sales_cost'] = array('data' => 'COST OF SALES', 'class' => array('uk-text-right'));
  // $header['ave_cost'] = array('data' => 'UNIT COST', 'class' => array('uk-text-right'));
  // $header['asset_value'] = array('data' => 'ASSET VALUE', 'class' => array('uk-text-right'));

  // Build table rows.
  $table_rows = array();
  $selected_product = NULL;
  if (!empty($user_filter['avprod.title'])) {
    // Get product initial cost and qty.
    $query = "SELECT 
      avprod.id,
      avprod.title,
      qty AS current_onhand,
      initial_qty,
      initial_value AS last_cost,
      (initial_value * initial_qty) AS asset_value,
      initial_qty_date,
      avclients.display_name AS vendor_name,
      avcat_uom.title AS uom_title
      FROM avtbl_products AS avprod
      LEFT JOIN avtbl_clients AS avclients ON avclients.id = avprod.vendor_id
      LEFT JOIN avtbl_categories AS avcat_uom ON avcat_uom.id = avprod.uom_id
      WHERE avprod.title = (:prod_title)";
    $selected_product = db_query($query, array(':prod_title' => $user_filter['avprod.title']))->fetchObject();
  }

  // Only do stuff if selected product exists.
  $do_stuff = !empty($selected_product->id) && $args[':date_from_stamp'] < $args[':date_to_stamp'];
  if ($do_stuff) {
    // $onhand = empty($selected_product->initial_qty) ? 0 : $selected_product->initial_qty;
    $last_value = empty($selected_product->asset_value) ? 0 : $selected_product->asset_value;
    $increment_types = array(AVTXNS_TXN_TYPE_RECEIVE, AVTXNS_TXN_TYPE_STOCK_TRANSFER_IN);
    $decrement_types = array(AVTXNS_TXN_TYPE_PURCHASE_RETURN, AVTXNS_TXN_TYPE_STOCK_TRANSFER_OUT);
    $fixed_cost_types = array_merge($increment_types, $decrement_types);
    $qty_change_types = avtxns_get_qty_change_txn_types();

    // Get most recent product cost based on selected end date.
    // Use beginning cost if no incoming transactions yet.
    $last_cost_query = "SELECT (avtxn_detail.cost / avtxn_detail.qty_per_uom) last_cost
      FROM {avtbl_transaction_details} avtxn_detail
      INNER JOIN {avtbl_transactions} avtxn ON avtxn.id = avtxn_detail.transaction_id
      WHERE avtxn_detail.item_id = :item_id 
      AND avtxn.status <> :void_status AND avtxn.transaction_type IN (:fixed_cost_types)
      AND avtxn.transaction_date < :date_to
      ORDER BY avtxn.transaction_date DESC, avtxn.id DESC LIMIT 1";
    $last_cost_args = array(
      ':item_id' => $selected_product->id,
      ':void_status' => AVTXNS_TXN_STATUS_VOID,
      ':fixed_cost_types' => $fixed_cost_types,
      ':date_to' => $args[':date_to_stamp'],
    );
    $last_cost = db_query($last_cost_query, $last_cost_args)->fetchField();
    $last_cost = empty($last_cost) ? $selected_product->last_cost : $last_cost;

    // Compute total received quantity up to selected start date then add it to initial quantity.
    $query = "SELECT " . avtxns_txn_detail_qty_query("avtxn.transaction_type IN (:pos_types)") . " onhand
      FROM {avtbl_transaction_details} avtxn_detail
      INNER JOIN {avtbl_transactions} avtxn ON avtxn.id = avtxn_detail.transaction_id
      WHERE avtxn_detail.item_id = :item_id
      AND avtxn.status <> :void_status AND avtxn.transaction_type IN (:qty_change_types) AND avtxn.transaction_date < :date_from";
    $onhand = db_query($query, array(
      ':item_id' => $selected_product->id,
      ':void_status' => AVTXNS_TXN_STATUS_VOID,
      ':pos_types' => avtxns_get_qty_change_txn_types('increment'),
      ':qty_change_types' => $qty_change_types,
      ':date_from' => $args[':date_from_stamp'],
    ))->fetchField();
    $onhand = empty($onhand) ? 0 : $onhand;
    $onhand += $selected_product->initial_qty;

    // Compute total received asset value up to selected start date then add it to initial asset value.
    $query = "SELECT " . avtxns_txn_detail_sum_query_discount_applied("avtxn.transaction_type IN (:inc_types)") . " last_value
      FROM {avtbl_transaction_details} avtxn_detail
      INNER JOIN {avtbl_transactions} avtxn ON avtxn.id = avtxn_detail.transaction_id
      WHERE avtxn_detail.item_id = :item_id
      AND avtxn.status <> :void_status AND avtxn.transaction_type IN (:fixed_cost_types) AND avtxn.transaction_date < :date_from";
    if (!empty($order_by)) {
      $query .= " ORDER BY " . implode(', ', $order_by);
    }
    $this_value = db_query($query, array(
      ':item_id' => $selected_product->id,
      ':void_status' => AVTXNS_TXN_STATUS_VOID,
      ':inc_types' => $increment_types,
      ':fixed_cost_types' => $fixed_cost_types,
      ':date_from' => $args[':date_from_stamp'],
    ))->fetchField();
    $this_value = empty($this_value) ? 0 : $this_value;
    $last_value += $this_value;

    // Compute total value of items that went in/out, then add it to initial asset value.
    // No discount applied because we are computing for the cost, not the sales price, and these are non purchase transactions.
//     $query = "SELECT " . avtxns_txn_detail_sum_query("avtxn.transaction_type IN (:pos_types)", TRUE, "($last_cost * avtxn_detail.qty_per_uom)") . " last_value
//       FROM {avtbl_transaction_details} avtxn_detail
//       INNER JOIN {avtbl_transactions} avtxn ON avtxn.id = avtxn_detail.transaction_id
//       WHERE avtxn_detail.item_id = :item_id
//       AND avtxn.status <> :void_status AND avtxn.transaction_type IN (:fucking_types) AND avtxn.transaction_date < :date_from";
//     if (!empty($order_by)) {
//       $query .= " ORDER BY " . implode(', ', $order_by);
//     }
//     $this_value = db_query($query, array(
//       ':item_id' => $selected_product->id,
//       ':void_status' => AVTXNS_TXN_STATUS_VOID,
//       ':pos_types' => avtxns_get_qty_change_txn_types('increment'),
//       ':fucking_types' => array_diff($qty_change_types, $fixed_cost_types),
//       ':date_from' => $args[':date_from_stamp'],
//     ))->fetchField();
//     $this_value = empty($this_value) ? 0 : $this_value;
//     $last_value += $this_value;
// dpm($this_value);
// dpm($last_value);
    // Get most recent cost based on selected END date this time.
    // $last_cost_args[':date_to'] = $args[':date_from_stamp'];
    // $last_cost = db_query($last_cost_query, $last_cost_args)->fetchField();
    // $last_cost = empty($last_cost) ? $selected_product->last_cost : $last_cost;

    // Initial date.
    $initial_date_timestamp = $args[':date_from_stamp'] > $selected_product->initial_qty_date ? $args[':date_from_stamp'] : $selected_product->initial_qty_date;
    // Add initial product row.
    $data = array();
    $data['transaction_type'] = '<span class="">Beginning inventory</span>';
    $data['transaction_date'] = format_date($initial_date_timestamp, 'custom', AVREPORTS_LIST_DATE_FORMAT);
    $data['transaction_id'] = NULL;
    $data['reference_id'] = NULL;
    $data['client_name'] = NULL;
    $data['row_qty'] = NULL;
    $data['purchase_cost'] = '<div class="uk-text-right">' . avbase_accounting_number_format($last_value) . '</div>';
    $data['onhand'] = '<div class="uk-text-right">' . avbase_accounting_number_format($onhand, 0) . '</div>';
    $data['asset_value'] = '<div class="uk-text-right">' . avbase_accounting_number_format($last_value) . '</div>';
    // $data['sales_cost'] = NULL;
    // $data['purchase_cost'] = NULL;
    // Careful when adding classes on rows, check if tableselect mode is ON.
    $table_rows[$selected_product->id . '_init'] = $data;


    // Get transactions made on selected date ranges.
    // Transaction details.
    $select[] = avtxns_txn_detail_qty_query('avtxn.transaction_type IN (:pos_types)') . ' as row_qty';
    $args[':pos_types'] = avtxns_get_qty_change_txn_types('increment');
    $select[] = 'SUM(IF(
      avtxn.transaction_type IN (:fixed_cost_types),
      ' . avtxns_txn_detail_sum_query_discount_applied('avtxn.transaction_type IN (:inc_types)', FALSE) . ',
      ' . avtxns_txn_detail_sum_query("avtxn.transaction_type IN (:pos_types)", FALSE, "($last_cost * avtxn_detail.qty_per_uom)") . '
      )) AS row_cost';
    // dpm(avtxns_txn_detail_sum_query("avtxn.transaction_type IN (:pos_types)", TRUE, "($last_cost * avtxn_detail.qty_per_uom)"));
    $args[':fixed_cost_types'] = $fixed_cost_types;
    $args[':inc_types'] = $increment_types;
    $args[':pos_types'] = avtxns_get_qty_change_txn_types('increment');
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
    $rs = db_query($query, $args);
    $beg_onhand = $onhand;
    $beg_value = $last_value;
    $rcv_tot = $stin_tot = $pr_tot = $stou_tot = 0;
    foreach ($rs as $row) {
      $onhand += empty($row->row_qty) ? 0 : $row->row_qty;
      // $total_cost = is_null($row->row_cost) ? (empty($row->row_qty) ? 0 : ($row->row_qty * $last_cost)) : $row->row_cost;
      $total_cost = $row->row_cost;
      $last_value += $total_cost;
      if (in_array($row->transaction_type, $fixed_cost_types)) {
        $_varname = $row->transaction_type . '_tot';
        $$_varname += $total_cost;
      }
      // Only show selected transaction type.
      // if (!empty($user_filter['avtxn.transaction_type']) && $row->transaction_type != $user_filter['avtxn.transaction_type']) {
      //   continue;
      // }
      $ttd = avtxns_txn_types($row->transaction_type);
      $txn_view_path = $ttd['base_path'] . "/{$row->transaction_id}/view";
      $data = array();
      $data['transaction_type'] = $ttd['name'];
      $data['transaction_date'] = format_date($row->transaction_date, 'custom', AVREPORTS_LIST_DATE_FORMAT);
      $data['transaction_id'] = l(avbase_txn_number_format($row->transaction_type, $row->transaction_id), $txn_view_path);
      $data['reference_id'] = empty($row->reference_id) ? NULL : check_plain($row->reference_id);
      $data['client_name'] = empty($ttd['ctd']) ? NULL : l($row->client_name, $ttd['ctd']['base_path'] . '/' . $row->client_id . '/view');
      $data['row_qty'] = '<div class="uk-text-right">' . avbase_accounting_number_format($row->row_qty, 0) . '</div>';
      $data['purchase_cost'] = in_array($row->transaction_type, $fixed_cost_types) ? '<div class="uk-text-right">' . avbase_accounting_number_format($total_cost) . '</div>' : NULL;
      $data['onhand'] = '<div class="uk-text-right">' . avbase_accounting_number_format($onhand, 0) . '</div>';
      // $data['sales_cost'] = in_array($row->transaction_type, $fixed_cost_types) ? NULL : '<div class="uk-text-right">' . avbase_accounting_number_format($total_cost) . '</div>';
      // $data['ave_cost'] = in_array($row->transaction_type, $fixed_cost_types) ? NULL : '<div class="uk-text-right">' . number_format($last_cost, 2) . '</div>';
      $data['asset_value'] = '<div class="uk-text-right">' . avbase_accounting_number_format($last_value) . '</div>';

      // Careful when adding classes on rows, check if tableselect mode is ON.
      $table_rows[$row->transaction_id] = $data;
    }

    $total_in = $beg_value + $rcv_tot + $stin_tot + $pr_tot + $stou_tot;
  }

  // Add summary rows at the bottom of the table.
  // if (!empty($table_rows)) {
  //   $table_rows[] = array(array('data' => NULL, 'colspan' => count($header)));
  //   $colspan = array_search('row_qty', array_keys($header));
  //   $totals_table_row = array();
  //   $totals_table_row[] = array('data' => '<div class="uk-text-right">TOTAL</div>', 'colspan' => $colspan);
  //   $totals_table_row[] = '<div class="uk-text-right">' . avbase_accounting_number_format($onhand, 0) . '</div>';
  //
  //   $remaining_cols = count($header) - ($colspan + (count($totals_table_row) - 1));
  //   if ($remaining_cols) {
  //     $totals_table_row[] = array('data' => NULL, 'colspan' => $remaining_cols);
  //   }
  //   $table_rows[] = $totals_table_row;
  // }



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
    // $transaction_list_table['product_name']['#markup'] .= "<div class=\"\">On hand ($uom_title): " . avbase_accounting_number_format($onhand, 0) . "</div>";
    $transaction_list_table['product_name']['#markup'] .= "<div class=\"\">On hand: " . avbase_accounting_number_format($onhand, 0) . " ($uom_title)</div>";
    $transaction_list_table['product_name']['#markup'] .= '<div class="">Recent purchase cost: ' . avbase_accounting_number_format($last_cost) . "</div>";
    // $transaction_list_table['other_info']['#markup'] = '<div class="uk-text-muted uk-text-small">Beginning inventory: ' . avbase_accounting_number_format($beg_value) . '</div>';

    $transaction_list_table['other_info']['#markup'] = '<div class=""><span class="uk-margin-right uk-text-muted">Cost of goods available for sale:</span>' . avbase_accounting_number_format($total_in) . '</div>';
    // $transaction_list_table['other_info']['#markup'] .= '<div id="cgas">
    // <div class="uk-text-muted uk-text-small">Purchases: ' . avbase_accounting_number_format($rcv_tot) . '</div>
    // <div class="uk-text-muted uk-text-small">Branch transfer in: ' . avbase_accounting_number_format($stin_tot) . '</div>
    // <div class="uk-text-muted uk-text-small">Purchase returns: ' . avbase_accounting_number_format($pr_tot) . '</div>
    // <div class="uk-text-muted uk-text-small">Branch transfer out: ' . avbase_accounting_number_format($stou_tot) . '</div>
    // </div>';
    $ending_inventory = $onhand * $last_cost;
    $transaction_list_table['other_info']['#markup'] .= '<div class=""><span class="uk-margin-right uk-text-muted">Ending inventory:</span>' . avbase_accounting_number_format($ending_inventory) . '</div>';
    $cogs = $total_in - $ending_inventory;
    $transaction_list_table['other_info']['#markup'] .= '<div class=""><span class="uk-margin-right uk-text-muted">Cost of sales:</span>' . avbase_accounting_number_format($cogs) . '</div>';

    // Update qty.
    // @todo Remove this in the future.
    // dpm($selected_product);
    // if ($selected_product->current_onhand != $selected_product->onhand) {
    //   db_query("UPDATE {avtbl_products} SET qty = :qty WHERE id = :id", array(':qty' => $selected_product->onhand, ':id' => $selected_product->id));
    // }
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
