<?php

function avreports_export() {
  module_load_include('inc', 'phpexcel');

// Prepare the file path. The PHPExcel library doesn't handle PHP stream
// wrappers, so we need the real path.
  $wrapper = file_stream_wrapper_get_instance_by_uri('temporary://');
// Generate a file name. If it's unique, it's less likely to conflict with an
// existing file. You could also put up some more checks on this, if it's likely
// to conflict (like, when you have many export/download requests).
  $filename = AVREPORTS_EXPORT_FILE_PREFIX . uniqid() . '.xls';
  $filepath = $wrapper->realpath() . '/' . $filename;
// Export, and store to file.
  $result = phpexcel_export(array('Header 1', 'Header 2'), array(
    array('A1', 'B1'),
    array('A2', 'B2'),
  ), $filepath);

  if ($result === PHPEXCEL_SUCCESS) {
    // Exported successfully. Let's register the file with Drupal. We simply
    // tell Drupal to copy the file over the existing one, by passing in
    // temporary://$filename.
    $file = file_save_data(
      file_get_contents($filepath),
      "temporary://$filename",
      FILE_EXISTS_REPLACE
    );

    // By default, the file is stored as a permanent file. Let's make it
    // temporary, so Drupal will remove it (in 6 hours, if your cron is set up
    // correctly).
    $file->status = 0;
    file_save($file);

    // Start downloading. This requires a hook_file_download() implementation!
    $suggested_filename = 'uct-report.xls';
    $headers = array(
      'Content-type' => 'application/vnd.openxmlformats-officedocument.spreadsheetml.sheet',
      'Content-Disposition' => 'attachment; filename="' . $suggested_filename . '"',
    );
    file_transfer($file->uri, $headers);
  }
  else {
    // Error.
  }
}

function avreports_export_submit($form, &$form_state) {
  unset($form_state['values']['avtxns_tl_filter']['settings']['table_group_by']);
  avtxns_txn_list_filter_form_submit($form, $form_state, TRUE);

  $triggering_element = $form_state['triggering_element'];
  $report_name = $triggering_element['#report_name'];
  $report_args = empty($triggering_element['#report_args']) ? array() : $triggering_element['#report_args'];
  array_unshift($report_args, $form_state);

  $batch = array(
    'operations' => array(
      array('avreports_export_batch_process', array($report_name, $report_args)),
    ),
    'finished' => 'avreports_export_batch_finished',
    'title' => t('Exporting'),
    'error_message' => t('The export process encountered an error.'),
    'file' => drupal_get_path('module', 'phpexcel') . '/phpexcel.inc',
  );

  batch_set($batch);
}

function avreports_export_batch_process($report_name, $report_args, &$context) {
  module_load_include('inc', 'phpexcel');

  $limit = 5000;
  $offset = empty($context['sandbox']['progress']) ? 0 : $context['sandbox']['progress'];
  $report_args[] = $limit;
  $report_args[] = $offset;

  // Include report inc file.
  module_load_include('inc', 'avreports', "inc/{$report_name}.form");
  $table_func_name = "avreports_{$report_name}_table";
  $table = call_user_func_array($table_func_name, $report_args);


  $_header = empty($table['#header']) ? array() : $table['#header'];
  $header = array();
  foreach ($_header as $k => $v) {
    $header[] = is_array($v) ? $v['data'] : $v;
  }

  if (!isset($context['sandbox']['progress'])) {
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['max'] = empty($table['#rows_max']) ? 1 : $table['#rows_max'];

    // Store the file in the temporary directory.
    $wrapper = file_stream_wrapper_get_instance_by_uri('temporary://');
    $context['sandbox']['filename'] = AVREPORTS_EXPORT_FILE_PREFIX . uniqid() . '.xls';
    $context['sandbox']['file'] = $wrapper->realpath() . '/' . $context['sandbox']['filename'];

    // Prepare the Excel file.
    $result = phpexcel_export($header, array(
      // Provide some empty data. We will append data later on.
      array(),
    ), $context['sandbox']['file']);

    if ($result !== PHPEXCEL_SUCCESS) {
      drupal_set_message('Something went wrong with the export library.', 'error');
      $context['sandbox']['finished'] = 1;
      $context['success'] = FALSE;
      return;
    }

    // Exported successfully. Let's register the file with Drupal. We simply
    // tell Drupal to copy the file over the existing one, by passing in
    // temporary://$filename.
    $file = file_save_data(
      file_get_contents($context['sandbox']['file']),
      'temporary://' . $context['sandbox']['filename'],
      FILE_EXISTS_REPLACE
    );

    // By default, the file is stored as a permanent file. Let's make it
    // temporary, so Drupal will remove it (in 6 hours, if your cron is set up
    // correctly).
    $file->status = 0;
    file_save($file);


    // Trick to pass the filepath to the finished callback.
    $context['results'] = "temporary://{$context['sandbox']['filename']}";


  }

  $_rows = empty($table['#rows']) ? $table['#rows'] : $table['#rows'];
  $rows = array();
  if (empty($_rows)) {
    $context['finished'] = 1;
    return;
  }
  else {
    $row_index = 0;
    foreach ($_rows as $v) {
      foreach ($v as $v2) {
        $rows[$row_index][] = is_array($v2) ? strip_tags($v2['data']) : strip_tags($v2);
      }
      $row_index++;
    }
  }


  $result = phpexcel_export($header, $rows, $context['sandbox']['file'], array(
    // Use our previously prepared file as a "template", which means we
    // will append data to it, instead of starting from scratch again.
    'template' => $context['sandbox']['file'],
  ));

  if ($result !== PHPEXCEL_SUCCESS) {
    drupal_set_message(t("Something went wrong on pass !pass", array(
      '!pass' => $context['sandbox']['progress'],
    )), 'error');
    $context['sandbox']['finished'] = 1;
    $context['success'] = FALSE;
    return;
  }

  $context['sandbox']['progress'] += $limit;
  $context['message'] = t('Exported %progress of %max rows.', array('%progress' => number_format($context['sandbox']['progress']), '%max' => number_format($context['sandbox']['max'])));

  //while($limit) {number_format($context['sandbox']['max'])
  //  $result = phpexcel_export($header, array(
  //    // Append some data to the file.
  //    array('Some value', 'Some other value'),
  //  ), $context['sandbox']['file'], array(
  //    // Use our previously prepared file as a "template", which means we
  //    // will append data to it, instead of starting from scratch again.
  //    'template' => $context['sandbox']['file'],
  //  ));
  //
  //  if ($result !== PHPEXCEL_SUCCESS) {
  //    drupal_set_message(t("Something went wrong on pass !pass", array(
  //      '!pass' => $context['sandbox']['progress'],
  //    )), 'error');
  //    $context['sandbox']['finished'] = 1;
  //    $context['success'] = FALSE;
  //    return;
  //  }
  //
  //  $context['sandbox']['progress']++;
  //  $limit--;
  //}


$memory_usage = memory_get_peak_usage() / 1024 / 1024;
$memory_limit = (int) ini_get('memory_limit');
  $memory_usage_percentage = $memory_usage / $memory_limit * 100;

  dpm($memory_usage_percentage);
  if ($context['sandbox']['progress'] != $context['sandbox']['max']) {
    $context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['max'];
  }
}

function avreports_export_batch_finished($success, $results, $operations) {
  if ($success) {
    //$wrapper = file_stream_wrapper_get_instance_by_uri($results);
    //drupal_set_message(t("Download it here: !link", array(
    //  '!link' => l(
    //    $results, $wrapper->getExternalUrl(),
    //    array(
    //      'attributes' => array(
    //        'target' => '_blank',
    //        'onclick' => 'window.close()',
    //      )
    //    )
    //  ),
    //)), 'status', FALSE);
    //drupal_goto('av/reports/export');

    $suggested_filename = 'uct-report.xls';
    $headers = array(
      'Content-type' => 'application/vnd.openxmlformats-officedocument.spreadsheetml.sheet',
      'Content-Disposition' => 'attachment; filename="' . $suggested_filename . '"',
    );
    file_transfer($results, $headers);
  }
}
