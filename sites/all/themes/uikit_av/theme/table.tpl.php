<?php

/**
 * @file
 * Returns HTML for a table.
 *
 * Available variables:
 * - $variables['header']: An array containing the table headers. Each element
 *   of the array can be either a localized string or an associative array with
 *   the following keys:
 *   - "data": The localized title of the table column.
 *   - "field": The database field represented in the table column (required if
 *     user is to be able to sort on this column).
 *   - "sort": A default sort order for this column ("asc" or "desc"). Only one
 *     column should be given a default sort order because table sorting only
 *     applies to one column at a time.
 *   - Any HTML attributes, such as "colspan", to apply to the column header
 *     cell.
 * - $variables['rows']: An array of table rows. Every row is an array of cells,
 *   or an associative array with the following keys:
 *   - "data": an array of cells
 *   - Any HTML attributes, such as "class", to apply to the table row.
 *   - "no_striping": a boolean indicating that the row should receive no 'even
 *     / odd' styling. Defaults to FALSE.
 *   Each cell can be either a string or an associative array with the following
 *   keys:
 *   - "data": The string to display in the table cell.
 *   - "header": Indicates this cell is a header.
 *   - Any HTML attributes, such as "colspan", to apply to the table cell.
 *   Here's an example for $rows:
 *   @code
 *   $rows = array(
 *     // Simple row
 *     array(
 *       'Cell 1', 'Cell 2', 'Cell 3'
 *     ),
 *     // Row with attributes on the row and some of its cells.
 *     array(
 *       'data' => array('Cell 1', array('data' => 'Cell 2', 'colspan' => 2)), 'class' => array('funky')
 *     )
 *   );
 *   @endcode
 * - $variables['attributes']: An array of HTML attributes to apply to the table
 *   tag.
 * - $variables['caption']: A localized string to use for the <caption> tag.
 * - $variables['colgroups']: An array of column groups. Each element of the
 *   array can be either:
 *   - An array of columns, each of which is an associative array of HTML
 *     attributes applied to the COL element.
 *   - An array of attributes applied to the COLGROUP element, which must
 *     include a "data" attribute. To add attributes to COL elements, set the
 *     "data" attribute with an array of columns, each of which is an
 *     associative array of HTML attributes.
 *   Here's an example for $colgroup:
 *   @code
 *   $colgroup = array(
 *     // COLGROUP with one COL element.
 *     array(
 *       array(
 *         'class' => array('funky'), // Attribute for the COL element.
 *       ),
 *     ),
 *     // Colgroup with attributes and inner COL elements.
 *     array(
 *       'data' => array(
 *         array(
 *           'class' => array('funky'), // Attribute for the COL element.
 *         ),
 *       ),
 *       'class' => array('jazzy'), // Attribute for the COLGROUP element.
 *     ),
 *   );
 *   @endcode
 *   These optional tags are used to group and set properties on columns within
 *   a table. For example, one may easily group three columns and apply same
 *   background style to all.
 * - $variables['sticky']: Use a "sticky" table header.
 * - $variables['empty']: The message to display in an extra row if table does
 *   not have any rows.
 *
 * @see theme_table()
 * @see uikit_preprocess_table()
 *
 * @ingroup uikit_themeable
 */

$header = $variables['header'];
$rows = $variables['rows'];
$attributes = $variables['attributes'];
$caption = $variables['caption'];
$colgroups = $variables['colgroups'];
$sticky = $variables['sticky'];
$empty = $variables['empty'];

// Add sticky headers, if applicable.
if (count($header) && $sticky) {
  drupal_add_js('misc/tableheader.js');
  // Add 'sticky-enabled' class to the table to identify it for JS.
  // This is needed to target tables constructed by this function.
  $attributes['class'][] = 'sticky-enabled';
}

$output = '<div class="' . ($sticky ? 'non-' : '') . 'uk-overflow-container">';
$output .= '<table' . drupal_attributes($attributes) . ">\n";

if (isset($caption)) {
  $output .= '<caption>' . $caption . "</caption>\n";
}

// Format the table columns:
if (count($colgroups)) {
  foreach ($colgroups as $number => $colgroup) {
    $attributes = array();

    // Check if we're dealing with a simple or complex column.
    if (isset($colgroup['data'])) {
      foreach ($colgroup as $key => $value) {
        if ($key == 'data') {
          $cols = $value;
        }
        else {
          $attributes[$key] = $value;
        }
      }
    }
    else {
      $cols = $colgroup;
    }

    // Build colgroup.
    if (is_array($cols) && count($cols)) {
      $output .= ' <colgroup' . drupal_attributes($attributes) . '>';
      $i = 0;
      foreach ($cols as $col) {
        $output .= ' <col' . drupal_attributes($col) . ' />';
      }
      $output .= " </colgroup>\n";
    }
    else {
      $output .= ' <colgroup' . drupal_attributes($attributes) . " />\n";
    }
  }
}

// Add the 'empty' row message if available.
if (!count($rows) && $empty) {
  $header_count = 0;
  foreach ($header as $header_cell) {
    if (is_array($header_cell)) {
      $header_count += isset($header_cell['colspan']) ? $header_cell['colspan'] : 1;
    }
    else {
      $header_count++;
    }
  }
  $rows[] = array(
    array(
      'data' => $empty,
      'colspan' => $header_count,
      'class' => array('empty', 'message'),
    ),
  );
}

// Format the table header:
if (count($header)) {
  $ts = tablesort_init($header);
  // HTML requires that the thead tag has tr tags in it followed by tbody
  // tags. Using ternary operator to check and see if we have any rows.
  $output .= (count($rows) ? ' <thead><tr class="uk-text-left">' : ' <tr>');
  foreach ($header as $cell) {
    $cell = tablesort_header($cell, $header, $ts);
    $output .= _theme_table_cell($cell, TRUE);
  }
  // Using ternary operator to close the tags based on whether or not there are
  // rows.
  $output .= (count($rows) ? " </tr></thead>\n" : "</tr>\n");
}
else {
  $ts = array();
}

// Format the table rows:
if (count($rows)) {
  $output .= "<tbody>\n";
  $flip = array('even' => 'odd', 'odd' => 'even');
  $class = 'even';
  foreach ($rows as $number => $row) {
    // Custom row class implementation for imrpoved memory usage.
    $custom_row_class = empty($row['#avclass']) ? NULL : $row['#avclass'];
    unset($row['#avclass']);

    // Check if we're dealing with a simple or complex row.
    if (isset($row['data'])) {
      $cells = $row['data'];
      $no_striping = isset($row['no_striping']) ? $row['no_striping'] : FALSE;

      // Set the attributes array and exclude 'data' and 'no_striping'.
      $attributes = $row;
      unset($attributes['data']);
      unset($attributes['no_striping']);
    }
    else {
      $cells = $row;
      $attributes = array();
      $no_striping = FALSE;
    }
    if (count($cells)) {
      // Add odd/even class.
      //if (!$no_striping) {
      //  $class = $flip[$class];
      //  $attributes['class'][] = $class;
      //}
      //$attributes['class'][] = 'uk-text-left';

      // Build row.
      $attr_str = empty($attributes) ? NULL : drupal_attributes($attributes);
      $custom_row_class_str = empty($custom_row_class) && empty($attributes['class']) ? NULL : ' class="' . $custom_row_class . '"';
      $output .= ' <tr' . drupal_attributes($attributes) . $custom_row_class_str . '>';
      $i = 0;
      foreach ($cells as $cell) {
        $cell = tablesort_cell($cell, $header, $ts, $i++);
        $output .= _theme_table_cell($cell);
      }
      $output .= " </tr>\n";
    }
  }
  $output .= "</tbody>\n";
}

$output .= "</table>\n";
$output .= "</div>\n";
print $output;
