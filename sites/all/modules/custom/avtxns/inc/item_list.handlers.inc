<?php

/**
 * Submit handler when adding or removing products on a PO form.
 * @param $form
 * @param $form_state
 */
function avtxns_item_list_prod_submit($form, &$form_state) {
  // Rebuild form.
  $form_state['rebuild'] = TRUE;

  // Get which element triggered this handler.
  $trigger = $form_state['triggering_element'];

  // Get form storage.
  $form_state['storage'] = isset($form_state['storage']) ? $form_state['storage'] : array();
  $_storage = &$form_state['storage'];

  // Get item rows.
  $_storage['item_rows'] = isset($_storage['item_rows']) ? $_storage['item_rows'] : array();
  $_item_rows = &$_storage['item_rows'];


  // Perform actions based on triggering element.
  switch ($trigger['#name']) {
    case 'prod_add_btn':
      // Add button triggered it.

      // Limit to row count per transaction.
      $form_state['#max_rows_reached'] = count($_item_rows) >= AVTXNS_TXN_ROW_LIMIT;
      if ($form_state['#max_rows_reached']) {
        break;
      }

      // Get new row counter.
      $_storage['new_row_counter'] = isset($_storage['new_row_counter']) ? $_storage['new_row_counter'] : 0;
      $_counter = &$_storage['new_row_counter'];


      for ($x = 0; $x < AVTXNS_TXN_ADD_NEW_ROW_COUNT; $x++) {
        //$row = db_query("SELECT b.title AS uom_title, a.title AS product_title, 1 AS qty_per_uom, 10 AS qty, a.uom_id, a.cost, a.discount1 AS discount_text
        //FROM avtbl_products a
        //LEFT JOIN avtbl_categories AS b ON b.id = a.uom_id
        //ORDER BY RAND() LIMIT 0,1")->fetch();
        //$_item_rows['new_' . $_counter++] = (array) $row;
        $_item_rows['new_' . $_counter++] = array();
      }
      break;

    default:
      // We'll presume it is the delete button.
      $item_row_key = isset($trigger['#item_row_key']) ? $trigger['#item_row_key'] : '';
      unset($_item_rows[$item_row_key]);
      break;
  }
}

/**
 * Ajax callback  when adding products or removing on a PO form.
 */
function avtxns_item_list_prod_js($form, $form_state) {
  $trigger = $form_state['triggering_element'];
  $output = array();

  if ($trigger['#name'] == 'prod_add_btn') {
    if (empty($form_state['#max_rows_reached'])) {
      $product_elements = $form['product_rows'];
      $keys = element_children($product_elements);
      $keys = array_splice($keys, (count($keys) - AVTXNS_TXN_ADD_NEW_ROW_COUNT));
      $new_elements = array();
      $last_key = NULL;
      foreach ($keys as $k) {
        $new_elements[$k] = empty($product_elements[$k]) ? array() : $product_elements[$k];
        $last_key = $k;
      }
      if ($last_key) {
        $new_elements[$last_key]['#suffix'] = empty($new_elements[$last_key]['#suffix']) ? '' : $new_elements[$last_key]['#suffix'];
        $new_elements[$last_key]['#suffix'] .= '<div id="item-list-new-product-wrapper"></div>';
      }

      //dpm($product_elements);
      //$last_key = array_pop($keys);
      //$last_element = empty($product_elements[$last_key]) ? array() : $product_elements[$last_key];
      //$last_element['#suffix'] = empty($last_element['#suffix']) ? '' : $last_element['#suffix'];
      //$last_element['#suffix'] .= '<div id="item-list-new-product-wrapper"></div>';

      $output = $new_elements;
    }
    else {
      $output = '<div class="uk-text-muted uk-text-center">Maximum number of items per transaction has been reached.</div>';
    }
  }
  elseif (strstr($trigger['#name'], 'product_id')) {
    $parents = $trigger['#parents'];
    array_pop($parents);
    $parents[] = 'uom_id';
    $x = drupal_array_get_nested_value($form, $parents);
    $x['#title_display'] = 'none';
    $output = $x;
    //$output = '<div>' . $x['#name'] . '</div>';
  }
  else {
    //$js_settings = array('avNestableProductForm' => array('ajaxAction' => 'remove'));
    //$output['#attached']['js'] = array(
    //  array(
    //    'data' => $js_settings,
    //    'type' => 'setting',
    //  ),
    //);
    //$output = array();
  }

  return $output;
}

