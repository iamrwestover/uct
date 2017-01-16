<?php

/**
 * Fields for Vendor Pending purchase orders.
 * @param $form
 * @param $form_state
 * @param $row
 *
 * @return array
 */
function avpurchase_gr_form_vendor_pos_fields(&$form, &$form_state, $row) {
  $fields = array();
  $fields['selected_pos'] = array(
    '#type' => 'container',
    '#attributes' => array(
      'id' => 'avpurchase-gr-form-vendor-pos-wrapper',
    ),
    '#tree' => TRUE,
  );
  $vendor_id = empty($form_state['values']['vendor_id']) ? NULL : $form_state['values']['vendor_id'];
  $vendor_open_pos = empty($form_state['values']['vendor_open_pos']) ? array() : $form_state['values']['vendor_open_pos'];
  $table_rows = array();
  foreach ($vendor_open_pos as $k => $v) {
    $table_cols = array();
    $table_cols['po_date'] = array(
      '#title' => 'PO Date',
      '#title_display' => 'none',
      '#type' => 'item',
      '#markup' => format_date($v->po_date, 'custom', 'Y M d, h:i a'),
      '#column_attributes' => array('class' => array('uk-width-4-10')),
    );
    $table_cols['total'] = array(
      '#title' => 'Total',
      '#title_display' => 'none',
      '#type' => 'item',
      '#markup' => number_format($v->grand_total, 2),
      '#column_attributes' => array('class' => array('uk-width-3-10', 'uk-text-right')),
    );
    $table_cols['po_num'] = array(
      '#title' => 'PO #',
      '#title_display' => 'none',
      '#type' => 'item',
      '#markup' => t('%po_num', array('%po_num' => $v->id)),
      '#column_attributes' => array('class' => array('uk-width-1-10', 'uk-text-right')),
    );
    $table_cols['values'] = array(
      'po' => array('#type' => 'value', '#value' => $v),
      'action' => array(
        '#type' => 'radios',
        '#title' => t('Action'),
        '#title_display' => 'none',
        '#options' => array(
          'ignore' => '<i class="uk-icon-ban"></i>',
          'receive' => '<i class="uk-icon-check"></i>',
        ),
        '#default_value' => 'ignore',
        '#attributes' => array('class' => array('uk-button-group')),
      ),
      //'new_cost' => array('#type' => 'value', '#value' => $proposed_cost),
      //'new_sales_price' => array('#type' => 'value', '#value' => $proposed_sales_price),
      '#column_attributes' => array('class' => array('uk-width-2-5', 'uk-text-center')),
    );
    $table_rows[$k] = $table_cols;
    $table_rows[$k]['#row_attributes'] = array('class' => array('uk-table-middle'));
  }

  if (!empty($table_rows)) {
    $table = array(
      '#theme' => 'avbase_form_table',
      '#header' => array(
        array('data' => t('P.O. Date')),
        array('data' => t('Total'), 'class' => 'uk-text-right'),
        array('data' => t('P.O. #'), 'class' => 'uk-text-right'),
        '&nbsp;',
      ),
      //'product_rows' => array('#tree' => TRUE),
    );
    $table['rows'] = $table_rows;
    $table['#attributes'] = array('class' => array('uk-table-striped', 'uk-table-condensed'));

    //$fields['product_cost_changes'] += $pcfields;
    //$fields['intro'] = array('#weight' => -5, '#markup' => '<p>Some product costs have been changed. Do you want to update?</p>');
    $fields['selected_pos']['table'] = $table;
  }

  // Buttons
  $fields['buttons']['check_po'] = array(
    '#id' => 'check-po-btn',
    '#type' => 'submit',
    '#value' => t('Check for floating po\'s'),
    '#attributes' => array(
      'class' => array('av-ajax-trigger', 'uk-button-primary', 'uk-hidden'),
    ),
    '#submit' => array('avpurchase_gr_vendor_pos_submit'),
    '#limit_validation_errors' => array(array('vendor_id')),
    '#ajax' => array(
      'callback' => 'avpurchase_gr_form_ajax',
      'wrapper' => 'avpurchase-gr-form-wrapper',
      'effect' => 'none',
      'event' => 'click',
    ),
  );
  $fields['buttons']['submit'] = array(
    '#id' => 'pos-selected-submit-btn',
    '#type' => 'submit',
    '#value' => t('Add selected PO\'s'),
    '#attributes' => array(
      'class' => array('av-ajax-trigger', 'uk-button-primary'),
    ),
    '#submit' => array('avpurchase_gr_vendor_pos_submit'),
    '#limit_validation_errors' => array(array('vendor_id'), array('selected_pos')),
    '#ajax' => array(
      'callback' => 'avpurchase_gr_form_ajax',
      //'wrapper' => 'avpurchase-gr-cost-changed-wrapper',
      'wrapper' => 'avpurchase-gr-form-wrapper',
      'effect' => 'none',
      'event' => 'click',
    ),
  );
  $fields['buttons']['cancel'] = array(
    '#id' => 'po-check-cancel-btn',
    '#type' => 'submit',
    '#value' => t('Cancel'),
    '#attributes' => array(
      'class' => array('uk-modal-close'),
    ),
  );

  $vendor_pos_form = $fields;
  $vendor_pos_form['#type'] = 'container';
  $vendor_pos_form['#theme'] = 'avbase_modal_form';
  $vendor_pos_form['#modal_options'] = array(
    'modal_id' => 'vendor-pos',
    'modal_title' => t('Pending purchase orders to this vendor'),
  );
  $form['vendor_pos'] = $vendor_pos_form;
}

/**
 * Submit handler when POs has been selected and submitted.
 * @param $form
 * @param $form_state
 */
function avpurchase_gr_vendor_pos_submit($form, &$form_state) {
  $form_state['rebuild'] = TRUE;
  $trigger = $form_state['triggering_element'];

  // Get form storage.
  $form_state['storage'] = isset($form_state['storage']) ? $form_state['storage'] : array();
  $_storage = &$form_state['storage'];

  // Get item rows.
  $_storage['item_rows'] = isset($_storage['item_rows']) ? $_storage['item_rows'] : array();
  $_item_rows = &$_storage['item_rows'];

  switch ($trigger['#id']) {
    case 'check-po-btn':
      $vendor_id = (int) $form_state['values']['vendor_id'];
      // Check if this item row is owned by this vendor.
      // Remove item rows not owned by this vendor.
      foreach ($_item_rows as $k => $item_row) {
        $item_vendor_id = empty($item_row['vendor_id']) ? NULL : $item_row['vendor_id'];
        if (!empty($item_row['po_id']) && (empty($vendor_id) || $item_vendor_id != $vendor_id)) {
          unset($_item_rows[$k]);
          unset($form_state['input']['product_rows'][$k]);
        }
      }

      $conditions = array();
      $conditions[] = array('avpo.vendor_id', $vendor_id);
      $conditions[] = array('avpo.status', 1);
      $pos = avpurchase_po_query_load($conditions);
      //foreach ($pos as $k => $po) {
      //  //avpurchase_po_attach_received_items($pos[$k]);
      //  //$po_items = (array) $po->items;
      //  //$po_item_ids[$po->id] = array_keys($po_items);
      //  dpm($po->items);
      //}

      // Compare pos to already added item rows.
      // If already added item rows' qty is less than db po qty, show them as floating.
      $floating_pos = array();
      foreach ($pos as $po_id => $po) {
        $po_items = empty($po->items) ? array() : $po->items;
        foreach ($po_items as $po_item_id => $po_item) {
          if ($po_item->remaining_base_qty < 1) {
            continue;
          }

          $item_row_id = "db_item_{$po_id}_{$po_item_id}";
          $match = isset($_item_rows[$item_row_id]) ? $_item_rows[$item_row_id] : FALSE;
          if (empty($match)) {
            $floating_pos[$po_id] = $po;
          }
          else {
            if (($match['qty'] * $match['qty_per_uom']) < $po_item->remaining_base_qty) {
              $floating_pos[$po_id] = $po;
            }
          }
        }
      }
      $form_state['values']['vendor_open_pos'] = $floating_pos;

//dpm($_item_rows);
//dpm($pos);
      break;

    case 'pos-selected-submit-btn':
      $selected_pos = empty($form_state['values']['selected_pos']['table']['rows']) ? array() : $form_state['values']['selected_pos']['table']['rows'];


      // Select po items and add them to $_item_rows.
      $receive_pos = array();
      foreach ($selected_pos as $k => $v) {
        $v_values = empty($v['values']) ? array() : $v['values'];
        $will_receive = empty($v_values['action']) ? FALSE : ($v_values['action'] == 'receive');
        if (!$will_receive || empty($v_values['po']->id)) {
          continue;
        }

        $db_item_rows = empty($v_values['po']->items) ? array() : $v_values['po']->items;
        foreach ($db_item_rows as $item_row) {
          $item_row_id = "db_item_{$v_values['po']->id}_{$item_row->id}";
          $item_row_arr = (array) $item_row;

          // Set qty to remaining qty value.
          $remaining_base_qty = isset($item_row_arr['remaining_base_qty']) ? $item_row_arr['remaining_base_qty'] : $item_row_arr['qty'];
          if ($remaining_base_qty < 1) {
            continue;
          }

          // Check if remaining qty is still divisible by qty per UOM.
          if ($remaining_base_qty % $item_row_arr['qty_per_uom']) {
            // Set qty to remaining base qty.
            $item_row_arr['qty'] = $remaining_base_qty;

            // Recompute cost.
            $item_row_arr['cost'] = $item_row_arr['cost'] / $item_row_arr['qty_per_uom'];

            // Replace UOM with base UOM.
            $item_row_arr['uom_id'] = $item_row_arr['base_uom_id'];
            $item_row_arr['uom_title'] = $item_row_arr['base_uom_title'];
            $item_row_arr['qty_per_uom'] = 1;
          }
          else {
            // Set qty by dividing remaining qty to qty per uom.
            $item_row_arr['qty'] = $remaining_base_qty / $item_row_arr['qty_per_uom'];
          }

          // Recompute total amount.
          $item_row_arr['total'] = $item_row_arr['cost'] * $item_row_arr['qty'];


          // More preparation.
          $item_row_arr['vendor_id'] = $v_values['po']->vendor_id;
          $item_row_arr['cost'] = avbase_min_decimal_precision($item_row_arr['cost']);
          $receive_pos[$item_row_id] = $item_row_arr;
        }
      }
      $_item_rows = $receive_pos + $_item_rows;
      $form_state['input']['product_rows'] = $receive_pos + $form_state['input']['product_rows'];
      break;
  }
}
