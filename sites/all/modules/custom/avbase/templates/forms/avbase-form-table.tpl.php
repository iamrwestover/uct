<?php
$rows = $form['product_rows'];
$header = $form['#header'];

// Setup the structure to be rendered and returned.
$content = array(
  '#theme' => 'table',
  '#header' => $header,
  '#rows' => array(),
);

// Traverse each row.  @see element_chidren().
foreach (element_children($rows) as $row_index) {
  $table_row = array();

  // Traverse each column in the row.  @see element_children().
  foreach (element_children($rows[$row_index]) as $col_index) {
    // Render the column form element.
    $table_col = array();
    $table_col['data'] = drupal_render($rows[$row_index][$col_index]);
    if (!empty($rows[$row_index][$col_index]['#column_attributes'])) {
      $table_col += $rows[$row_index][$col_index]['#column_attributes'];
    }
    $table_row[] = $table_col;
  }

  // Add the row to the table.
  $table_row = array('data' => $table_row);
  if (!empty($rows[$row_index]['#row_attributes'])) {
    $table_row += $rows[$row_index]['#row_attributes'];
  }
  $content['#rows'][] = $table_row;
}

$content['#rows'][] = array(
  'data' => array(
    array(
    'data' => '&nbsp;',
    'colspan' => 10,
    )
  ),
  'id' => 'po-new-product-wrapper',
);

print drupal_render($content);
