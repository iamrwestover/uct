<?php

/**
 * Load chart of account details by id.
 *
 * @param $account_chart_id
 * @return object
 */
function avtxns_account_chart_load($account_chart_id) {
  if (empty($account_chart_id) || $account_chart_id == 'add') {
    return FALSE;
  }

  $data = &drupal_static('avbase', array());
  $_account_charts = &$data['account_charts'];
  if (isset($_account_charts[$account_chart_id])) {
    return $_account_charts[$account_chart_id];
  }

  $where = array('avachart.id = :account_chart_id');
  $args = array(':account_chart_id' => $account_chart_id);
  $rows = avtxns_account_chart_query_load($where, $args);
  $row = empty($rows[$account_chart_id]) ? FALSE : $rows[$account_chart_id];

  // Update static variable.
  $_account_charts[$account_chart_id] = $row;
  return $row;
}

/**
 * Load chart of account by ids.
 * @param array $ids
 *
 * @return array
 */
function avtxns_account_chart_load_ids($ids) {
  if (empty($ids)) {
    return FALSE;
  }

  $data = &drupal_static('avbase', array());
  $_account_charts = &$data['account_charts'];

  $return = array();
  $query_ids = array();
  foreach ($ids as $id) {
    if (!empty($id)) {
      if (isset($_account_charts[$id])) {
        $return[$id] = $_account_charts[$id];
      }
      else {
        $query_ids[] = $id;
      }
    }
  }

  if (!empty($query_ids)) {
    $where = array('avachart.id IN(:account_chart_ids)');
    $args = array(':account_chart_ids' => $query_ids);
    $rows = avtxns_account_chart_query_load($where, $args);
    if (!empty($rows)) {
      $return += $rows;
    }
  }

  return $return;
}

/**
 * Advanced chart of account load.
 * @param array $param_where
 * @param array $param_args
 * @param $limit_string
 *
 * @return array
 */
function avtxns_account_chart_query_load($param_where, $param_args = array(), $limit_string = '') {
  if (empty($param_where)) {
    return array();
  }

  // Load account_charts static variable.
  $avbase = &drupal_static('avbase', array());
  $avbase['account_charts'] = empty($avbase['account_charts']) ? array() : $avbase['account_charts'];
  $_account_charts = &$avbase['account_charts'];

  // Set / load static variable for this query.
  $data = &drupal_static(__FUNCTION__, array());
  $_data_id = $param_where;
  sort($_data_id);
  $data_id = md5(json_encode($_data_id) . json_encode($param_args));

  if (isset($data[$data_id])) {
    return $data[$data_id];
  }

  $query = "SELECT
    avachart.*, avachart_sub.title as parent_name
    FROM {avtbl_account_chart} avachart
    LEFT JOIN {avtbl_account_chart} avachart_sub ON avachart_sub.id = avachart.parent";
  if (!empty($param_where)) {
    $query .= " WHERE " . implode(' AND ', $param_where);
  }
  $query .= " ORDER BY avachart.title";
  if (!empty($limit_string)) {
    $query .= " LIMIT " . $limit_string;
  }

  $rs = db_query($query, $param_args);
  $rows = array();
  foreach ($rs as $row) {
    $rows[$row->id] = $row;
  }

  // Update static variables.
  $data[$data_id] = $rows;
  $_account_charts += $rows;

  return $rows;
}

/**
 * Return list of account chart types.
 * @param string $type
 *
 * @return array
 */
function avtxns_account_chart_types($type = NULL) {
  $ret = &drupal_static(__FUNCTION__, array());
  if (empty($ret)) {
    $ret[AVTXN_AC_TYPE_AR] = array(
      'type' => AVTXN_AC_TYPE_AR,
      'name' => 'Accounts Receivable',
      'fixed' => TRUE,
      'balance_type' => 'debit',
      'ctd' => avclients_client_types(AVCLIENT_TYPE_CUSTOMER),
    );
    $ret[AVTXN_AC_TYPE_AP] = array(
      'type' => AVTXN_AC_TYPE_AP,
      'name' => 'Accounts Payable',
      'fixed' => TRUE,
      'balance_type' => 'credit',
      'ctd' => avclients_client_types(AVCLIENT_TYPE_VENDOR),
    );
    $ret[AVTXN_AC_TYPE_CURRENT] = array(
      'type' => AVTXN_AC_TYPE_CURRENT,
      'name' => 'Current Asset',
      'balance_type' => 'debit',
    );
    $ret[AVTXN_AC_TYPE_FIXED] = array(
      'type' => AVTXN_AC_TYPE_FIXED,
      'name' => 'Fixed Asset',
      'balance_type' => 'debit',
    );
    $ret[AVTXN_AC_TYPE_COGS] = array(
      'type' => AVTXN_AC_TYPE_COGS,
      'name' => 'Cost of Goods Sold',
      'balance_type' => 'debit',
    );
    $ret[AVTXN_AC_TYPE_LIABILITY] = array(
      'type' => AVTXN_AC_TYPE_LIABILITY,
      'name' => 'Liability',
      'balance_type' => 'credit',
    );
    $ret[AVTXN_AC_TYPE_INCOME] = array(
      'type' => AVTXN_AC_TYPE_INCOME,
      'name' => 'Income',
      'balance_type' => 'credit',
    );
    $ret[AVTXN_AC_TYPE_EXPENSE] = array(
      'type' => AVTXN_AC_TYPE_EXPENSE,
      'name' => 'Expense',
      'balance_type' => 'debit',
    );
    $ret[AVTXN_AC_TYPE_CAPITAL] = array(
      'type' => AVTXN_AC_TYPE_CAPITAL,
      'name' => 'Capital',
      'balance_type' => 'credit',
    );
    $ret[AVTXN_AC_TYPE_OTHER_INCOME] = array(
      'type' => AVTXN_AC_TYPE_OTHER_INCOME,
      'name' => 'Other Income',
      'balance_type' => 'credit',
    );
  }

  if (isset($ret[$type])) {
    return $ret[$type];
  }
  elseif (is_null($type)) {
    return $ret;
  }
  return NULL;
}

/**
 * Return List of chart types for form #options use.
 */
function avtxns_account_chart_types_option_array() {
  $types = avtxns_account_chart_types();
  $ret = array();
  foreach ($types as $t_k => $t) {
    if (!empty($t['fixed'])) {
      continue;
    }
    $ret[$t_k] = $t['name'];
  }
  return $ret;
}

/**
 * Menu callback - retrieve a JSON object containing autocomplete suggestions for existing chart of accounts.
 * @param $account_type_string
 * @param string $string
 */
function avtxns_account_chart_autocomplete($account_type_string, $string = '') {
  $matches = array();
  $string = trim($string);
  if ($string) {
    $where_or = array(
      'avachart.title LIKE :string',
    );
    $where_or_str = '(' . implode(' OR ', $where_or) . ')';
    $args = array(':string' => '%' . db_like($string) . '%');
    $where = array($where_or_str);
    //$where[] = 'avachart.fixed <> 1';

    $account_types = explode('-', $account_type_string);
    $account_types = array_filter($account_types);
    if (!empty($account_types)) {
      $where[] = 'avachart.account_type IN(:account_types)';
      $args[':account_types'] = $account_types;
    }

    $rows = avtxns_account_chart_query_load($where, $args, '8');
    $rows_tree = avbase_rows_build_tree($rows, 0, FALSE);
    $matches = avtxns_account_chart_autocomplete_matches($rows_tree);
  }

  drupal_json_output($matches);
}

/**
 * Recursive function to build autocomplete matches with parent-children indentations.
 * @param $tree
 * @param int $depth
 *
 * @return array
 */
function avtxns_account_chart_autocomplete_matches($tree, $depth = 0) {
  $matches = array();
  foreach ($tree as $row) {
    $account_type = empty($row['account_type']) ? '' : $row['account_type'];
    if (in_array($account_type, array(AVTXN_AC_TYPE_AR, AVTXN_AC_TYPE_AP))) {
      continue;
    }

    $row = (object) $row;
    $acd = avtxns_account_chart_types($row->account_type);
    $depth_html = '';
    for ($i = 0; $i < $depth; $i++) {
      $depth_html .= '<span class="uk-margin-left"></span>';
    }
    $row_html = $depth_html . check_plain($row->title) . '<div class="uk-text-muted uk-text-small">' . $depth_html . check_plain($acd['name']) .  '</div>';
    $row_html .= '<div style="display: none;" id="av-row-id">' . $row->id . '</div>';
    $row_html .= '<div style="display: none;" id="av-row-json">' . drupal_json_encode($row) . '</div>';
    $matches[$row->title] = $row_html;

    if (!empty($row->children)) {
      $matches += avtxns_account_chart_autocomplete_matches($row->children, $depth + 1);
    }
  }
  return $matches;
}

/**
 * Validate account chart parent element.
 * @param $element
 * @param $form_state
 */
function avtxns_element_validate_account_chart_name($element, &$form_state) {
  $element_value = $element['#value'];
  if (trim($element_value) == '') {
    return;
  }

  $this_row_id = empty($form_state['values']['id']) ? NULL : $form_state['values']['id'];
  $this_account_type = empty($form_state['values']['account_type']) ? NULL : $form_state['values']['account_type'];
  $query = "SELECT id, account_type, parent FROM {avtbl_account_chart} WHERE title = :title";
  $row = db_query($query, array(':title' => $element_value))->fetch();
  if (empty($row->id)) {
    form_error($element, t('@field_title doesn\'t exist.', array('@field_title' => $element['#title'])));
  }
  elseif (!empty($element['#parent_chart_of_account']) && $row->id == $this_row_id) {
    form_error($element, t('Cannot select self as @field_title.', array('@field_title' => $element['#title'])));
  }
  elseif (!empty($element['#parent_chart_of_account']) && !empty($row->parent)) {
    form_error($element, t('Cannot select a sub-account as @field_title.', array('@field_title' => $element['#title'])));
  }
  elseif (!empty($element['#parent_chart_of_account']) && $this_account_type != $row->account_type) {
    form_error($element, t('@field_title account type must be the same as this account\'s type.', array('@field_title' => $element['#title'])));
  }
  else if (!empty($element['#exclude_types']) && in_array($row->account_type, $element['#exclude_types'])) {
    form_error($element, t('Cannot accept this type of account.'));
  }
  else {

    // Set extra form state value for this element.
    avbase_element_set_form_state_extra_value($form_state, $element, $row->id);

    // Update form_state storage item_rows.
    avtrans_item_list_update_item_row($form_state, $element, $row->id);
  }
}

/**
 * Change chart of account status.
 * @param $id
 * @param $status
 *
 * @return bool
 */
function avtxns_account_chart_change_status($id, $status) {
  if (empty($id) || !is_numeric($id)) {
    return FALSE;
  }
  if (!array_key_exists($status, array(AVBASE_RECORD_ACTIVE, AVBASE_RECORD_DISABLED))) {
    return FALSE;
  }

  if (db_query("UPDATE {avtbl_account_chart} SET status = :status WHERE id = :record_id", array(':record_id' => $id, ':status' => $status))) {
    avbase_custom_watchdog("achart__status_change", "ID: {$id}", array('#status' => $status));
    return TRUE;
  }
  return FALSE;
}
