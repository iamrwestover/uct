<?php
function avreports_stock_history_form($form, &$form_state) {
  $dad = avtxns_txn_list_filter_date_auto_options();
  $dad_default = empty($dad['this_week']) ? array() : $dad['this_week'];
  $filter_form_settings = array(
    'date_auto' => array(
      '#title' => t('Dates'),
      '#default_value' => 'this_week',
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
  $header['client_name'] = array('data' => 'CLIENT');
  $header['row_qty'] = array('data' => 'QTY.', 'class' => array('uk-text-right'));
  $header['cost'] = array('data' => 'COST', 'class' => array('uk-text-right'));
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

//  $xx = 0;
//  $r = array();
//  for ($j=0; $j < 10000000; $j++) {
//    $xx++;
//    $r['x'] = $xx;
//    $r['d'] = $xx;
//    $r['the_quick_brown'] = "Unbreak my heart.... say you love me again. Undo this hurt, that cause that you wall out of the door. And wallk out of my life.";
//    $r['the_quick_brown2'] = "Unbreak my heart.... say you love me again. Undo this hurt, that cause that you wall out of the door. And wallk out of my life.";
//    $r['the_quick_brown3'] = "Unbreak my heart.... say you love me again. Undo this hurt, that cause that you wall out of the door. And wallk out of my life.";
//    $r['the_quick_brown4'] = "Unbreak my heart.... say you love me again. Undo this hurt, that cause that you wall out of the door. And wallk out of my life.";
//    $r['the_quick_brown4'] = "Unbreak my heart.... say you love me again. Undo this hurt, that cause that you wall out of the door. And wallk out of my life.";
//    $r['the_quick_brown4'] = "Unbreak my heart.... say you love me again. Undo this hurt, that cause that you wall out of the door. And wallk out of my life.";
//    $r['the_quick_brown4'] = "Unbreak my heart.... say you love me again. Undo this hurt, that cause that you wall out of the door. And wallk out of my life.";
//    $r['the_quick_brown4'] = "Unbreak my heart.... say you love me again. Undo this hurt, that cause that you wall out of the door. And wallk out of my life.";
//    //$r['xs']['the_quick_brown'][] = "Unbreak my heart.... say you love me again. Undo this hurt, that cause that you wall out of the door. And wallk out of my life.";
//    //$r['xs']['the_quick_brown'][] = "Unbreak my heart.... say you love me again. Undo this hurt, that cause that you wall out of the door. And wallk out of my life.";
//    //$r['xs']['the_quick_brown'][] = "Unbreak my heart.... say you love me again. Undo this hurt, that cause that you wall out of the door. And wallk out of my life.";
//    //$r['xs']['the_quick_brown'][] = "Unbreak my heart.... say you love me again. Undo this hurt, that cause that you wall out of the door. And wallk out of my life.";
//    $r['the_quick_brow2n'] = "Uncry these tears I cried so many nights. Unbreak my heaaaaaartttt. heaaart..";
//  }
//  $jj = array();
//  for ($j=0; $j < 10000000; $j++) {
//    $xx++;
//    $jj['x'] = $xx;
//    $jj['d'] = $xx;
//    $jj['the_quick_brown'] = "Unbreak my heart.... say you love me again. Undo this hurt, that cause that you wall out of the door. And wallk out of my life.";
//    $jj['the_quick_brown2'] = "Unbreak my heart.... say you love me again. Undo this hurt, that cause that you wall out of the door. And wallk out of my life.";
//    $jj['the_quick_brown3'] = "Unbreak my heart.... say you love me again. Undo this hurt, that cause that you wall out of the door. And wallk out of my life.";
//    $jj['the_quick_brown4'] = "Unbreak my heart.... say you love me again. Undo this hurt, that cause that you wall out of the door. And wallk out of my life.";
//    $jj['the_quick_brown4'] = "Unbreak my heart.... say you love me again. Undo this hurt, that cause that you wall out of the door. And wallk out of my life.";
//    $jj['the_quick_brown4'] = "Unbreak my heart.... say you love me again. Undo this hurt, that cause that you wall out of the door. And wallk out of my life.";
//    $jj['the_quick_brown4'] = "Unbreak my heart.... say you love me again. Undo this hurt, that cause that you wall out of the door. And wallk out of my life.";
//    $jj['the_quick_brown4'] = "Unbreak my heart.... say you love me again. Undo this hurt, that cause that you wall out of the door. And wallk out of my life.";
//    //$jj['xs']['the_quick_brown'][] = "Unbreak my heart.... say you love me again. Undo this hurt, that cause that you wall out of the door. And wallk out of my life.";
//    //$jj['xs']['the_quick_brown'][] = "Unbreak my heart.... say you love me again. Undo this hurt, that cause that you wall out of the door. And wallk out of my life.";
//    //$jj['xs']['the_quick_brown'][] = "Unbreak my heart.... say you love me again. Undo this hurt, that cause that you wall out of the door. And wallk out of my life.";
//    //$jj['xs']['the_quick_brown'][] = "Unbreak my heart.... say you love me again. Undo this hurt, that cause that you wall out of the door. And wallk out of my life.";
//    $jj['the_quick_brow2n'] = "Uncry these tears I cried so many nights. Unbreak my heaaaaaartttt. heaaart..";
//  }
//dpm($xx);

  $table_rows = array();
  $selected_product_id = NULL;
  if (!empty($user_filter['avprod.title'])) {
    // QUERY that utilizes temporary table to get onhand qty per transaction.
//    db_query("create temporary table test (INDEX item_id (item_id), INDEX transaction_date (transaction_date), INDEX transaction_id (transaction_id), INDEX stest (transaction_date, transaction_id))
//
//SELECT avtxn.id as transaction_id, avtxn.transaction_date, avtxn.transaction_type, avtxn_detail.item_id,
//SUM((avtxn_detail.qty * avtxn_detail.qty_per_uom) * (IF(avtxn.transaction_type IN ('dr','pr','stou'), -1, 1))) as row_qty
//
//FROM avtbl_transaction_details AS avtxn_detail
//INNER JOIN avtbl_transactions AS avtxn ON avtxn.id = avtxn_detail.transaction_id AND avtxn.transaction_type IN ('dr','sr','rcv','pr','stk','stou','stin')
//INNER JOIN avtbl_products AS avprod ON avprod.id = avtxn_detail.item_id
//
//
//GROUP BY avtxn.id, avtxn_detail.item_id
//ORDER BY avtxn_detail.item_id ASC, avtxn.transaction_date ASC, avtxn.id ASC;");
//
//
//$xs = db_query("SELECT avtxn.id as transaction_id, avtxn.transaction_type, avtxn_detail.item_id,
//(avprod.initial_qty + IFNULL(SUM(s2.row_qty), 0)) AS onhand,
//SUM((avtxn_detail.qty * avtxn_detail.qty_per_uom) * (IF(avtxn.transaction_type IN ('dr','pr','stou'), -1, 1))) as row_qty
//
//FROM avtbl_transaction_details AS avtxn_detail
//INNER JOIN avtbl_transactions AS avtxn ON avtxn.id = avtxn_detail.transaction_id AND avtxn.transaction_type IN ('dr','sr','rcv','pr','stk','stou','stin')
//INNER JOIN avtbl_products AS avprod ON avprod.id = avtxn_detail.item_id
//
//LEFT JOIN test AS s2 ON s2.item_id = avtxn_detail.item_id AND (s2.transaction_date < avtxn.transaction_date OR (s2.transaction_date = avtxn.transaction_date AND s2.transaction_id < avtxn.id))
//
//
//GROUP BY avtxn.id, avtxn_detail.item_id
//ORDER BY avtxn_detail.item_id ASC, avtxn.transaction_date ASC, avtxn.id ASC;");

    $i_query = "SELECT avprod.id, avprod.title, initial_qty AS onhand,
      initial_value AS asset_value, initial_value/initial_qty AS ave_cost, initial_qty_date, avcat_uom.title AS uom_title
      FROM avtbl_products AS avprod
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
      $order_by[] = 'avtxn_detail.item_id ASC';
      $order_by[] = 'avtxn.transaction_date ASC';
      $order_by[] = 'avtxn.id ASC';
      $query = "SELECT " . implode(', ', $select) . " FROM {avtbl_transaction_details} AS avtxn_detail " . implode(' ', $joins);

      unset($where['date_from']);
      unset($args[':date_from_stamp']);
      avbase_query_supplements($query, $where, $order_by, NULL, NULL, $group_by);
      $rs = db_query($query, $args);

      $date_from = strtotime($user_filter['date_from']);

      // First row for initial values.
      if (!empty($prod_init[$selected_product_id]) && empty($table_rows[$selected_product_id . '_init'])) {
        $init = &$prod_init[$selected_product_id];
        if ($date_from <= $init->initial_qty_date) {
          $data = array();
          $data['transaction_type'] = array('data' => 'Added product');
          $data['transaction_date'] = array('data' => format_date($init->initial_qty_date, 'custom', 'M-d-Y'));
          $data['transaction_id'] = array('data' => '');
          $data['client_name'] = array('data' => '');
          $data['row_qty'] = array('data' => number_format($init->onhand), 'class' => array('uk-text-right'));
          $data['amount'] = array('data' => '', 'class' => array('uk-text-right'));
          //$data['price'] = array('data' => 'PRICE', 'class' => array('uk-text-right'));
          $data['onhand'] = array('data' => number_format($init->onhand), 'class' => array('uk-text-right'));
          $data['ave_cost'] = array('data' => number_format($init->ave_cost, 2), 'class' => array('uk-text-right'));
          $data['asset_value'] = array('data' => number_format($init->asset_value, 2), 'class' => array('uk-text-right'));
          // Careful when adding classes on rows, check if tableselect mode is ON.
          $table_rows[$selected_product_id . '_init'] = $data;
        }
      }
      foreach ($rs as $row) {
        if (empty($init)) {
          continue;
        }

        // Transaction values.
        $init->onhand += $row->row_qty;
        $init->ave_cost = in_array($row->transaction_type, $ave_cost_types) ? (((float)$row->row_cost + $init->asset_value) / $init->onhand) : $init->ave_cost;
        $init->asset_value = $init->ave_cost * $init->onhand;
        if ($date_from <= $row->transaction_date) {
          $ttd = avtxns_txn_types($row->transaction_type);
          $data = array();
          $txn_view_path = $ttd['base_path'] . "/{$row->transaction_id}/view";
          $data['transaction_type'] = array('data' => l($ttd['name'], $txn_view_path));
          $data['transaction_date'] = array('data' => l(format_date($row->transaction_date, 'custom', 'M-d-Y'), $txn_view_path));
          $data['transaction_id'] = array('data' => l(avbase_txn_number_format($row->transaction_type, $row->transaction_id), $txn_view_path));
          $data['client_name'] = array('data' => l($row->client_name, $ttd['ctd']['base_path'] . '/' . $row->client_id . '/view'));
          $data['row_qty'] = array('data' => number_format($row->row_qty), 'class' => array('uk-text-right'));
          $data['amount'] = array('data' => (is_null($row->row_cost) ? '' : number_format($row->row_cost, 2)), 'class' => array('uk-text-right'));
          //$data['price'] = array('data' => 'PRICE', 'class' => array('uk-text-right'));
          $data['onhand'] = array('data' => number_format($init->onhand), 'class' => array('uk-text-right'));
          $data['ave_cost'] = array('data' => number_format($init->ave_cost, 2), 'class' => array('uk-text-right'));
          $data['asset_value'] = array('data' => number_format($init->asset_value, 2), 'class' => array('uk-text-right'));

          // Careful when adding classes on rows, check if tableselect mode is ON.
          $table_rows[$row->transaction_id] = $data;
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
  if (!empty($selected_product_id)) {
    $transaction_list_table['product_name'] = array(
      '#type' => 'item',
      '#title_display' => 'inline-before',
      '#markup' => '<span class="uk-text-bold">' . l($user_filter['avprod.title'], "av/products/{$selected_product_id}/view") . '</span>',
    );
    if (!empty($init)) {
      $transaction_list_table['product_name']['#markup'] .= '<br />On Hand: ' . number_format($init->onhand);
      $transaction_list_table['product_name']['#markup'] .= empty($init->uom_title) ? '' : (' (' . check_plain($init->uom_title) . ')');
      $transaction_list_table['product_name']['#markup'] .= '<br />Asset Value: ' . number_format($init->asset_value, 2);
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
