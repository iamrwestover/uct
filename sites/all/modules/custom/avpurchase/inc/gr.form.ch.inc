<?php

/**
 * Product Cost Changes form fields.
 * @param $form
 * @param $form_state
 * @param $row
 *
 * @return mixed
 */
function avpurchase_gr_form_cost_changes_fields(&$form, &$form_state, $row) {
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

    $table_rows[$k] = $table_cols;
    $table_rows[$k]['#row_attributes'] = array('class' => array('uk-table-middle'));
  }

  if (!empty($table_rows)) {
    $table = array(
      '#theme' => 'avbase_form_table',
      '#header' => array(
        array('data' => t('Product')),
        array('data' => t('Change on cost')),
        array('data' => t('Change on sales price')),
        '&nbsp;',
      ),
      //'product_rows' => array('#tree' => TRUE),
    );
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
      'callback' => 'avpurchase_gr_form_ajax',
      //'wrapper' => 'avpurchase-gr-cost-changed-wrapper',
      'wrapper' => 'avpurchase-gr-form-wrapper',
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
  $cost_changes_form['#modal_options'] = array(
    'modal_id' => 'cost-changed',
    'modal_title' => t('Some product costs have been changed. Below is the recommended record updates.'),
    'modal_classes' => array('uk-modal-dialog-large'),
  );
  $form['cost_changes'] = $cost_changes_form;


}
