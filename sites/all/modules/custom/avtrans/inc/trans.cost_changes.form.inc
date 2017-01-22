<?php

/**
 * Product Cost Changes form fields.
 * @param $form
 * @param $form_state
 * @param $row
 *
 * @return mixed
 */
function avtrans_cost_changes_form_fields(&$form, &$form_state, $row, $view_mode = FALSE) {
  // Get form storage.
  $form_state['storage'] = isset($form_state['storage']) ? $form_state['storage'] : array();
  $_storage = &$form_state['storage'];
  $transaction = empty($_storage['transaction']) ? '' : $_storage['transaction'];

  $detected_cost_changes = empty($form_state['values']['detected_cost_changes']) ? array() : $form_state['values']['detected_cost_changes'];
  $fields = array();
  $fields['product_cost_changes'] = array('#tree' => TRUE);
  $table_rows = array();
  foreach ($detected_cost_changes as $k => $v) {
    if (empty($v['product_id'])) {
      continue;
    }

    $table_cols = array();
    $current_cost = $v['current_cost'];
    $current_sales_price = $v['current_sales_price'];
    $proposed_cost = $v['proposed_cost'];
    $proposed_sales_price = $v['proposed_sales_price'];
    $price_markup = $v['price_markup'];
    $direction_class = array('arrow-up' => 'uk-text-primary', 'arrow-down' => 'uk-text-warning');
    $table_cols['product_title'] = array(
      '#title' => 'Product',
      '#title_display' => 'none',
      '#type' => 'item',
      '#markup' => check_plain($v['title']),
      '#column_attributes' => array('class' => array('uk-width-1-5')),
    );

    $current_cost_formatted = number_format($current_cost, 2);
    $proposed_cost_formatted = number_format($proposed_cost, 2);
    $direction = ($proposed_cost > $current_cost) ? 'arrow-up' : 'arrow-down';
    $table_cols['cost'] = array(
      '#title' => 'Proposed cost change',
      '#title_display' => 'none',
      '#type' => 'item',
      '#markup' => t('!direction from @current_cost to @proposed_cost', array('!direction' => "<i class=\"uk-icon-{$direction}\"></i>", '@current_cost' => $current_cost_formatted, '@proposed_cost' => $proposed_cost_formatted)),
      '#column_attributes' => array('class' => array('uk-width-1-5', $direction_class[$direction])),
    );
    if ($transaction == 'sales') {
      $table_cols['cost']['#title'] = t('Proposed sales price change');
    }
    else {
      $current_sales_price_formatted = number_format($current_sales_price, 2);
      $proposed_sales_price_formatted = number_format($proposed_sales_price, 2);
      $direction = ($proposed_sales_price > $current_sales_price) ? 'arrow-up' : 'arrow-down';
      $table_cols['sales_price'] = array(
        '#title' => 'Proposed sales price change',
        '#title_display' => 'none',
        '#type' => 'item',
        '#markup' => t('!direction from @current_sales_price to @proposed_sales_price', array('!direction' => "<i class=\"uk-icon-{$direction}\"></i>", '@current_sales_price' => $current_sales_price_formatted, '@proposed_sales_price' => $proposed_sales_price_formatted)),
        '#column_attributes' => array('class' => array("uk-width-1-5", $direction_class[$direction])),
      );
    }

    $table_cols['values'] = array(
      'action' => array(
        '#type' => 'radios',
        '#title' => t('Action'),
        '#title_display' => 'none',
        '#options' => array(
          'ignore' => t('Leave unchanged'),
          'cost' => t('Update cost'),
          'cost_and_price' => t('Update cost and sales price'),
        ),
        '#default_value' => 'ignore',
        '#attributes' => array('class' => array('uk-button-group')),
      ),
      'product_id' => array('#type' => 'value', '#value' => $v['product_id']),
      'new_cost' => array('#type' => 'value', '#value' => $proposed_cost),
      'new_sales_price' => array('#type' => 'value', '#value' => $proposed_sales_price),
      '#column_attributes' => array('class' => array('uk-width-2-5')),
    );
    if ($transaction == 'sales') {
      unset($table_cols['values']['action']['#options']['cost_and_price']);
      $table_cols['values']['action']['#options']['cost'] = t('Update sales price');
    }

    $table_rows[$k] = $table_cols;
    $table_rows[$k]['#row_attributes'] = array('class' => array('uk-table-middle'));
  }

  if (!empty($table_rows)) {
    $table = array(
      '#theme' => 'avbase_form_table',
      '#header' => array(
        'title' => array('data' => t('Product')),
        'cost' => array('data' => t('Change on cost')),
        'cost_and_price' => array('data' => t('Change on sales price')),
        'actions' => '&nbsp;',
      ),
      //'product_rows' => array('#tree' => TRUE),
    );
    if ($transaction == 'sales') {
      unset($table['#header']['cost_and_price']);
      $table['#header']['cost'] = t('Change on price');
    }
    $table['rows'] = $table_rows;
    $table['#attributes'] = array('class' => array('uk-table-striped', 'uk-table-condensed'));

    //$fields['product_cost_changes'] += $pcfields;
    //$fields['intro'] = array('#weight' => -5, '#markup' => '<p>Some product costs have been changed. Do you want to update?</p>');
    $fields['product_cost_changes'] += $table;
  }

  // Buttons
  $fields['buttons']['submit'] = array(
    '#id' => 'cost-changed-submit-btn',
    '#type' => 'submit',
    '#value' => t('Continue'),
    '#attributes' => array(
      'class' => array('av-ajax-trigger', 'uk-button-primary'),
    ),
    '#ajax' => array(
      'callback' => 'avtrans_transaction_form_ajax',
      //'wrapper' => 'avtrans-gr-cost-changed-wrapper',
      'wrapper' => 'avtrans-transaction-form-wrapper',
      'effect' => 'none',
      'event' => 'click',
    ),
  );
  $fields['buttons']['cancel'] = array(
    '#id' => 'cost-changed-cancel-btn',
    '#type' => 'submit',
    '#value' => t('Cancel'),
    '#attributes' => array(
      'class' => array('uk-modal-close'),
    ),
  );

  $cost_changes_form = $fields;
  $cost_changes_form['#type'] = 'container';
  $cost_changes_form['#theme'] = 'avbase_modal_form';
  $field_name = $transaction == 'sales' ? 'prices' : 'costs';
  $cost_changes_form['#modal_options'] = array(
    'modal_id' => 'cost-changed',
    'modal_title' => t('Some product @field_name have been changed. Do you want to update our records?', array('@field_name' => $field_name)),
    'modal_classes' => array('uk-modal-dialog-large'),
  );
  $form['cost_changes'] = $cost_changes_form;

  // Attachments.
  $form['#attached']['js'][] = AVTRANS_MODULE_PATH . '/js/trans.cost_changes.form.js';
}

/**
 * Update products record with changes to cost and price.
 * @param $form_state
 * @param $row
 * @param $product_ids
 * @param $transaction_type
 */
function avtrans_cost_changes_db_update($form_state, $row, $product_ids, $transaction_type ='') {
  // Get form storage.
  $form_state['storage'] = isset($form_state['storage']) ? $form_state['storage'] : array();
  $_storage = &$form_state['storage'];

  // Get transaction type.
  $transaction = empty($_storage['transaction']) ? '' : $_storage['transaction'];
  $transaction_type = empty($_storage['transaction_type']) ? 'gr' : $_storage['transaction_type'];

  // Get form values.
  $values = empty($form_state['values']) ? array() : $form_state['values'];

  $cost_changes = empty($values['product_cost_changes']) ? array() : $values['product_cost_changes'];
  if (!empty($cost_changes)) {
    $cost_set = array();
    $price_set = array();
    $cost_args = array();
    $price_args = array();
    foreach ($cost_changes['rows'] as $cost_change) {
      $changes = empty($cost_change['values']) ? array() : $cost_change['values'];
      if (empty($changes)) {
        continue;
      }
      $product_id = empty($changes['product_id']) ? 'none' : $changes['product_id'];
      if (!in_array($product_id, $product_ids)) {
        continue;
      }

      switch ($changes['action']) {
        case 'cost':
          if ($transaction == 'sales') {
            $price_set[$product_id] = "WHEN $product_id THEN :price{$product_id}";
            $price_args[":price{$product_id}"] = $changes['new_cost'];
          }
          else {
            $cost_set[$product_id] = "WHEN $product_id THEN :cost{$product_id}";
            $cost_args[":cost{$product_id}"] = $changes['new_cost'];
          }
          break;
        case 'cost_and_price':
          $cost_set[$product_id] = "WHEN $product_id THEN :cost{$product_id}";
          $cost_args[":cost{$product_id}"] = $changes['new_cost'];
          if ($changes['action'] == 'cost_and_price') {
            $price_set[$product_id] = "WHEN $product_id THEN :price{$product_id}";
            $price_args[":price{$product_id}"] = $changes['new_sales_price'];
          }
          break;
        default:
          continue;
          break;
      }
    }

    if (!empty($cost_set)) {
      $query = "UPDATE {avtbl_products} SET cost = CASE id " . implode(' ', $cost_set) . " END WHERE id IN(:product_ids)";
      $cost_args[':product_ids'] = array_keys($cost_set);
      if (db_query($query, $cost_args)) {
        avbase_custom_watchdog("{$transaction_type}__cost_update", "ID: {$row['id']}", array('#row' => $row, '#cost_changes' => $cost_changes, '#cost_set' => $cost_set, '#args' => $cost_args));
      }
    }
    if (!empty($price_set)) {
      $query = "UPDATE {avtbl_products} SET price = CASE id " . implode(' ', $price_set) . " END WHERE id IN(:product_ids)";
      $price_args[':product_ids'] = array_keys($price_set);
      if (db_query($query, $price_args)) {
        avbase_custom_watchdog("{$transaction_type}__price_update", "ID: {$row['id']}", array('#row' => $row, '#cost_changes' => $cost_changes, '#price_set' => $price_set, '#args' => $price_args));
      }
    }
  }
}