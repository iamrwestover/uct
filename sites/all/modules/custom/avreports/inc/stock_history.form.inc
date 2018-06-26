<?php
function avreports_stock_history_form($form, &$form_state) {
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
      //'#default_value' => '3',
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



  $header = array();
  //$header['product_title'] = array('data' => 'PRODUCT', 'field' => 'avprod.title');
  $header['transaction_type'] = array('data' => 'TYPE');
  $header['transaction_date'] = array('data' => 'DATE');
  $header['transaction_id'] = array('data' => 'TXN. #');
  $header['reference_id'] = array('data' => 'REF. ID');
  $header['client_name'] = array('data' => 'CLIENT');
  $header['row_qty'] = array('data' => 'QTY.', 'class' => array('uk-text-right'));
  $header['cost'] = array('data' => 'TOTAL', 'class' => array('uk-text-right'));
  //$header['price'] = array('data' => 'PRICE', 'class' => array('uk-text-right'));
  $header['onhand'] = array('data' => 'ON HAND', 'class' => array('uk-text-right'));
  $header['ave_cost'] = array('data' => 'AVG COST', 'class' => array('uk-text-right'));
  $header['asset_value'] = array('data' => 'ASSET VALUE', 'class' => array('uk-text-right'));

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
    $_GET['order'] = $header['transaction_date']['data'];
    $_GET['sort'] = 'ASC';
  }
  $order_by = avbase_build_header_order_array($header);

  // Supporting order by if ordered by On Hand.
  if (isset($order_by['avtxn.transaction_date'])) {
    $order_by['avtxn.id'] = 'avtxn.id ASC';
  }


  $table_rows = array();
  $selected_product_id = NULL;
  if (!empty($user_filter['avprod.title'])) {
    $i_query = "SELECT avprod.id, avprod.title, initial_qty AS onhand, qty AS current_onhand,
      initial_value AS asset_value,
      initial_value AS ave_cost,
      initial_qty_date,
      avclients.display_name AS vendor_name,
      avcat_uom.title AS uom_title
      FROM avtbl_products AS avprod
      LEFT JOIN avtbl_clients AS avclients ON avclients.id = avprod.vendor_id
      LEFT JOIN avtbl_categories AS avcat_uom ON avcat_uom.id = avprod.uom_id
      WHERE avprod.title = (:prod_title)";
    $i_rs = db_query($i_query, array(':prod_title' => $user_filter['avprod.title']));
    $prod_init = array();
    foreach ($i_rs as $row) {
      $prod_init[$row->id] = $row;
      if (strtoupper($user_filter['avprod.title']) == strtoupper($row->title)) {
        $selected_product_id = $row->id;
      }
    }

    if (!empty($prod_init)) {
      $ave_cost_types = array(AVTXNS_TXN_TYPE_RECEIVE, AVTXNS_TXN_TYPE_STOCK_TRANSFER_IN);
      $select = array();
      $select[] = 'avtxn_detail.item_id';
      $select[] = 'SUM(avtxn_detail.qty * avtxn_detail.qty_per_uom * (IF(avtxn.transaction_type IN (:neg_ttypes), -1, 1))) AS row_qty';
      $select[] = 'SUM(IF(avtxn.transaction_type IN (:ave_cost_ttypes), (avtxn_detail.qty * avtxn_detail.cost), NULL)) AS row_cost';
      $select[] = 'avtxn.id AS transaction_id';
      $select[] = 'avtxn.transaction_date';
      $select[] = 'avtxn.transaction_type';
      $select[] = 'avtxn.client_id';
      $select[] = 'avtxn.reference_id';
      $joins[] = "INNER JOIN {avtbl_transactions} AS avtxn ON avtxn.id = avtxn_detail.transaction_id AND avtxn.transaction_type IN (:ttypes)";

      //$select[] = 'avprod.title AS product_title';
      $joins[] = 'LEFT JOIN {avtbl_products} AS avprod ON avprod.id = avtxn_detail.item_id';

      $select[] = 'avclient.display_name AS client_name';
      $joins[] = 'LEFT JOIN {avtbl_clients} AS avclient ON avclient.id = avtxn.client_id';

      //$where[] = 'avprod.title.item_id = :prod_title';
      $where[] = 'avtxn.status <> ' . AVTXNS_TXN_STATUS_VOID;
      $args[':ttypes'] = avtxns_get_qty_change_txn_types();
      $args[':neg_ttypes'] = avtxns_get_qty_change_txn_types('decrement');
      $args[':ave_cost_ttypes'] = $ave_cost_types;
      //$args[':ave_cost_ttypes'] = $user_filter['avprod.title'];
      $group_by = array();
      $group_by[] = 'avtxn.id';
      $group_by[] = 'avtxn_detail.item_id';
      $order_by = array();
      // $order_by[] = 'avtxn_detail.item_id ASC';
      $order_by[] = 'avtxn.transaction_date';
      $order_by[] = 'avtxn.id';
      $query = "SELECT " . implode(', ', $select) . " FROM {avtbl_transaction_details} AS avtxn_detail " . implode(' ', $joins);

      unset($where['date_from']);
      unset($where['avtxn.transaction_type']);
      unset($args[':date_from_stamp']);
      unset($args[':avtxntransaction_type']);
      // dpm($order_by);
      // $order_by = array('avtxn.id');
      avbase_query_supplements($query, $where, $order_by, NULL, NULL, $group_by);
      $rs = db_query($query, $args);

      $date_from = strtotime($user_filter['date_from']);

      //$spacer_data = array('data' => array('-' => array('data' => NULL, 'colspan' => count($header))), 'class' => array('uk-alert-mute'));
      //$after_skips = FALSE;
      // First row for initial values.
      if (!empty($prod_init[$selected_product_id]) && empty($table_rows[$selected_product_id . '_init'])) {
        $init = &$prod_init[$selected_product_id];
        if ($date_from <= $init->initial_qty_date) {
          $data = array();
          $data['transaction_type'] = '<span class="">Added Product</span>';
          $data['transaction_date'] = format_date($init->initial_qty_date, 'custom', AVREPORTS_LIST_DATE_FORMAT);
          $data['transaction_id'] = NULL;
          $data['reference_id'] = NULL;
          $data['client_name'] = NULL;
          $data['row_qty'] = '<div class="uk-text-right">' . number_format($init->onhand) . '</div>';
          $data['amount'] = NULL;
          $data['onhand'] = '<div class="uk-text-right">' . number_format($init->onhand) . '</div>';
          $data['ave_cost'] = '<div class="uk-text-right">' . number_format($init->ave_cost, 2) . '</div>';
          $data['asset_value'] = '<div class="uk-text-right">' . number_format($init->asset_value, 2) . '</div>';
          // Careful when adding classes on rows, check if tableselect mode is ON.
          $table_rows[$selected_product_id . '_init'] = $data;
        }
        //else {
        //  $table_rows['before_skips'] = $spacer_data;
        //}
      }

      foreach ($rs as $row) {
        if (empty($init)) {
          continue;
        }


        // Transaction values.
        $init->onhand += $row->row_qty;
        $init->ave_cost = in_array($row->transaction_type, $ave_cost_types) ? (((float)$row->row_cost + $init->asset_value) / $init->onhand) : $init->ave_cost;
        $init->asset_value = $init->ave_cost * $init->onhand;

        // Only show selected transaction type.
        if (!empty($user_filter['avtxn.transaction_type']) && $row->transaction_type != $user_filter['avtxn.transaction_type']) {
          //if (!isset($table_rows['before_skips'])) {
          //  $table_rows['before_skips'] = $spacer_data;
          //}
          //else {
          //  $after_skips = TRUE;
          //}
          continue;
        }

        if ($date_from <= $row->transaction_date) {
          $ttd = avtxns_txn_types($row->transaction_type);
          $data = array();
          $txn_view_path = $ttd['base_path'] . "/{$row->transaction_id}/view";
          // $data['transaction_type'] = l($ttd['name'], $txn_view_path);
          $data['transaction_type'] = $ttd['name'];
          $data['transaction_date'] = format_date($row->transaction_date, 'custom', AVREPORTS_LIST_DATE_FORMAT);
          $data['transaction_id'] = l(avbase_txn_number_format($row->transaction_type, $row->transaction_id), $txn_view_path);
          $data['reference_id'] = empty($row->reference_id) ? NULL : check_plain($row->reference_id);
          $data['client_name'] = empty($ttd['ctd']) ? NULL : l($row->client_name, $ttd['ctd']['base_path'] . '/' . $row->client_id . '/view');
          $data['row_qty'] = '<div class="uk-text-right">' . number_format($row->row_qty) . '</div>';
          $data['amount'] = is_null($row->row_cost) ? NULL : '<div class="uk-text-right">' . number_format($row->row_cost, 2) . '</div>';
          $data['onhand'] = '<div class="uk-text-right">' . number_format($init->onhand) . '</div>';
          $data['ave_cost'] = '<div class="uk-text-right">' . number_format($init->ave_cost, 2) . '</div>';
          $data['asset_value'] = '<div class="uk-text-right">' . number_format($init->asset_value, 2) . '</div>';

          // Careful when adding classes on rows, check if tableselect mode is ON.
          $table_rows[$row->transaction_id] = $data;
          //$after_skips = FALSE;
        }
      }
    }
  }

  // Highlight final onhand qty.
  //if ($after_skips) {
  //  $table_rows['_spacer'] = $spacer_data;
  //}
  //else {
  //  end($table_rows);
  //  $tr_last_visible_key = key($table_rows);
  //  $table_rows[$tr_last_visible_key]['onhand']['data'] = '<div class="uk-text-bold">' . $table_rows[$tr_last_visible_key]['onhand']['data'] . '</div>';
  //}



  $transaction_list_table['table'] = array(
    '#header' => $header,
    '#empty' => '<div class="uk-text-muted">' . (empty($user_filter['avprod.title']) ? 'Input product name to view its history.' : t('No match found.')) . '</div>',
    '#attributes' => array(
      'id' => 'transaction-list-table',
      'class' => array('uk-table-condensed uk-table-hover uk-text-small' /*. (strtotime($user_filter['date_to']) < strtotime(format_date(time(), 'custom', 'F d, Y')) ? ' uk-alert-warning' : '')*/),
    ),
  );

  $transaction_list_table['table']['#theme'] = 'table';
  $transaction_list_table['table']['#rows'] = $table_rows;

  avtxns_txn_list_table_add_date_from($transaction_list_table, $user_filter);
  if (!empty($selected_product_id)) {
    $transaction_list_table['product_name'] = array(
      '#type' => 'item',
      '#title_display' => 'inline-before',
      '#markup' => '<span class="uk-text-bold">' . l($user_filter['avprod.title'], "av/products/{$selected_product_id}/view") . '</span>',
    );
    if (!empty($init)) {
      $transaction_list_table['product_name']['#markup'] .= '<br />Preferred Vendor: ' . check_plain($init->vendor_name);
      $transaction_list_table['product_name']['#markup'] .= '<br />On Hand: ' . number_format($init->onhand);
      $transaction_list_table['product_name']['#markup'] .= empty($init->uom_title) ? '' : (' (' . check_plain($init->uom_title) . ')');
      $transaction_list_table['product_name']['#markup'] .= '<br />Asset Value: ' . number_format($init->asset_value, 2);

      // Update qty.
      if ($init->current_onhand != $init->onhand) {
        db_query("UPDATE {avtbl_products} SET qty = :qty WHERE id = :id", array(':qty' => $init->onhand, ':id' => $init->id));
      }
    }
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
