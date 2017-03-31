<?php

/**
 * Fields for Client's Pending Orders.
 * @param $form
 * @param $form_state
 * @param string $transaction_type
 * @param $row
 * @param bool $view_mode
 *
 * @return array
 */
function avtxns_pending_orders_form_fields(&$form, &$form_state, $transaction_type, $row, $view_mode = FALSE) {
  // Get transaction type details.
  $ttd = avtxns_txn_types($transaction_type);

  // Get form storage.
  $form_state['storage'] = isset($form_state['storage']) ? $form_state['storage'] : array();
  $_storage = &$form_state['storage'];

  // Get transaction type.
  //$transaction_type = empty($_storage['transaction_type']) ? 'gr' : $_storage['transaction_type'];

  $reference_type = empty($form_state['values']['client_pending_pos_reference_type']) ? '' : $form_state['values']['client_pending_pos_reference_type'];
  $fields = array();
  $fields['selected_pos'] = array(
    '#type' => 'container',
    '#attributes' => array(
      'id' => 'avtxns-pending-orders-form-wrapper',
    ),
    '#tree' => TRUE,
  );

  $header = array(
    'transaction_id' => array('data' => '#', 'class' => 'suk-text-right'),
    'agent_name' => array('data' => t('Agent'), 'class' => ''),
    'transaction_date' => array('data' => t('Transaction Date')),
    'total' => array('data' => t('Total'), 'class' => 'uk-text-right'),
  );
  if (!empty($ttd['payment'])) {
    unset($header['agent_name']);
  }
  $client_pending_pos = empty($form_state['values']['client_pending_pos']) ? array() : $form_state['values']['client_pending_pos'];
  $table_rows = array();
  //$transaction_details = array();
  foreach ($client_pending_pos as $k => $v) {
    //$transaction_details[$v->id] = $v;
    $table_rows[$v->id] = array(
      'transaction_id' => array('data' => t('@ttype# @order_num', array('@ttype' => strtoupper($v->transaction_type), '@order_num' => $v->id)), 'class' => array('suk-text-right')),
      'agent_name' => check_plain($v->agent_name),
      'transaction_date' => format_date($v->transaction_date, 'custom', 'Y M d'),
      'total' => array('data' => number_format($v->grand_total, 2), 'class' => array('uk-text-right')),
    );
    if (!empty($ttd['payment'])) {
      unset($table_rows[$v->id]['agent_name']);
    }
  }

  if (!empty($table_rows)) {
    //$fields['selected_pos']['transaction_details'] = array('#type' => 'value', '#value' => $transaction_details);
    $fields['selected_pos']['transaction_id'] = array(
      '#type' => 'tableselect',
      '#header' => $header,
      '#options' => $table_rows,
      //'#sticky' => FALSE,
      //'#empty' => '<div class="uk-text-muted">' . t('No match found.') . '</div>',
      '#attributes' => array('class' => array('uk-table-striped')),
      //'#prefix' => '<div id="avtxns-pending-orders-form-wrapper">',
      //'#suffix' => '</div>',
      '#multiple' => (count($reference_type) > 1 || in_array($reference_type[0], array(AVTXNS_TXN_TYPE_SALES_RETURN, AVTXNS_TXN_TYPE_PURCHASE_RETURN))),
    );
  }
  // Buttons
  $fields['buttons']['check_po'] = array(
    '#id' => 'check-po-btn',
    '#type' => 'submit',
    '#value' => t('Check for pending orders'),
    '#attributes' => array(
      'class' => array('av-ajax-trigger', 'color' => 'uk-button-primary', 'hide' => 'uk-hidden'),
    ),
    '#submit' => array('avtxns_pending_orders_form_submit'),
    '#limit_validation_errors' => array(array('client_name'), array('client_id'), array('transaction_type')),
    '#ajax' => array(
      'callback' => 'avtxns_txn_form_ajax',
      'wrapper' => 'avtxns-txn-form-wrapper',
      'effect' => 'none',
      'event' => 'click',
    ),
    '#reference_type' => $ttd['pending_txns_reference_type'],
  );
  $fields['buttons']['submit'] = array(
    '#id' => 'pos-selected-submit-btn',
    '#type' => 'submit',
    '#value' => t('Add selected'),
    '#attributes' => array(
      'class' => array('av-ajax-trigger', 'uk-button-primary'),
    ),
    '#submit' => array('avtxns_pending_orders_form_submit'),
    '#limit_validation_errors' => array(array('client_name'), array('selected_pos'), array('transaction_type')),
    '#ajax' => array(
      'callback' => 'avtxns_txn_form_ajax',
      //'wrapper' => 'avtxns-gr-cost-changed-wrapper',
      'wrapper' => 'avtxns-txn-form-wrapper',
      'effect' => 'none',
      'event' => 'click',
    ),
    '#reference_type' => $reference_type,
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
    case AVTXNS_TXN_TYPE_RECEIVE:
      $modal_title = t('Pending purchase orders to this vendor');
      break;
    case AVTXNS_TXN_TYPE_DELIVERY:
      $modal_title = t('Pending sales orders from this customer');
      break;
    case AVTXNS_TXN_TYPE_SALES_RETURN:
      $modal_title = t('Recent delivery orders to this customer');
      break;
    case AVTXNS_TXN_TYPE_INVOICE:
      $modal_title = (empty($reference_type) || $reference_type[0] != AVTXNS_TXN_TYPE_DELIVERY) ? t('Recent returns from this customer') : t('Recent delivery orders to this customer');
      break;
    case AVTXNS_TXN_TYPE_PAYMENT:
      $modal_title = t('Recent customer receivables');
      break;
    case AVTXNS_TXN_TYPE_BILL:
      $modal_title = t('Recent items received from this vendor');
      break;
    case AVTXNS_TXN_TYPE_PAY_BILL:
      $modal_title = t('Recent bills for this vendor');
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
  $form['#attached']['js'][] = AVTXNS_MODULE_PATH . '/js/trans.pending_orders.form.js';
}

/**
 * Submit handler when Pending Orders have been selected and submitted.
 * @param $form
 * @param $form_state
 */
function avtxns_pending_orders_form_submit($form, &$form_state) {
  $form_state['rebuild'] = TRUE;
  $trigger = $form_state['triggering_element'];

  // Get form storage.
  $form_state['storage'] = isset($form_state['storage']) ? $form_state['storage'] : array();
  $_storage = &$form_state['storage'];

  // Get item rows.
  $_storage['item_rows'] = isset($_storage['item_rows']) ? $_storage['item_rows'] : array();
  $_item_rows = &$_storage['item_rows'];

  // Get transaction type.
  $transaction_type = empty($form_state['values']['transaction_type']) ? '' : $form_state['values']['transaction_type'];
  $ttd = avtxns_txn_types($transaction_type);
  $reference_type = empty($trigger['#reference_type']) ? array() : (array) $trigger['#reference_type'];
  //$rtd = avtxns_txn_types($reference_type);
  switch ($trigger['#id']) {
    case 'check-po-btn':
    case 'check-ret-btn':
      $client_id = (int) $form_state['values']['client_id'];
      // Check if this item row is owned by this client.
      // Remove item rows not owned by this client.
      foreach ($_item_rows as $k => $item_row) {
        $item_client_id = empty($item_row['client_id']) ? NULL : $item_row['client_id'];
        if (/*!empty($item_row['transaction_id']) && */!empty($item_client_id) && (empty($client_id) || $item_client_id != $client_id)) {
          unset($_item_rows[$k]);
          unset($form_state['input']['product_rows'][$k]);
        }
      }

      $conditions = array();
      $conditions[] = array('avtxns.client_id', $client_id);
      if (!empty($ttd['payment'])) {
        $conditions[] = array('avtxns.status', array(AVTXNS_TXN_STATUS_OPEN, AVTXNS_TXN_STATUS_CLOSED), 'IN');
      }
      else {
        $conditions[] = array('avtxns.status', AVTXNS_TXN_STATUS_OPEN);
      }
      $conditions[] = array('avtxns.transaction_type', $reference_type, 'IN');
      $pos = avtxns_txn_query_load($conditions);
      $floating_pos = array();
      foreach ($pos as $po_k => $po_v) {
        $rtd = avtxns_txn_types($po_v->transaction_type);
        if (!empty($ttd['payment']) && round($po_v->balance, 2) <= 0) {
          continue;
        }

        $pr_statuses = empty($rtd['pr_statuses']) ? array(AVTXNS_TXN_STATUS_OPEN) : $rtd['pr_statuses'];
        if (!in_array($po_v->status, $pr_statuses)) {
          continue;
        }
        $trimmed_po = new stdClass();
        $trimmed_po->id = $po_v->id;
        $trimmed_po->transaction_type = $po_v->transaction_type;
        $trimmed_po->agent_name = $po_v->agent_name;
        $trimmed_po->transaction_date = $po_v->transaction_date;
        $trimmed_po->grand_total = $po_v->grand_total;
        $floating_pos[] = $trimmed_po;
      }

      // Compare pos to already added item rows.
      // If already added item rows' qty is less than db po qty, show them as floating.
      //$floating_pos = array();
      //foreach ($pos as $po_id => $po) {
      //  if (empty($po->status)) {
      //    continue;
      //  }
      //  $po_items = empty($po->items) ? array() : $po->items;
      //  foreach ($po_items as $reference_id => $po_item) {
      //    if (!in_array($transaction_type, array(AVTXNS_TXN_TYPE_SALES_RETURN, 'sinv', 'spay'))) {
      //      if ($po_item->remaining_base_qty < 1) {
      //        continue;
      //      }
      //    }
      //
      //    $item_row_id = "{$reference_type}_db_item_{$po_id}_{$reference_id}";
      //    $match = isset($_item_rows[$item_row_id]) ? $_item_rows[$item_row_id] : FALSE;
      //    if (empty($match)) {
      //      $floating_pos[$po_id] = $po;
      //    }
      //    else {
      //      if (($match['qty'] * $match['qty_per_uom']) < $po_item->remaining_base_qty) {
      //        $floating_pos[$po_id] = $po;
      //      }
      //    }
      //  }
      //}

      $form_state['values']['client_pending_pos'] = $floating_pos;
      $form_state['values']['client_pending_pos_reference_type'] = $reference_type;

//dpm($_item_rows);
//dpm($floating_pos);
      break;

    case 'pos-selected-submit-btn':
      $selected_pos = empty($form_state['values']['selected_pos']) ? array() : $form_state['values']['selected_pos'];
      if (empty($selected_pos['transaction_id'])) {
        return;
      }
      $transaction_ids = (array) $selected_pos['transaction_id'];
      $transaction_ids = array_filter($transaction_ids);
      //$transaction_details = empty($selected_pos['transaction_details']) ? array() : $selected_pos['transaction_details'];
      //$selected_transaction_details = array();
      //foreach ($transaction_ids as $v) {
      //  $selected_transaction_details[$v] = empty($transaction_details[$v]) ? array() : $transaction_details[$v];
      //}
      $selected_transaction_details = avtxns_txn_load_ids($transaction_ids);

      //$txn_type_is_return = count($reference_type) == 1 && in_array($reference_type[0], array(AVTXNS_TXN_TYPE_SALES_RETURN, AVTXNS_TXN_TYPE_PURCHASE_RETURN));

      // Remove previously loaded pending items.
      foreach ($_item_rows as $k => $v) {
        //if ($txn_type_is_return) {
        //  continue;
        //}
        if (!isset($v['ref_txn_type'])) {
          continue;
        }
        if (strstr($k, "{$v['ref_txn_type']}_db_item_")) {
          unset($_item_rows[$k]);
          unset($form_state['input']['product_rows'][$k]);
        }
      }
      // Select po items and add them to $_item_rows.
      $receive_pos = array();
      foreach ($selected_transaction_details as $transaction) {
        if (!empty($ttd['payment'])) {
          $item_row_id = "{$transaction->transaction_type}_db_item_{$transaction->id}";
          $transaction_arr = (array) $transaction;
          $transaction_arr['ref_txn_dtl_id'] = $transaction->id;
          // Remember Invoice ID to immediately close invoice after payment, regardless of amount paid.
          $transaction_arr['ref_txn_id'] = $transaction->id;
          $transaction_arr['ref_txn_type'] = $transaction->transaction_type;
          //if (!empty($trigger['#reference_type'])) {
          //  $transaction_arr['reference_type'] = $trigger['#reference_type'];
          //}
          //$transaction_arr['paid_amount'] = $transaction_arr['total'];
          //$transaction_arr['transaction_id'] = $transaction_arr['id'];
          unset($transaction_arr['id']);
          unset($transaction_arr['items']);
          $transaction_arr['transaction_date_formatted'] = format_date($transaction_arr['transaction_date'], 'custom', 'Y M d');
          //$query = "SELECT SUM(paid_amount) FROM {avtbl_transaction_details} WHERE ref_txn_dtl_id = :invoice_id";
          //$total_paid_for_invoice = db_query($query, array(':invoice_id' => $transaction->id))->fetchField();
          //$total_paid_for_invoice = empty($total_paid_for_invoice) ? 0 : $total_paid_for_invoice;
          //$transaction_arr['balance'] = avbase_number_format(round($transaction->grand_total - $total_paid_for_invoice, 2), 2);
          $transaction_arr['balance'] = avbase_number_format($transaction_arr['balance'], 2);
          $transaction_arr['total'] = $transaction_arr['balance'];
          $transaction_arr['previous_payment'] = avbase_number_format($transaction_arr['total_paid'], 2);
          $receive_pos[$item_row_id] = $transaction_arr;
        }
        else {
          $db_item_rows = empty($transaction->items) ? array() : $transaction->items;
          foreach ($db_item_rows as $item_row) {
            $item_row_id = "{$transaction->transaction_type}_db_item_{$transaction->id}_{$item_row->id}";
            $item_row_arr = (array)$item_row;
            unset($item_row_arr['id']);
            $item_row_arr['ref_txn_dtl_id'] = $item_row->id;
            $item_row_arr['ref_txn_id'] = $transaction->id;
            $item_row_arr['ref_txn_type'] = $transaction->transaction_type;

            //if (!empty($trigger['#reference_type'])) {
            //  $item_row_arr['reference_type'] = $trigger['#reference_type'];
            //}

            // Set qty to remaining qty value.
            //$remaining_base_qty = isset($item_row_arr['remaining_base_qty']) ? $item_row_arr['remaining_base_qty'] : $item_row_arr['qty'];
            //if ($remaining_base_qty < 1) {
            //  continue;
            //}
            //
            //// Check if remaining qty is still divisible by qty per UOM.
            //if ($remaining_base_qty % $item_row_arr['qty_per_uom']) {
            //  // Set qty to remaining base qty.
            //  $item_row_arr['qty'] = $remaining_base_qty;
            //
            //  // Recompute cost.
            //  $item_row_arr['cost'] = $item_row_arr['cost'] / $item_row_arr['qty_per_uom'];
            //
            //  // Replace UOM with base UOM.
            //  $item_row_arr['uom_id'] = $item_row_arr['base_uom_id'];
            //  $item_row_arr['uom_title'] = $item_row_arr['base_uom_title'];
            //  $item_row_arr['qty_per_uom'] = 1;
            //} else {
            //  // Set qty by dividing remaining qty to qty per uom.
            //  $item_row_arr['qty'] = $remaining_base_qty / $item_row_arr['qty_per_uom'];
            //}

            // Recompute total amount.
            //$item_row_arr['total'] = round(($item_row_arr['cost'] * $item_row_arr['qty']), 2);
            ////if (!in_array($transaction_type, array(AVTXNS_TXN_TYPE_SALES_RETURN, 'rept'))) {
            //$discount = empty($item_row_arr['discount']) ? array() : explode('/', $item_row_arr['discount']);
            //foreach ($discount as $v) {
            //  if (is_numeric($v)) {
            //    $v = (float)$v;
            //    $item_row_arr['total'] = $item_row_arr['total'] - (($item_row_arr['total'] * $v) / 100);
            //  }
            //}
            //$item_row_arr['total'] = round($item_row_arr['total'], 2);
            //$item_row_arr['total'] *= $txn_type_is_return ? -1 : 1;
            $item_row_arr['total'] = avbase_number_format($item_row_arr['total'], 2);
            //}


            // More preparation.
            $item_row_arr['client_id'] = $transaction->client_id;
            $item_row_arr['cost'] = avbase_number_format($item_row_arr['cost'], 2);
            $receive_pos[$item_row_id] = $item_row_arr;
          }

          $form_state['#pending_agent_name'] = isset($transaction->agent_name) ? $transaction->agent_name : NULL;
          $form_state['#pending_term_id'] = isset($transaction->term_id) ? $transaction->term_id : NULL;
          $form_state['#pending_due_date_timestamp'] = isset($transaction->term_data['days']) ? strtotime('+' . $transaction->term_data['days'] . ' days') : NULL;
        }
      }

      $form_state['input']['product_rows'] = empty($form_state['input']['product_rows']) ? array() : $form_state['input']['product_rows'];
      //if ($txn_type_is_return) {
      //  $_item_rows = $_item_rows + $receive_pos;
      //  $form_state['input']['product_rows'] = $form_state['input']['product_rows'] + $receive_pos;
      //}
      //else {
        $_item_rows = $receive_pos + $_item_rows;
        $form_state['input']['product_rows'] = $receive_pos + $form_state['input']['product_rows'];
      //}
      break;
  }
}
