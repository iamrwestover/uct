<?php

function avreports_client_list_form($form, &$form_state, $client_type) {
  $ctd = avclients_client_types($client_type);
  if (empty($ctd['type'])) {
    drupal_not_found();
    drupal_exit();
  }

  $col_payment = empty($ctd['no_payment']);
  $col_category = empty($ctd['no_category']);
  $col_agent = empty($ctd['no_agent']);
  $col_company = empty($ctd['no_company']);
  $col_area = empty($ctd['no_area']);
  $col_credit_limit = $client_type == AVCLIENT_TYPE_CUSTOMER;

    // Filter form.
  $filter_form_settings = array(
    'reset_button' => array('#attributes' => array('style' => 'margin-top: 25px;')),
  );
  $filter_form_settings['client_status'] = array(
    '#options' => array(
      AVCLIENT_STATUS_ACTIVE . '-' . AVCLIENT_STATUS_SUSPENDED => 'All except disabled',
      AVCLIENT_STATUS_ACTIVE  => 'Enabled only',
      AVCLIENT_STATUS_DISABLED  => 'Disabled only',
    ),
    '#default_value' => AVCLIENT_STATUS_ACTIVE . '-' . AVCLIENT_STATUS_SUSPENDED,
  );
  if ($client_type == AVCLIENT_TYPE_CUSTOMER) {
    $filter_form_settings['client_status']['#options'][AVCLIENT_STATUS_SUSPENDED] = 'Suspended only';
  }
  if ($col_area) {
    $filter_form_settings['avcat_area.title'] = array('#title' => 'Area');
  }
  if ($col_agent) {
    $filter_form_settings['avcat_agent.title'] = array(
      '#title' => 'Agent',
      '#autocomplete_path' => 'av/categories/agent' . $ctd['type'] . '/autocomplete',
    );
  }
  if ($col_category) {
    $filter_form_settings['category_id'] = array('#options' => avbase_get_category_options_array('category' . $ctd['type']));
  }
  $form['transaction_list_filter'] = avtxns_txn_list_filter_form($form, $form_state, $filter_form_settings);

  // Prepare query variables.
  $select = $joins = $where = $args = $group_by = $table_rows = array();
  $user_filter = empty($form_state['values']['avtxns_tl_filter']) ? array() : $form_state['values']['avtxns_tl_filter'];
  $filter_query = empty($user_filter['filter_query']) ? array() : $user_filter['filter_query'];
  $where = empty($filter_query['where']) ? array() : $filter_query['where'];
  $args = empty($filter_query['args']) ? array() : $filter_query['args'];


  // Show hide columns depending on selected filters.
  $col_category_show = $col_category && empty($user_filter['category_id']);
  $col_agent_show = $col_agent && empty($user_filter['avcat_agent.title']);
  $col_area_show = $col_area && empty($user_filter['avcat_area.title']);

  $header = array();
  $header['display_name'] = array('data' => strtoupper($ctd['name'] . ' name'), 'field' => 'display_name');
  if ($col_company) {
    //$header['company'] = array('data' => 'COMPANY', 'field' => 'company');
  }
  //$header['first_name'] = array('data' => 'FIRST NAME', 'field' => 'display_name');
  //$header['last_name'] = array('data' => 'LAST NAME', 'field' => 'display_name');
  if ($col_agent_show) {
    $header['agent'] = array('data' => 'AGENT NAME', 'field' => 'avcat_agent.title');
  }
  //$header['email'] = array('data' => 'EMAIL', 'field' => 'display_name');
  $header['contact_number'] = array('data' => 'CONTACT NUMBER');
  if ($col_category_show) {
    $header['category'] = array('data' => 'CATEGORY', 'field' => 'avcat_cat.title');
  }
  if ($col_area_show) {
    $header['area'] = array('data' => 'AREA', 'field' => 'avcat_area.title');
  }
  $header['address'] = array('data' => 'ADDRESS');
  $header['city'] = array('data' => 'CITY', 'field' => 'city');
  $header['province'] = array('data' => 'PROVINCE', 'field' => 'province');
  $header['zip_code'] = array('data' => 'ZIP CODE', 'field' => 'zip_code');
  if ($col_payment) {
    $header['term'] = array('data' => 'TERM', 'field' => 'avcat_term.title');
    //$header['payment_method'] = array('data' => 'PAYMENT METHOD', 'field' => 'display_name');
  }
  if ($col_credit_limit) {
    $header['credit_limit'] = array('data' => 'CREDIT LIMIT', 'class' => array('uk-text-right'), 'field' => 'credit_limit');
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

  // Set default order.
  if (empty($_GET['order'])) {
    $_GET['order'] = $header['display_name']['data'];
    $_GET['sort'] = 'ASC';
  }
  $order_by = avbase_build_header_order_array($header);


  // Build query.
  $select[] = 'avclients.*';
  $select[] = 'avcat_agent.title AS agent_name';
  $select[] = 'avcat_cat.title AS category_name';
  $select[] = 'avcat_area.title AS area_name';
  $select[] = 'avcat_term.title AS term_name';
  $joins[] = 'LEFT JOIN {avtbl_categories} AS avcat_agent ON avcat_agent.id = avclients.agent_id';
  $joins[] = 'LEFT JOIN {avtbl_categories} AS avcat_cat ON avcat_cat.id = avclients.category_id';
  $joins[] = 'LEFT JOIN {avtbl_categories} AS avcat_area ON avcat_area.id = avclients.area_id';
  $joins[] = 'LEFT JOIN {avtbl_categories} AS avcat_term ON avcat_term.id = avclients.term_id';

  $where[] = 'avclients.client_type = :ctype';
  $args[':ctype'] = $client_type;
  $query = "SELECT " . implode(', ', $select) . " FROM {avtbl_clients} AS avclients " . implode(' ', $joins);
  avbase_query_supplements($query, $where, $order_by, NULL, NULL, $group_by);
  $rs = db_query($query, $args);
  foreach ($rs as $row) {
    $data = array();
    $data['display_name'] = array('data' => l($row->display_name, $ctd['base_path'] . "/{$row->id}/view"));
    if ($col_company) {
      //$data['company'] = array('data' => $row->company);
    }
    //$data['first_name'] = array('data' => $row->first_name);
    //$data['last_name'] = array('data' => $row->last_name);
    if ($col_agent_show) {
      $data['agent'] = array('data' => $row->agent_name);
    }
    //$data['email'] = array('data' => $row->email);
    $data['contact_number'] = array('data' => $row->contact_number);
    if ($col_category_show) {
      $data['category'] = array('data' => $row->category_name);
    }
    if ($col_area_show) {
      $data['area'] = array('data' => $row->area_name);
    }
    $data['address'] = array('data' => $row->address);
    $data['city'] = array('data' => $row->city);
    $data['province'] = array('data' => $row->province);
    $data['zip_code'] = array('data' => $row->zip_code);
    if ($col_payment) {
      $data['term'] = array('data' => $row->term_name);
      //$data['payment_method'] = array('data' => $row->payment_method_id);
    }
    if ($col_credit_limit) {
      $data['credit_limit'] = array('data' => number_format($row->credit_limit, 2), 'class' => array('uk-text-right'));
    }

    // Careful when adding classes on rows, check if tableselect mode is ON.
    $row_class = array();
    $csd = avclients_client_status_details($row->status);
    if (!empty($csd['color_class'])) {
      $row_class[] = $csd['color_class'];
    }
    $table_rows[] = array('data' => $data, 'class' => $row_class);
  }

  $transaction_list_table['table'] = array(
    '#header' => $header,
    '#empty' => '<div class="uk-text-muted">' . t('No match found.') . '</div>',
    '#attributes' => array(
      'id' => 'transaction-list-table',
      'class' => array('uk-table-condensed uk-table-hover uk-text-small'),
    ),
  );
  $transaction_list_table['table']['#theme'] = 'table';
  $transaction_list_table['table']['#rows'] = $table_rows;


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
  if (!empty($user_filter['category_id'])) {
    $transaction_list_table['category_id'] = array(
      '#type' => 'item',
      '#title' => 'Category',
      '#title_display' => 'inline-before',
      '#markup' => check_plain($filter_form_settings['category_id']['#options'][$user_filter['category_id']]),
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