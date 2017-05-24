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
  dpm($form_state);
}