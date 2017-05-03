<?php

function avtxns_return_form($form, &$form_state, $transaction_type, $row, $goto_txn_type) {
  switch ($goto_txn_type) {
    case AVTXNS_TXN_TYPE_SALES_RETURN:
      $action_text = 'Return';
      break;
    case AVTXNS_TXN_TYPE_DELIVERY:
      $action_text = 'Copy';
      break;
    default:
      $action_text = '-';
      break;
  }
  drupal_set_title($action_text . ' items from ' . strtoupper($row->transaction_type) . '# ' . $row->id);

  // Get transaction type details.
  $ttd = avtxns_txn_types($transaction_type);

  if (!empty($ttd['ctd'])) {
    $form['client_name'] = array(
      '#markup' => empty($row->client_name) ? '' : '<div class="uk-text-bold">' . $row->client_name . '</div>',
      '#title' => $ttd['ctd']['name'],
    );
    $form['client_address_string'] = array(
      '#markup' => empty($row->address_string) ? '' : nl2br($row->address_string),
    );
    if (!empty($row->contact_number)) {
      $form['client_address_string']['#markup'] .= '<br />' . check_plain($row->contact_number);
    }
  }

  // Button.
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => $action_text . t(' selected products'),
    '#attributes' => array(
      'class' => array('uk-button-primary'),
      'data-ks' => 'CTRL+D',
    ),
    '#prefix' => '<br /><br />',
  );

  $items_by_category = avtxns_item_rows_by_category((array) $row->items);
  $table_rows = array();
  foreach ($items_by_category as $group_name => $item_group) {
    foreach ($item_group as $item_row) {
      if (!empty($item_row->qty) || $goto_txn_type != AVTXNS_TXN_TYPE_SALES_RETURN) {
        $table_rows[$item_row->id] = array(
          'qty' => array('data' => $item_row->qty, 'class' => array('uk-text-center')),
          'uom' => array('data' => check_plain($item_row->uom_title)),
          'product' => array('data' => check_plain($item_row->product_title)),
          'price' => array('data' => avbase_number_format($item_row->cost, 2), 'class' => array('uk-text-right')),
          'discount' => array('data' => (isset($item_row->discount_text) ? $item_row->discount_text : ''), 'class' => array('uk-text-right')),
          'total' => array('data' => avbase_number_format($item_row->total, 2), 'class' => array('uk-text-right')),
        );
      }
    }
  }

  $header = array(
    'qty' => array('data' => 'QTY', 'class' => array('uk-text-center')),
    'uom' => array('data' => 'UOM'),
    'product' => array('data' => 'PRODUCT'),
    'price' => array('data' => 'UNIT PRICE', 'class' => array('uk-text-right')),
    'discount' => array('data' => 'DISC.%', 'class' => array('uk-text-right')),
    'total' => array('data' => 'TOTAL', 'class' => array('uk-text-right')),
  );
  $form['table'] = array(
    '#type' => 'tableselect',
    '#header' => $header,
    '#options' => $table_rows,
    '#empty' => '<div class="uk-text-muted">' . t('No items found.') . '</div>',
    '#attributes' => array('class' => array('uk-table-condensed uk-table-hover av-frs')),
  );

  $form['transaction'] = array('#type' => 'value', '#value' => $row);
  $form['goto_txn_type'] = array('#type' => 'value', '#value' => $goto_txn_type);
  return $form;
}

function avtxns_return_form_validate($form, &$form_state) {
  $ids = empty($form_state['values']['table']) ? array() : $form_state['values']['table'];
  $transaction = empty($form_state['values']['transaction']) ? array() : $form_state['values']['transaction'];

  if (empty($transaction->id) || empty($transaction->items)) {
    form_set_error('', t('Invalid transaction.'));
    return;
  }

  $selected = array();
  foreach ($ids as $k => $v) {
    if (empty($v)) {
      continue;
    }
    $selected[] = $v;
  }

  if (empty($selected)) {
    form_set_error('', t('Please select at least 1 product.'));
    return;
  }

  if (count($selected) > AVTXNS_TXN_ROW_LIMIT) {
    form_set_error('', t('Maximum number of items allowed per transaction is %max.', array('%max' => AVTXNS_TXN_ROW_LIMIT)));
    return;
  }
}

function avtxns_return_form_submit($form, &$form_state) {
  $ids = empty($form_state['values']['table']) ? array() : $form_state['values']['table'];
  $transaction = empty($form_state['values']['transaction']) ? array() : $form_state['values']['transaction'];
  $goto_txn_type = empty($form_state['values']['goto_txn_type']) ? array() : $form_state['values']['goto_txn_type'];



  $item_rows = array();
  $i = 0;
  foreach ($ids as $txn_detail_id) {
    if (isset($transaction->items[$txn_detail_id])) {
      $item = $transaction->items[$txn_detail_id];
      $row_key = 'new_' . $i++;
      $item_rows[$row_key] = array(
        'item_id' => $item->item_id,
        'product_title' => $item->product_title,
        'uom_title' => $item->uom_title,
        'qty_per_uom' => $item->qty_per_uom,
        'qty' => $item->qty,
        'cost' => $goto_txn_type == AVTXNS_TXN_TYPE_DELIVERY ? NULL : $item->cost,
        'discount_text' => $item->discount_text,
        'total' => $item->total,
      );
    }
  }

  if ($goto_txn_type == AVTXNS_TXN_TYPE_DELIVERY) {
    drupal_set_message('Products were copied from a previous delivery. Unit Prices were not copied to make sure that this transaction uses up-to-date unit prices.<br />You must manually change UOMs to load current unit prices.
    <br /><i class="uk-icon-lightbulb-o"></i> Tip: Use &uarr; & &darr; to easily switch UOM fields. Type the first letter of your prefferred UOM to select it (e.g. "P" for pc, "D" for dozen, etc.).', 'warning');
  }

  $_SESSION['preload']['products'] = $item_rows;
  $_SESSION['preload']['client_name'] = $transaction->client_name;
  $_SESSION['preload']['agent_name'] = $transaction->agent_name;
  $_SESSION['preload']['term_id'] = empty($transaction->term_id) ? NULL : $transaction->term_id;
  $form_state['redirect'] = 'av/' . $goto_txn_type . '/new';
}
