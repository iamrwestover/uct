<?php

/**
 * Fields for Client's Pending Orders.
 * @param $form
 * @param $form_state
 * @param $row
 *
 * @return array
 */
function avtrans_pending_orders_form_fields(&$form, &$form_state, $row, $view_mode = FALSE) {
  // Get form storage.
  $form_state['storage'] = isset($form_state['storage']) ? $form_state['storage'] : array();
  $_storage = &$form_state['storage'];

  // Get transaction type.
  $transaction_type = empty($_storage['transaction_type']) ? 'gr' : $_storage['transaction_type'];

  $fields = array();
  $fields['selected_pos'] = array(
    '#type' => 'container',
    '#attributes' => array(
      'id' => 'avtrans-pending-orders-form-wrapper',
    ),
    '#tree' => TRUE,
  );

  $client_pending_pos = empty($form_state['values']['client_pending_pos']) ? array() : $form_state['values']['client_pending_pos'];
  $table_rows = array();
  foreach ($client_pending_pos as $k => $v) {
    $table_cols = array();
    $table_cols['transaction_date'] = array(
      '#title' => 'Date',
      '#title_display' => 'none',
      '#type' => 'item',
      '#markup' => format_date($v->transaction_date, 'custom', 'Y M d, h:i a'),
      '#column_attributes' => array('class' => array('uk-width-3-10')),
    );
    $table_cols['total'] = array(
      '#title' => 'Total',
      '#title_display' => 'none',
      '#type' => 'item',
      '#markup' => number_format($v->grand_total, 2),
      '#column_attributes' => array('class' => array('uk-width-3-10', 'uk-text-right')),
    );
    $table_cols['order_num'] = array(
      '#title' => '#',
      '#title_display' => 'none',
      '#type' => 'item',
      '#markup' => t('Order # %order_num', array('%order_num' => $v->id)),
      '#column_attributes' => array('class' => array('uk-width-2-10', 'uk-text-right')),
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
        array('data' => t('Order Date')),
        array('data' => t('Total'), 'class' => 'uk-text-right'),
        array('data' => t('&nbsp;'), 'class' => 'uk-text-right'),
        '&nbsp;',
      ),
      //'#caption' => 'Ivan Dorseys',
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
    '#value' => t('Check for pending orders'),
    '#attributes' => array(
      'class' => array('av-ajax-trigger', 'uk-button-primary', 'uk-hidden'),
    ),
    '#submit' => array('avtrans_pending_orders_form_submit'),
    '#limit_validation_errors' => array(array('client_name'), array('client_id')),
    '#ajax' => array(
      'callback' => 'avtrans_transaction_form_ajax',
      'wrapper' => 'avtrans-transaction-form-wrapper',
      'effect' => 'none',
      'event' => 'click',
    ),
  );
  $fields['buttons']['submit'] = array(
    '#id' => 'pos-selected-submit-btn',
    '#type' => 'submit',
    '#value' => t('Add selected orders'),
    '#attributes' => array(
      'class' => array('av-ajax-trigger', 'uk-button-primary'),
    ),
    '#submit' => array('avtrans_pending_orders_form_submit'),
    '#limit_validation_errors' => array(array('client_name'), array('selected_pos')),
    '#ajax' => array(
      'callback' => 'avtrans_transaction_form_ajax',
      //'wrapper' => 'avtrans-gr-cost-changed-wrapper',
      'wrapper' => 'avtrans-transaction-form-wrapper',
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

  $modal_title = t('Pending orders to this vendor');
  switch ($transaction_type) {
    case 'gr':
      $modal_title = t('Pending purchase orders to this vendor');
      break;
    case 'dr':
      $modal_title = t('Pending sales orders from this customer');
      break;
    case 'ret':
      $modal_title = t('Recent delivery orders to this customer');
      break;
  }
  $pending_orders_form = $fields;
  $pending_orders_form['#type'] = 'container';
  $pending_orders_form['#theme'] = 'avbase_modal_form';
  $pending_orders_form['#modal_options'] = array(
    'modal_id' => 'pending-orders',
    'modal_title' => $modal_title,
  );
  $form['pending_orders'] = $pending_orders_form;

  // Attachments.
  $form['#attached']['js'][] = AVTRANS_MODULE_PATH . '/js/trans.pending_orders.form.js';
}

/**
 * Submit handler when Pending Orders have been selected and submitted.
 * @param $form
 * @param $form_state
 */
function avtrans_pending_orders_form_submit($form, &$form_state) {
  $form_state['rebuild'] = TRUE;
  $trigger = $form_state['triggering_element'];

  // Get form storage.
  $form_state['storage'] = isset($form_state['storage']) ? $form_state['storage'] : array();
  $_storage = &$form_state['storage'];

  // Get item rows.
  $_storage['item_rows'] = isset($_storage['item_rows']) ? $_storage['item_rows'] : array();
  $_item_rows = &$_storage['item_rows'];

  // Get transaction type.
  $transaction_type = empty($_storage['transaction_type']) ? 'gr' : $_storage['transaction_type'];

  switch ($trigger['#id']) {
    case 'check-po-btn':
      $client_id = (int) $form_state['values']['client_id'];
      // Check if this item row is owned by this client.
      // Remove item rows not owned by this client.
      foreach ($_item_rows as $k => $item_row) {
        $item_client_id = empty($item_row['client_id']) ? NULL : $item_row['client_id'];
        if (!empty($item_row['po_id']) && (empty($client_id) || $item_client_id != $client_id)) {
          unset($_item_rows[$k]);
          unset($form_state['input']['product_rows'][$k]);
        }
      }

      $conditions = array();
      $pos = array();
      switch ($transaction_type) {
        case 'gr':
          $conditions[] = array('avpo.client_id', $client_id);
          $conditions[] = array('avpo.status', 1);
          $pos = avtrans_po_query_load($conditions);
          break;
        case 'dr':
          $conditions[] = array('avso.client_id', $client_id);
          $conditions[] = array('avso.status', 1);
          $pos = avtrans_so_query_load($conditions);
          break;
        case 'ret':
          $conditions[] = array('avdr.client_id', $client_id);
          $conditions[] = array('avdr.status', 1);
          $pos = avtrans_dr_query_load($conditions);
          break;
      }

      //foreach ($pos as $k => $po) {
      //  //avtrans_po_attach_received_items($pos[$k]);
      //  //$po_items = (array) $po->items;
      //  //$reference_ids[$po->id] = array_keys($po_items);
      //  dpm($po->items);
      //}

      // Compare pos to already added item rows.
      // If already added item rows' qty is less than db po qty, show them as floating.
      $floating_pos = array();
      foreach ($pos as $po_id => $po) {
        $po_items = empty($po->items) ? array() : $po->items;
        foreach ($po_items as $reference_id => $po_item) {
          if ($transaction_type != 'ret') {
            if ($po_item->remaining_base_qty < 1) {
              continue;
            }
          }

          $item_row_id = "db_item_{$po_id}_{$reference_id}";
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

      $form_state['values']['client_pending_pos'] = $floating_pos;

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
          $item_row_arr['total'] = round(($item_row_arr['cost'] * $item_row_arr['qty']), 2);
          //if (!in_array($transaction_type, array('ret', 'rept'))) {
            $discount = empty($item_row_arr['discount']) ? array() : explode('/', $item_row_arr['discount']);
            foreach ($discount as $v) {
              if (is_numeric($v)) {
                $v = (float)$v;
                $item_row_arr['total'] = $item_row_arr['total'] - (($item_row_arr['total'] * $v) / 100);
              }
            }
            $item_row_arr['total'] = round($item_row_arr['total'], 2);
          //}


          // More preparation.
          $item_row_arr['client_id'] = $v_values['po']->client_id;
          $item_row_arr['cost'] = avbase_number_format($item_row_arr['cost']);
          $receive_pos[$item_row_id] = $item_row_arr;
        }
      }
      $_item_rows = $receive_pos + $_item_rows;
      $form_state['input']['product_rows'] = $receive_pos + $form_state['input']['product_rows'];
      break;
  }
}
