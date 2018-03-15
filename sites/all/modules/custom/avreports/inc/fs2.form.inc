<?php

/**
 * Build an fs form.
 * @param $form
 * @param $form_state
 * @param $type
 *
 * @return mixed
 */
function avreports_fs_form($form, &$form_state, $type) {
  $dad = avtxns_txn_list_filter_date_auto_options();
  $dad_default = empty($dad['last_month']) ? array() : $dad['last_month'];
  $filter_form_settings = array(
    'date_auto' => array(
      '#title' => t('Dates'),
      '#default_value' => 'last_month',
    ),
    'date_from' => array(
      '#title' => t('Date from'),
      '#default_value' => $dad_default['date_from'],
    ),
    'date_to' => array(
      '#title' => t('Date to'),
      '#default_value' => $dad_default['date_to'],
    ),
    'reset_button' => array('#attributes' => array('style' => 'margin-top: 25px;')),
  );
  $form['transaction_list_filter'] = avtxns_txn_list_filter_form($form, $form_state, $filter_form_settings);
  $form['transaction_list_filter']['settings']['update_notice'] = array(
    '#markup' => '<div class="uk-text-right uk-text-muted uk-text-small">Last update: mm/dd/yy. ' . l('Update now', 'av/reports/fs-update-options', array('attributes' => array('class' => array('uk-button uk-button-mini')))) . '</div>',
  );
  return $form;
}

/**
 * FS Update Options form.
 * @param $form
 * @param $form_state
 *
 * @return array
 */
function avreports_fs_update_form($form, &$form_state) {
  $form['range'] = array(
    '#type' => 'radios',
    '#title' => 'Scan range',
    '#options' => array(
      'all' => 'Re-scan everything',
      'new' => 'Only scan new transactions',
    ),
    '#default_value' => 'new',
    '#attributes' => array(
      'class' => array('uk-button-group'),
    ),
    '#required' => TRUE,
    '#description' => 'To help generate reports quicker and more efficiently, the system has to <strong>scan and remember</strong> transaction history.
    <br />It may take some time to finish updating the system\'s <strong>memory</strong> depending on the number of transactions that need to be scanned.',
  );
  $form['buttons']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Begin scan'),
    '#icon_key' => 'search',
    '#attributes' => array('class' => array('uk-button-primary uk-margin-top')),
  );
  return $form;
}

/**
 * Submit handler for avreports_fs_update_form().
 */
function avreports_fs_update_form_submit($form, &$form_state) {
  $range = empty($form_state['values']['range']) ? NULL : $form_state['values']['range'];
  if (empty($range)) {
    return;
  }

  // Set start date.
  $query = "SELECT transaction_date FROM {avtbl_transactions} ORDER BY transaction_date ASC LIMIT 1";
  $date_start = db_query($query)->fetchField();
  if (empty($date_start)) {
    drupal_set_message('There are no transactions recorded yet.', 'warning');
    return;
  }
  $date_start = strtotime('today midnight', $date_start);
  $date_end = strtotime('tomorrow midnight');

  $operations = array();
  $query = "SELECT id, title, account_type FROM {avtbl_account_chart} ORDER BY id ASC";
  $rs = db_query($query);
  foreach ($rs as $account_row) {
    $operations[] = array('avreports_batch_scan_accounts', array($account_row, $date_start, $date_end));
  }
  if (empty($operations)) {
    drupal_set_message('Scan is not required yet.', 'warning');
    return;
  }

  $batch = array(
    'operations' => $operations,
    'finished' => 'avreports_batch_scan_finished',
    'title' => 'Scanning transactions',
    // 'init_message' => t('Example Batch is starting.'),
    // 'progress_message' => 'Scanning transaction history',
    'error_message' => t('Scan encountered an error'),
    'file' => AVREPORTS_MODULE_PATH . '/inc/fs2.form.inc',
  );
  batch_set($batch);


}

/**
 * Batch process to scan Accounts transaction history.
 * @param $account_row
 *
 * @throws Exception
 */
function avreports_batch_scan_accounts($account_row, $date_start, $date_end, &$context) {
  if (!isset($context['sandbox']['progress'])) {
    $date_diff = $date_end - $date_start;
    $context['sandbox']['max'] = round($date_diff / (60 * 60 * 24));
    $context['results'] = array();
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['date_start'] = $date_start;
  }

  // Pointers.
  $_results = &$context['results'];
  $_progress = &$context['sandbox']['progress'];
  $_max = &$context['sandbox']['max'];
  $_start = &$context['sandbox']['date_start'];

  // Set limit per iteration.
  $limit = 1;
  $remaining = $_max - $_progress;
  if ($limit > $remaining) {
    $limit = $remaining;
  }
  for ($i = 0; $i < $limit; $i++) {
    $_end = strtotime('tomorrow midnight', $_start);
    avtxns_test($account_row->id, $account_row->account_type, $_start, $_end);
    $_start = $_end;
  }

  // $query = "SELECT id, title FROM {avtbl_account_chart} WHERE id > :current_id AND id = 10 ORDER BY id ASC LIMIT $limit";
  // $args = array(':current_id' => $_current_id);
  // $rs = db_query($query, $args);
  //
  // //dpm($data);
  // $date_start = time();
  // $date_end = strtotime('tomorrow midnight', $date_start);
  // $date_start = 1517414400;
  // $date_end = 1519833600;
  // foreach ($rs as $account_row) {
  //   $_current_id = $account_row->id;
  //
  //   $_start = $date_start;
  //   while ($_start < $date_end) {
  //     $_end = strtotime('tomorrow midnight', $_start);
  //     avtxns_test($_current_id, $_start, $_end);
  //     $_start = $_end;
  //   }
  //
  // }

  // Update progress counter.
  $_progress += $limit;

  $context['message'] = t('Scanned @progress of @max transaction days for %title.', array('@progress' => $_progress, '@max' => $_max, '%title' => $account_row->title));
  $context['message'] .= '<div class="uk-text-muted uk-text-small">' . t('Earliest transaction on record: ' . format_date($date_start, 'custom', 'M d, Y')) . '</div>';
  // 'custom', 'M d, Y'

  // Compute for progress percentage.
  $finished = $_progress / $_max;
  if ($finished >= 1 && empty($_results['success'])) {
    $finished = $_progress / $_max;
  }
  // Set progress percentage.
  $context['finished'] = $finished;

  // $context['message'] = "Scanne";
}

/**
 * Callback when Transaction History scanning has finished.
 * @param $success
 * @param $results
 * @param $operations
 *
 * @throws Exception
 */
function avreports_batch_scan_finished($success, $results, $operations) {
  if ($success) {
    drupal_set_message('Scan has finished.');
  }
  else {
    drupal_set_message(t('Scan was interrupted.'), 'error');
  }
}
