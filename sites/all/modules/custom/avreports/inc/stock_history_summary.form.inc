<?php

function avreports_stock_history_summary_form($form, &$form_state) {
  // ini_set('memory_limit', AVBASE_EXPANDED_MEMORY_LIMIT);
  $transaction_lock_date = variable_get('avtxns_as_of_date_lock', NULL);
  if (empty($transaction_lock_date)) {
    $default_date = strtotime('today', time());
    $default_date = strtotime('last day of last month', $default_date);
  }
  else {
    $default_date = $transaction_lock_date;
  }
  $filter_form_settings = array(
    'date_to' => array(
      '#title' => t('Period'),
      '#default_value' => format_date($default_date, 'custom', AVBASE_MONTH_ONLY_FORMAT),
      '#av_date_format' => AVBASE_MONTH_ONLY_FORMAT,
    ),
    'reset_button' => array('#button_label' => 'This month', '#attributes' => array('style' => 'margin-top: 25px;')),
  );

  // $filter_form_settings['avcat_prodcat.id'] = array('#options' => avbase_get_category_options_array('product_cat'));
  // $filter_form_settings['product_status'] = array('#default_value' => AVPRODUCTS_STATUS_ACTIVE);
  $form['transaction_list_filter'] = avtxns_txn_list_filter_form($form, $form_state, $filter_form_settings, AVBASE_MONTH_ONLY_FORMAT);

  // Prepare query variables.
  $select = $joins = $where = $args = array();
  $user_filter = empty($form_state['values']['avtxns_tl_filter']) ? array() : $form_state['values']['avtxns_tl_filter'];
  $filter_query = empty($user_filter['filter_query']) ? array() : $user_filter['filter_query'];
  $where = empty($filter_query['where']) ? array() : $filter_query['where'];
  $args = empty($filter_query['args']) ? array() : $filter_query['args'];

  // $group_by[] = 'avtxn_detail.item_id';
  // Order by.
  $order_by = array();
  // $order_by[] = 'avtxn.transaction_date';
  // $order_by[] = 'avtxn.id';

  // Header.
  $header = array();
  $header['product_title'] = array('data' => 'TITLE', 'field' => 'title');
  $header['uom'] = array('data' => 'UOM');
  $header['uom']['data-uk-tooltip'] = '{cls: "tt-blue"}';
  $header['uom']['title'] = 'Base unit of measure';

  $header['item_cost'] = array('data' => 'COST', 'class' => array('uk-text-right'), 'field' => 'item_cost');
  $header['item_cost']['data-uk-tooltip'] = '{cls: "tt-blue"}';
  $header['item_cost']['title'] = 'Most recent unit cost';

  $header['beg_qty'] = array('data' => 'BEG', 'class' => array('uk-text-right'), 'field' => 'beg_qty');
  $header['beg_qty']['data-uk-tooltip'] = '{cls: "tt-blue"}';
  $header['beg_qty']['title'] = 'Beginning quantity';

  $header['rcv_qty'] = array('data' => 'RCV', 'class' => array('uk-text-right'), 'field' => 'rcv_qty');
  $header['rcv_qty']['data-uk-tooltip'] = '{cls: "tt-blue"}';
  $header['rcv_qty']['title'] = 'Received quantity';

  $header['dr_qty'] = array('data' => 'DR', 'class' => array('uk-text-right'), 'field' => 'dr_qty');
  $header['dr_qty']['data-uk-tooltip'] = '{cls: "tt-blue"}';
  $header['dr_qty']['title'] = 'Delivered quantity';

  $header['stou_qty'] = array('data' => 'STOU', 'class' => array('uk-text-right'), 'field' => 'stou_qty');
  $header['stou_qty']['data-uk-tooltip'] = '{cls: "tt-blue"}';
  $header['stou_qty']['title'] = 'Transferred-OUT quantity';

  $header['stin_qty'] = array('data' => 'STIN', 'class' => array('uk-text-right'), 'field' => 'stin_qty');
  $header['stin_qty']['data-uk-tooltip'] = '{cls: "tt-blue"}';
  $header['stin_qty']['title'] = 'Transferred-IN quantity';

  $header['sr_qty'] = array('data' => 'SR', 'class' => array('uk-text-right'), 'field' => 'sr_qty');
  $header['sr_qty']['data-uk-tooltip'] = '{cls: "tt-blue"}';
  $header['sr_qty']['title'] = 'Sales return quantity';

  $header['pr_qty'] = array('data' => 'PR', 'class' => array('uk-text-right'), 'field' => 'pr_qty');
  $header['pr_qty']['data-uk-tooltip'] = '{cls: "tt-blue"}';
  $header['pr_qty']['title'] = 'Purchase return quantity';

  $header['stk_qty'] = array('data' => 'ADJ', 'class' => array('uk-text-right'), 'field' => 'stk_qty');
  $header['stk_qty']['data-uk-tooltip'] = '{cls: "tt-blue"}';
  $header['stk_qty']['title'] = 'Adjusted quantity';

  $header['end_qty'] = array('data' => 'END', 'class' => array('uk-text-right'), 'field' => 'end_qty');
  $header['end_qty']['data-uk-tooltip'] = '{cls: "tt-blue"}';
  $header['end_qty']['title'] = 'Ending quantity';

  $header['cogafs'] = array('data' => 'COGAFS', 'class' => array('uk-text-right'), 'field' => 'cogafs');
  $header['cogafs']['data-uk-tooltip'] = '{cls: "tt-blue"}';
  $header['cogafs']['title'] = 'Cost of goods available for sale';

  $header['inventory'] = array('data' => 'INVENTORY', 'class' => array('uk-text-right'), 'field' => 'inventory');
  // $header['inventory']['data-uk-tooltip'] = '{cls: "tt-blue"}';
  // $header['inventory']['title'] = 'Inventory';

  $header['cogs'] = array('data' => 'COGS', 'class' => array('uk-text-right'), 'field' => 'cogs');
  $header['cogs']['data-uk-tooltip'] = '{cls: "tt-blue"}';
  $header['cogs']['title'] = 'Cost of goods sold';

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

  // Build table rows.
  $table_rows = array();
  // $date_start = $args[':date_from_stamp'];
  $date_end = $args[':date_to_stamp'] = strtotime('first day of next month', $args[':date_to_stamp']);

  // Group by.
  $group_by = array();
  // $group_by[] = 'avprod.id';
  $select[] = 'avprod.id';
  $select[] = 'avprod.title';
  $select[] = 'avinvcache.*';
  // $select[] = "IF (avinvcache.product_id IS NULL, (SELECT " . avtxns_inv_cache_query('end_qty', FALSE, 'ayay') . " FROM {avtbl_inventory_cache} ayay WHERE ayay.before_this_day <= :date_to_stamp AND ayay.product_id = avprod.id ORDER BY before_this_day DESC LIMIT 1), avinvcache.beg_qty) beg_qty";
  // $select[] = "IF (avinvcache.product_id IS NULL, (SELECT ayay.item_cost FROM {avtbl_inventory_cache} ayay WHERE ayay.before_this_day <= :date_to_stamp AND ayay.product_id = avprod.id ORDER BY before_this_day DESC LIMIT 1), avinvcache.item_cost) item_cost";
  // $select[] = "IF (avinvcache.product_id IS NULL, (SELECT ayay.beg_amt FROM {avtbl_inventory_cache} ayay WHERE ayay.before_this_day <= :date_to_stamp AND ayay.product_id = avprod.id ORDER BY before_this_day DESC LIMIT 1), avinvcache.beg_amt) beg_amt";
  // $select[] = avtxns_inv_cache_query('end_qty') . " end_qty";
  // $select[] = avtxns_inv_cache_query('cogafs') . " cogafs";
  // $select[] = avtxns_inv_cache_query('inventory') . " inventory";
  // $select[] = avtxns_inv_cache_query('cogs') . " cogs";
  $joins[] = "LEFT JOIN {avtbl_inventory_cache} avinvcache ON avinvcache.product_id = avprod.id AND avinvcache.before_this_day = :date_to_stamp";
  // $joins[] = "LEFT JOIN {avtbl_inventory_cache} avinvcache ON avinvcache.product_id = avprod.id AND avinvcache.before_this_day = (SELECT ayay.before_this_day FROM {avtbl_inventory_cache} ayay WHERE ayay.before_this_day <= :date_to_stamp AND ayay.product_id = avprod.id ORDER BY before_this_day DESC LIMIT 1)";
  $select[] = 'avcat_uom.title AS uom_title';
  $joins[] = 'LEFT JOIN {avtbl_categories} AS avcat_uom ON avcat_uom.id = avprod.uom_id';
  // $joins[] = 'LEFT JOIN {avtbl_inventory_cache} avinvcache ON avinvcache.product_id = avprod.id AND avinvcache.before_this_day = (SELECT ayay.before_this_day FROM {avtbl_inventory_cache} ayay WHERE ayay.before_this_day <= :date_to_stamp AND ayay.product_id = avprod.id ORDER BY before_this_day DESC LIMIT 1)';
  // $joins[] = 'LEFT JOIN {avtbl_inventory_cache} avinvcache ON avinvcache.product_id = avprod.id AND avinvcache.before_this_day = :date_to_stamp';

  // $where[] = 'avprod.id=2209';

  // $where['date_to'] = "avinvcache.before_this_day = :date_to_stamp";
  unset($where['date_to']);
  // Build query.
  $query = "SELECT " . implode(', ', $select) . " FROM {avtbl_products} AS avprod " . implode(' ', $joins);
  avbase_query_supplements($query, $where, $order_by, NULL, NULL, $group_by);
  // dpm($where);
  // dpm($query);
  // dpm($args);
  $rs = db_query($query, $args);
  $tot_inventory = $tot_cogafs = $tot_cogs = 0;
  // dpm($date_end);
  foreach ($rs as $row) {
    // If no matching cache on selected date. Find the very recent one.
    if (empty($row->product_id)) {
      $query = "SELECT
        " . avtxns_inv_cache_query('end_qty') . " end_qty,
        avinvcache.end_cost
        FROM {avtbl_inventory_cache} avinvcache
        WHERE avinvcache.before_this_day <= :date_to_stamp AND avinvcache.product_id = {$row->id}
        ORDER BY avinvcache.before_this_day DESC LIMIT 1";
      $last_cache = db_query($query, $args)->fetchObject();
      $row->beg_qty = empty($last_cache->end_qty) ? 0 : $last_cache->end_qty;
      $row->beg_cost = empty($last_cache->end_cost) ? 0 : $last_cache->end_cost;
      $row->end_cost = empty($last_cache->end_cost) ? 0 : $last_cache->end_cost;
    }

    $end_qty = $row->beg_qty + $row->rcv_qty + $row->stin_qty + $row->pr_qty + $row->stou_qty + $row->dr_qty + $row->sr_qty + $row->stk_qty;
    $inventory = $end_qty * $row->end_cost;
    // $end_qty = $row->rcv_qty + $row->stin_qty + $row->pr_qty + $row->stou_qty + $row->dr_qty + $row->sr_qty + $row->stk_qty;
    // $inventory = ($row->beg_qty * $row->beg_cost) + ($end_qty * $row->end_cost);
    $cogafs = ($row->beg_qty * $row->beg_cost) + $row->rcv_amt + $row->stin_amt + $row->pr_amt + $row->stou_amt;
    $cogs = $cogafs - $inventory;
    // Total.
    $tot_inventory += $inventory;
    $tot_cogafs += $cogafs;
    $tot_cogs += $cogs;
    $data = array();
    $data['product_title'] = avreports_product_history_link(
      $row->title, array(
        'date_from' => format_date(strtotime('first day of this month - 1 month', $date_end), 'custom', AVBASE_DATE_FORMAT),
        'date_to' => format_date(strtotime('last day of this month - 1 month', $date_end), 'custom', AVBASE_DATE_FORMAT),
      )
    );
    $data['uom'] = check_plain($row->uom_title);
    $data['item_cost'] = '<div class="uk-text-right">' . number_format($row->end_cost, 2) . '</div>';
    $data['beg_qty'] = '<div class="uk-text-right">' . number_format($row->beg_qty) . '</div>';
    $data['rcv_qty'] = '<div class="uk-text-right">' . number_format($row->rcv_qty) . '</div>';
    $data['dr_qty'] = '<div class="uk-text-right">' . number_format($row->dr_qty) . '</div>';
    $data['stou_qty'] = '<div class="uk-text-right">' . number_format($row->stou_qty) . '</div>';
    $data['stin_qty'] = '<div class="uk-text-right">' . number_format($row->stin_qty) . '</div>';
    $data['sr_qty'] = '<div class="uk-text-right">' . number_format($row->sr_qty) . '</div>';
    $data['pr_qty'] = '<div class="uk-text-right">' . number_format($row->pr_qty) . '</div>';
    $data['stk_qty'] = '<div class="uk-text-right">' . number_format($row->stk_qty) . '</div>';
    $data['end_qty'] = '<div class="uk-text-right">' . number_format($end_qty) . '</div>';
    $data['cogafs'] = '<div class="uk-text-right">' . avbase_accounting_number_format($cogafs) . '</div>';
    $data['inventory'] = '<div class="uk-text-right">' . avbase_accounting_number_format($inventory) . '</div>';
    $data['cogs'] = '<div class="uk-text-right">' . avbase_accounting_number_format($cogs) . '</div>';

    // Careful when adding classes on rows, check if tableselect mode is ON.
    $table_rows[] = $data;
  }

  // Add "totals" row.
  if (!empty($table_rows)) {
    $totals_table_row = array();
    $colspan = array_search('cogafs', array_keys($header));
    if (!empty($colspan)) {
      $totals_table_row[] = array('data' => '<div class="uk-text-right">TOTAL</div>', 'colspan' => $colspan);
      $totals_table_row[] = '<div class="uk-text-right">' . avbase_accounting_number_format($tot_cogafs) . '</div>';
      $totals_table_row[] = '<div class="uk-text-right">' . avbase_accounting_number_format($tot_inventory) . '</div>';
      $totals_table_row[] = '<div class="uk-text-right">' . avbase_accounting_number_format($tot_cogs) . '</div>';
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


  $transaction_list_table['table'] = array(
    '#header' => $header,
    '#empty' => '<div class="uk-text-muted">' . t('No record found.') . '</div>',
    '#attributes' => array(
      'id' => 'transaction-list-table',
      'class' => array('uk-table-condensed uk-table-hover uk-text-small'),
    ),
  );
  $transaction_list_table['table']['#theme'] = 'table';
  $transaction_list_table['table']['#rows'] = $table_rows;



  if (1) {
    $transaction_list_table['mid_info']['#markup'] = '<div class="uk-text-large">' . format_date(strtotime('yesterday', $date_end), 'custom', 'F Y') . '</div>';
    $transaction_list_table['mid_info']['#markup'] .= t('<div>Total Inventory: <span class="uk-text-bold">@inventory</span></div>', array('@inventory' => avbase_accounting_number_format($tot_inventory)));
    $transaction_list_table['mid_info']['#markup'] .= t('<div>Total COGS: <span class="uk-text-bold">@cogs</span></div>', array('@cogs' => avbase_accounting_number_format($tot_cogs)));
    $transaction_list_table['other_info']['#markup'] = empty($transaction_lock_date)
      ? '<span class="uk-text-danger uk-text-bold"><i class="uk-icon-exclamation-triangle uk-margin-small-right"></i>Scan and Lock has never been run before.</span>'
      : t('<span class="uk-text-uppercase uk-text-bold uk-text-large">As of @date</span>', array('@date' => format_date($transaction_lock_date, 'custom', 'F d, Y')));
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
