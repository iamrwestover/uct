<?php

function avreports_stock_history_summary_form($form, &$form_state) {
  ini_set('memory_limit', AVBASE_EXPANDED_MEMORY_LIMIT);
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
  // Group by.
  $group_by = array();
  $group_by[] = 'avprod.id';
  // $group_by[] = 'avtxn_detail.item_id';
  // Order by.
  $order_by = array();
  // $order_by[] = 'avtxn.transaction_date';
  // $order_by[] = 'avtxn.id';

  // Header.
  $header = array();
  $header['product_title'] = array('data' => 'TITLE', 'field' => 'title');
  $header['onhand'] = array('data' => 'ON HAND', 'class' => array('uk-text-right'), 'field' => 'onhand');
  $header['item_cost'] = array('data' => 'UNIT COST', 'class' => array('uk-text-right'), 'field' => 'item_cost');
  $header['inventory'] = array('data' => 'ENDING INVENTORY', 'class' => array('uk-text-right'), 'field' => 'inventory');
  $header['cogs'] = array('data' => 'COGS', 'class' => array('uk-text-right'), 'field' => 'cogs');

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

  $select[] = 'avprod.id';
  $select[] = 'avprod.title';
  $select[] = 'avinvcache.item_cost';
  $select[] = '(avinvcache.qty_in - avinvcache.qty_out) onhand';
  $select[] = '(avinvcache.item_cost * (avinvcache.qty_in - avinvcache.qty_out)) inventory';
  $select[] = '(avinvcache.qty_in_amount - (avinvcache.item_cost * (avinvcache.qty_in - avinvcache.qty_out))) cogs';
  $joins[] = 'LEFT JOIN {avtbl_inventory_cache} avinvcache ON avinvcache.product_id = avprod.id AND avinvcache.before_this_day = (SELECT ayay.before_this_day FROM {avtbl_inventory_cache} ayay WHERE ayay.before_this_day <= :date_to_stamp AND ayay.product_id = avprod.id ORDER BY before_this_day DESC LIMIT 1)';
  // $joins[] = 'LEFT JOIN {avtbl_inventory_cache} avinvcache ON avinvcache.product_id = avprod.id AND avinvcache.before_this_day = :date_to_stamp';

  // $where[] = 'avprod.id=2209';
  unset($where['date_to']);

  // Build query.
  $query = "SELECT " . implode(', ', $select) . " FROM {avtbl_products} AS avprod " . implode(' ', $joins);
  avbase_query_supplements($query, $where, $order_by, NULL, NULL, $group_by);
  // dpm($where);
  // dpm($query);
  // dpm($args);
  $rs = db_query($query, $args);
  $tot_inventory = $tot_cogs = 0;
  $i=0;
  foreach ($rs as $row) {
    $tot_inventory += $row->inventory;
    $tot_cogs += $row->cogs;
    $data = array();
    $data['product_title'] = avreports_product_history_link(
      $row->title, array(
        'date_from' => format_date(strtotime('first day of this month - 1 month', $date_end), 'custom', AVBASE_DATE_FORMAT),
        'date_to' => format_date(strtotime('last day of this month - 1 month', $date_end), 'custom', AVBASE_DATE_FORMAT),
      )
    );
    $data['onhand'] = '<div class="uk-text-right">' . number_format($row->onhand) . '</div>';

    //////////////////////
    /// @todo remove this
    // $db_qty = db_query("SELECT qty FROM avtbl_products WHERE id = :id", array(':id' => $row->id))->fetchField();
    // $data['onhand'] = $db_qty . $data['onhand'];
    // $db_qty = (int) $db_qty;
    // $row->onhand = (int) $row->onhand;
    // if ($row->onhand == $db_qty) continue;
    // $data['#avclass'] = $db_qty > $row->onhand ? 'uk-alert-danger' : ($db_qty < $row->onhand ? 'uk-alert-success' : '');
    ///////////////////////

    $data['item_cost'] = '<div class="uk-text-right">' . number_format($row->item_cost, 2) . '</div>';
    $data['inventory'] = '<div class="uk-text-right">' . avbase_accounting_number_format($row->inventory) . '</div>';
    $data['cogs'] = '<div class="uk-text-right">' . avbase_accounting_number_format($row->cogs) . '</div>';

    // $data['sales_cost'] = in_array($row->transaction_type, $fixed_cost_types) ? NULL : '<div class="uk-text-right">' . avbase_accounting_number_format($total_cost) . '</div>';
    // $data['ave_cost'] = in_array($row->transaction_type, $fixed_cost_types) ? NULL : '<div class="uk-text-right">' . number_format($item_cost, 2) . '</div>';
    // $data['asset_value'] = '<div class="uk-text-right">' . avbase_accounting_number_format($last_value) . '</div>';

    // Careful when adding classes on rows, check if tableselect mode is ON.
    $table_rows[] = $data;
  }

  // Add "totals" row.
  if (!empty($table_rows)) {
    $totals_table_row = array();
    $colspan = array_search('inventory', array_keys($header));
    if (!empty($colspan)) {
      $totals_table_row[] = array('data' => '<div class="uk-text-right">TOTAL</div>', 'colspan' => $colspan);
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
