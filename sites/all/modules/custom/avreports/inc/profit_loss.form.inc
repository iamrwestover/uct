<?php

/**
 * Profit and Loss Report Form.
 * @param $form
 * @param $form_state
 */
function avreports_profit_loss_form($form, &$form_state) {
  $dad = avtxns_txn_list_filter_date_auto_options();
  $dad_default = empty($dad['this_week']) ? array() : $dad['this_week'];
  $filter_form_settings = array(
    'date_auto' => array(
      '#title' => t('Dates'),
      '#default_value' => 'this_week',
    ),
    'date_from' => array(
      '#title' => t('Date from'),
      '#default_value' => $dad_default['date_from'],
    ),
    'date_to' => array(
      '#title' => t('Date to'),
      '#default_value' => $dad_default['date_to'],
    ),
    'reset_button' => array('#attributes' => array('style' => 'margin-top: 25px;')),
  );
  $form['transaction_list_filter'] = avtxns_txn_list_filter_form($form, $form_state, $filter_form_settings);
  return $form;
}
