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
      // Get new row counter.
      $_storage['new_row_counter'] = isset($_storage['new_row_counter']) ? $_storage['new_row_counter'] : 0;
      $_counter = &$_storage['new_row_counter'];

      //// Add a new empty item row.
      //$_item_rows['new_' . $_counter] = array();
      //$_counter++;
      //
      // Add a new empty item row.
      //$_item_rows['new_' . $_counter] = array();

      for ($x = 0; $x < 20; $x++) {
        //$zz = avproduct_load($_counter + 1);
        //$xx['product_title'] = $zz->title;
        //$xx['qty_per_uom'] = 1;
        //$xx['qty'] = $zz->qty ? 1 : 0;
        //$xx['uom_id'] = 4;
        //$xx['uom_title'] = 'PC';
        //$xx['cost'] = $zz->price;
        //
        //$_item_rows['new_' . $_counter++] = $xx;
        $_item_rows['new_' . $_counter++] = array();
      }
      //$xx = avproduct_load($_counter + 1);
      //$xx->product_title = $xx->title;
      //$xx->qty_per_uom = 1;
      //$xx->qty = 1;
      //
      //$_item_rows['new_' . $_counter] = (array) $xx;

      break;

    default:
      // We'll presume it is the delete button.
      $item_row_key = isset($trigger['#item_row_key']) ? $trigger['#item_row_key'] : '';
      unset($_item_rows[$item_row_key]);
  }
}

/**
 * Ajax callback  when adding products or removing on a PO form.
 */
function avtxns_item_list_prod_js($form, $form_state) {
  $trigger = $form_state['triggering_element'];
  $output = array();
  if ($trigger['#name'] == 'prod_add_btn') {
    $product_elements = $form['product_rows'];


    $keys = element_children($product_elements);
    $keys = array_splice($keys, (count($keys) - 20));
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

