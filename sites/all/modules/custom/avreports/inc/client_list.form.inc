<?php

function avreports_client_list_form($form, &$form_state, $client_type, $limit = NULL, $offset = NULL) {
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
    'export_button' => array(
      '#func_name' => __FUNCTION__ ,
      '#func_args' => array($client_type),
      '#report_name' => $ctd['name'] . ' List',
      '#report_inc' => 'client_list.form',
    ),
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
    $filter_form_settings['avclients.category_id'] = array('#options' => avbase_get_category_options_array('category' . $ctd['type']));
  }
  $form['transaction_list_filter'] = avtxns_txn_list_filter_form($form, $form_state, $filter_form_settings);

  // Prepare query variables.
  $select = $joins = $where = $args = $group_by = $table_rows = array();
  $user_filter = empty($form_state['values']['avtxns_tl_filter']) ? array() : $form_state['values']['avtxns_tl_filter'];
  $filter_query = empty($user_filter['filter_query']) ? array() : $user_filter['filter_query'];
  $where = empty($filter_query['where']) ? array() : $filter_query['where'];
  $args = empty($filter_query['args']) ? array() : $filter_query['args'];


  // Show hide columns depending on selected filters.
  $col_category_show = $col_category && empty($user_filter['avclients.category_id']);
  $col_agent_show = $col_agent && empty($user_filter['avcat_agent.title']);
  $col_area_show = $col_area && empty($user_filter['avcat_area.title']);

  $header = array();
  $header['#'] = '';
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
  $query = "SELECT _expression_ FROM {avtbl_clients} AS avclients " . implode(' ', $joins);
  // Count max rows before applying supplements to $query.
  avreports_count_max_rows($form, $query, $select, $where, $group_by, $args, $offset);
  avbase_query_supplements($query, $where, $order_by, $limit, $offset, $group_by);
  $rs = db_query($query, $args);
  $index = 0;
  foreach ($rs as $row) {
    $data = array();
    $data['#'] = ++$index;
    $data['display_name'] = l($row->display_name, $ctd['base_path'] . "/{$row->id}/view");
    if ($col_company) {
      //$data['company'] = $row->company;
    }
    //$data['first_name'] = $row->first_name;
    //$data['last_name'] = $row->last_name;
    if ($col_agent_show) {
      $data['agent'] = $row->agent_name;
    }
    //$data['email'] = $row->email;
    $data['contact_number'] = $row->contact_number;
    if ($col_category_show) {
      $data['category'] = $row->category_name;
    }
    if ($col_area_show) {
      $data['area'] = $row->area_name;
    }
    $data['address'] = $row->address;
    $data['city'] = $row->city;
    $data['province'] = $row->province;
    $data['zip_code'] = $row->zip_code;
    if ($col_payment) {
      $data['term'] = $row->term_name;
      //$data['payment_method'] = $row->payment_method_id;
    }
    if ($col_credit_limit) {
      $data['credit_limit'] = '<div class="uk-text-right">' . number_format($row->credit_limit, 2) . '</div>';
    }

    // Careful when adding classes on rows, check if tableselect mode is ON.
    $csd = avclients_client_status_details($row->status);
    if (!empty($csd['color_class'])) {
      $data['#avclass'] = $csd['color_class'];
    }

    $table_rows[] = $data;
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
  if (!empty($user_filter['avclients.category_id'])) {
    $transaction_list_table['avclients.category_id'] = array(
      '#type' => 'item',
      '#title' => 'Category',
      '#title_display' => 'inline-before',
      '#markup' => check_plain($filter_form_settings['avclients.category_id']['#options'][$user_filter['avclients.category_id']]),
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

  // Show / hide preview?
  avreports_report_live_preview($form);
  return $form;
}
