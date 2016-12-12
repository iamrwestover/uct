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


// Sample Form prep for form-table theme.
//$form['product_table'] = array(
//  '#theme' => 'avbase_form_table',
//  '#header' => array(
//    array('data' => '#'),
//    array('data' => 'Product'),
//    array('data' => 'Description'),
//    array('data' => 'Qty.'),
//    array('data' => 'Price'),
//    array('data' => 'Amount'),
//    '&nbsp;',
//  ),
//  'product_rows' => array('#tree' => TRUE),
//);
//$prod_index = -1;
//foreach ($prod_rows as $prod_key => $prod_row) {
//  $prod_index++;
//  $table_row = array();
//
//  $table_row[] = array(
//    'row_id' => array(
//      '#markup' => 'a',
//    )
//  );
//
//  $table_row[] = array(
//    'prod_id' => array(
//      '#id' => 'prod-id-' . $prod_index,
//      '#type' => 'textfield',
//      '#title' => 'Product',
//      '#title_display' => 'invisible',
//      //'#description' => t('@uom1 per @uom2', array('@uom1' => $plural_form, '@uom2' => $current_uom_name)),
//      //'#default_value' => isset($prod_row['qty']) ? $prod_row['qty'] : '',
//      '#maxlength' => 255,
//      //'#element_validate' => array('element_validate_integer_positive'),
//      '#required' => TRUE,
//      '#attributes' => array(
//        //'data-prod-index' => $prod_index,
//        'class' => array('prod-group-id'),
//      ),
//    )
//  );
//
//  $table_row[] = array(
//    'description' => array(
//      '#id' => 'prod-desc-' . $prod_index,
//      '#type' => 'textfield',
//      '#title' => 'Description',
//      '#title_display' => 'invisible',
//      //'#description' => t('@uom1 per @uom2', array('@uom1' => $plural_form, '@uom2' => $current_uom_name)),
//      //'#default_value' => isset($prod_row['qty']) ? $prod_row['qty'] : '',
//      '#maxlength' => 255,
//      //'#element_validate' => array('element_validate_integer_positive'),
//      '#required' => TRUE,
//      '#attributes' => array(
//        //'data-prod-index' => $prod_index,
//        'class' => array('prod-group-id'),
//      ),
//    )
//  );
//
//  $table_row[] = array(
//    'qty' => array(
//      '#id' => 'prod-qty-' . $prod_index,
//      '#type' => 'textfield',
//      '#title' => 'Qty.',
//      '#title_display' => 'invisible',
//      //'#description' => t('@uom1 per @uom2', array('@uom1' => $plural_form, '@uom2' => $current_uom_name)),
//      //'#default_value' => isset($prod_row['qty']) ? $prod_row['qty'] : '',
//      '#maxlength' => 10,
//      '#element_validate' => array('element_validate_integer_positive'),
//      '#required' => TRUE,
//      '#attributes' => array(
//        //'data-prod-index' => $prod_index,
//        'class' => array('prod-group-qty'),
//      ),
//    )
//  );
//
//  $table_row[] = array(
//    'price' => array(
//      '#id' => 'prod-price-' . $prod_index,
//      '#type' => 'textfield',
//      '#title' => 'Price',
//      '#title_display' => 'invisible',
//      //'#default_value' => isset($prod_row['qty']) ? $prod_row['qty'] : '',
//      '#maxlength' => 19,
//      //'#element_validate' => array('element_validate_integer_positive'),
//      '#required' => TRUE,
//      '#attributes' => array(
//        //'data-prod-index' => $prod_index,
//        'class' => array('prod-group-price'),
//      ),
//    )
//  );
//
//  $table_row[] = array(
//    'amount' => array(
//      '#id' => 'prod-amt-' . $prod_index,
//      '#type' => 'textfield',
//      '#title' => 'Amount',
//      '#title_display' => 'invisible',
//      //'#default_value' => isset($prod_row['qty']) ? $prod_row['qty'] : '',
//      '#maxlength' => 19,
//      //'#element_validate' => array('element_validate_integer_positive'),
//      '#required' => TRUE,
//      '#attributes' => array(
//        //'data-prod-index' => $prod_index,
//        'class' => array('prod-group-amt'),
//      ),
//    ),
//    //'#column_attributes' => array(
//    //  'colspan' => 3,
//    //  'class' => array('a', 'b'),
//    //),
//  );
//
//
//
//  $form['product_table']['product_rows'][$prod_key] = $table_row;
//  //$form['product_table']['product_rows'][$prod_key]['#row_attributes'] = array(
//  //  'class' => array('e', 'd'),
//  //  'style' => 'background: red;',
//  //);
//  //if (count($prod_rows) == ($prod_index + 1)) {
//  //  $form['products'][$prod_key]['#suffix'] = '<div id="po-new-product-wrapper"></div>';
//  //}
//}