<?php

module_load_include('inc', 'avreports', 'inc/product_list.form');
module_load_include('inc', 'avproducts', 'inc/crud');

/**
 * Product batch update form.
 * @param $form
 * @param $form_state
 *
 * @return array
 */
function avproducts_product_batch_update_form($form, &$form_state) {
  unset($_SESSION['product_batch']['selected_product_ids']);
  $form = array();
  $buttons = array();
  $buttons['enable'] = array(
    '#type' => 'submit',
    '#value' => t('Enable selected products'),
    '#attributes' => array(
      'class' => array('uk-margin-small-right'),
      'data-ks' => 'CTRL+E',
    ),
    '#batch_action' => 'status',
    '#status' => AVPRODUCTS_STATUS_ACTIVE,
  );
  $buttons['disable'] = array(
    '#type' => 'submit',
    '#value' => t('Disable selected products'),
    '#attributes' => array(
      'class' => array('uk-margin-small-right'),
      'data-ks' => 'CTRL+D',
    ),
    '#batch_action' => 'status',
    '#status' => AVPRODUCTS_STATUS_DISABLED,
  );
  $buttons['update_vendor_id'] = array(
    '#type' => 'submit',
    '#value' => t('Update Preferred Vendor of selected products'),
    '#attributes' => array(
      'class' => array('uk-margin-small-right'),
      'data-ks' => 'CTRL+D',
    ),
    '#batch_action' => 'field',
    '#field_parents' => array('info', 'vendor_id'),
  );
  $form['buttons'] = $buttons;
  $form['hr'] = array('#markup' => '<hr />');
  $temp = array();
  $product_list_form = avreports_product_list_form($temp, $form_state, FALSE, TRUE, 'onhand');
  $form['product_list'] = $product_list_form;
//$form['#validate'][] = 'avproducts_product_batch_update_form_validate';

  return $form;
}

function avproducts_product_batch_update_form_validate($form, &$form_state) {
  $triggering_element = $form_state['triggering_element'];
  $ids = empty($form_state['values']['table']) ? array() : $form_state['values']['table'];
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

  $_SESSION['product_batch']['selected_product_ids'] = $selected;
  if ($triggering_element['#batch_action'] == 'status' && isset($triggering_element['#status'])) {
    drupal_goto('av/products/batch-update/status/' . $triggering_element['#status']);
  }
  elseif ($triggering_element['#batch_action'] == 'field') {
    drupal_goto('av/products/batch-update/field/' . implode('-', $triggering_element['#field_parents']));
  }
}

/**
 * Menu callback: confirm product status change.
 * @param $form
 * @param $form_state
 * @param object $row
 * @param $status
 *
 * @return array
 */
function avproducts_product_batch_change_status_confirm_form($form, &$form_state, $status) {
  $selected_ids = empty($_SESSION['product_batch']['selected_product_ids']) ? array() : $_SESSION['product_batch']['selected_product_ids'];
  if (empty($selected_ids)) {
    drupal_goto();
  }
  switch ($status) {
    case AVCLIENT_STATUS_DISABLED:
      $prompt = t('Are you sure you want to disable the %num product/s you selected?', array('%num' => count($selected_ids)));
      break;
    case AVCLIENT_STATUS_ACTIVE:
      $prompt = t('Are you sure you want to enable the %num product/s you selected?', array('%num' => count($selected_ids)));
      break;
    default:
      $prompt = 'Are you sure?';
      break;
  }

  $form_state['status'] = $status;
  return confirm_form(array(), $prompt, 'av/products/batch-update', '');
}

function avproducts_product_batch_change_status_confirm_form_submit($form, &$form_state) {
  $selected_ids = empty($_SESSION['product_batch']['selected_product_ids']) ? array() : $_SESSION['product_batch']['selected_product_ids'];
  unset($_SESSION['product_batch']['selected_product_ids']);
  $status = isset($form_state['status']) ? $form_state['status'] : '';
  if (avproducts_product_change_status($selected_ids, $status)) {
    drupal_set_message('Status changed for selected products.');
  }
  $form_state['redirect'] = 'av/products/batch-update';
}


function avproducts_product_batch_update_field_confirm_form($form, &$form_state, $field_parents) {
  $selected_ids = empty($_SESSION['product_batch']['selected_product_ids']) ? array() : $_SESSION['product_batch']['selected_product_ids'];
  if (empty($selected_ids)) {
    drupal_goto();
  }
  $field_parents = explode('-', $field_parents);
  $prompt = t('Enter new field value for the %num product/s you selected.', array('%num' => count($selected_ids)));

  $temp = array();
  $products_edit_form = avproducts_product_form($temp, $temp, NULL);

  $field = drupal_array_get_nested_value($products_edit_form, $field_parents);
  $field_name = array_pop($field_parents);
  $form[$field_name] = $field;
  $form_state['field_name'] = $field_name;
  return confirm_form($form, $prompt, 'av/products/batch-update', '', 'Update');
}

function avproducts_product_batch_update_field_confirm_form_submit($form, &$form_state) {
  $selected_ids = empty($_SESSION['product_batch']['selected_product_ids']) ? array() : $_SESSION['product_batch']['selected_product_ids'];
  unset($_SESSION['product_batch']['selected_product_ids']);

  $field_name = isset($form_state['field_name']) ? $form_state['field_name'] : '';
  $field_value = !isset($form_state['values'][$field_name]) ? NULL : $form_state['values'][$field_name];
  if (!is_null($field_value)) {
    if (db_query('UPDATE {avtbl_products} SET ' . check_plain($field_name) . ' = :field_value WHERE id IN (:ids)', array(
      ':field_value' => $field_value,
      ':ids' => $selected_ids,
    ))) {
      drupal_set_message('Selected products updated.');
    }
  }
  $form_state['redirect'] = 'av/products/batch-update';
}
