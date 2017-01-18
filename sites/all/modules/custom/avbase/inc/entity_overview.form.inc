<?php
module_load_include('inc', 'avbase', 'inc/entity_overview.handlers');

/**
 * Search form.
 * @param $form_state
 * @param $entity_name
 *
 * @return mixed
 * @throws Exception
 */
function avbase_entity_overview_search_form(&$form_state, $entity_name) {
  $search_string = empty($form_state['values']['search_text']) ? '' : $form_state['values']['search_text'];

  $form['filter']['search_text'] = array(
    '#id' => 'entity-search-text',
    '#type' => 'textfield',
    '#icon_key' => 'search',
  );

  $form['filter']['search'] = array(
    '#id' => 'entity-search-btn',
    '#type' => 'button',
    '#value' => t('Search'),
    '#attributes' => array(
      'class' => array('uk-hidden')
    ),
    '#ajax' => array(
      'callback' => 'avbase_entity_overview_search_form_ajax',
      'wrapper' => 'entity-list-wrapper',
      'effect' => 'none',
      'event' => 'click',
      'progress' => array(),
    ),
  );

  //
  $header = array(
    array('data' => 'ID', 'class' => array('uk-hidden')),
    array('data' => 'Name / Company'),
    '&nbsp;',
  );


  //$query = db_select("avtbl_$entity_name", 'entity')
  //  ->fields('entity')
  //  ->extend('PagerDefault')
  //  ->extend('TableSort')
  //  ->limit(15)
  //  ->orderByHeader($header);
  //
  //if (!empty($search_string)) {
  //  $db_or = db_or();
  //  $db_or->condition('entity.display_name', '%' . db_like($search_string) . '%', 'LIKE');
  //  $db_or->condition('entity.company', '%' . db_like($search_string) . '%', 'LIKE');
  //  $db_or->condition('entity.first_name', '%' . db_like($search_string) . '%', 'LIKE');
  //  $db_or->condition('entity.last_name', '%' . db_like($search_string) . '%', 'LIKE');
  //  $query->condition($db_or);
  //}
  //
  //$result = $query->execute();

  $where = array(1);
  $args = array();
  if (!empty($search_string)) {
    $where_or = array(
      "av{$entity_name}.display_name LIKE :string",
      "av{$entity_name}.company LIKE :string",
      "av{$entity_name}.first_name LIKE :string",
      "av{$entity_name}.last_name LIKE :string",
    );
    $where_or_str = '(' . implode(' OR ', $where_or) . ')';
    $where = array($where_or_str);
    $args = array(':string' => '%' . db_like($search_string) . '%');
  }

  $result = array();
  switch ($entity_name) {
    case 'vendors':
      $result = avvendor_query_load($where, $args, '12');
      break;
    case 'customers':
      $result = avcustomer_query_load($where, $args, '12');
      break;
  }

  $action_links_all = array();
  $loaded_entities = array();
  $rows = array();
  foreach ($result as $row) {
    $menu_prefix = "av/$entity_name/" . $row->id;
    $linked_name = l($row->display_name, $menu_prefix . '/view');
    $rows[] = array(
      'data' => array(
        'id' => array('data' => $row->id, 'class' => array('uk-hidden')),
        'display_name' => $linked_name . '<span class="uk-text-muted"> / ' . check_plain($row->company) .  '</span>',
      ),
      'class' => array('uk-table-middle'),
      'data-entity-id' => $row->id,
    );
    $loaded_entities[] = $row;
  }

  $table_attributes = array(
    'id' => "entity-list-table",
    'class' => array('uk-margin-top', 'uk-table-hover'),
  );
  if (empty($rows)) {
    $table_markup = '<div class="uk-margin-top uk-text-muted">No match found</div>';
  }
  else {
    $table_markup = theme('table', array('header' => array(), 'rows' => $rows, 'attributes' => $table_attributes, 'action_links_all' => $action_links_all));
  }
  $form['entity_list'] = array(
    '#type' => 'container',
    '#attributes' => array('id' => 'entity-list-wrapper')
  );
  $form['entity_list']['table']['#markup'] = $table_markup . '<!--Do not remove me-->';
  //$form['entity_list']['pager']['#markup'] = theme('pager', array('tags' => array()));

  $form['clicked_entity_id'] = array(
    '#type' => 'hidden',
    '#attributes' => array('id' => 'clicked-entity-id'),
  );
  $form['load_entity'] = array(
    '#id' => 'load-entity-btn',
    '#value' => 'Load entity',
    '#type' => 'button',
    '#attributes' => array('class' => array('uk-hidden')),
    '#ajax' => array(
      'callback' => 'avbase_entity_overview_search_form_ajax',
      'wrapper' => 'entity-info-wrapper',
      'effect' => 'none',
      'event' => 'click',
      'progress' => array(),
    ),
  );

  $k = array_rand($loaded_entities);
  $form_state['#random_entity'] = empty($loaded_entities[$k]) ? array() : $loaded_entities[$k];
  return $form;
}

/**
 * Entity Overview buttons
 * @param $form_state
 * @param $entity_name
 *
 * @return array
 */
function avbase_entity_overview_buttons_form(&$form_state, $entity_name, $row) {
  $entity_id = empty($row->id) ? '' : $row->id;

  $transaction_buttons = array();
  switch ($entity_name) {
    case 'vendors':
      $transaction_buttons[] = l(t('Purchase Order'), 'av/po/new', array('query' => array('id' => $entity_id)));
      $transaction_buttons[] = l(t('Receive items'), 'av/gr/new', array('query' => array('id' => $entity_id)));
      break;
    case 'customers':
      $transaction_buttons[] = l(t('Sales Order'), 'av/sales-order/new');
      $transaction_buttons[] = l(t('Delivery'), 'av/delivery/new');
      break;
  }

  $transaction_button_list_markup = '<li>' . implode('</li><li>', $transaction_buttons) . '</li>';
  $transaction_buttons_markup = <<<HTML
<!-- This is the container enabling the JavaScript -->
<div class="uk-button-dropdown uk-text-left" data-uk-dropdown>
  <!-- This is the button toggling the dropdown -->
  <a class="uk-button uk-button-primary">New transaction<i class="uk-margin-small-left uk-icon-caret-down"></i></a>
  <!-- This is the dropdown -->
  <div class="uk-dropdown uk-dropdown-small">
    <ul class="uk-nav uk-nav-dropdown">
      $transaction_button_list_markup
    </ul>
  </div>
</div>
HTML;

  $entity_details = avbase_entity_details();
  $entity_details = $entity_details[$entity_name];
  $other_buttons = array(
    'new_entity' => l(t('New') . ' ' . $entity_details['title'], "av/$entity_name/add", array('attributes' => array(
      'class' => array('uk-button uk-button-success uk-margin-small-left')
    ))),
  );
  if ($entity_id) {
    $other_buttons['edit_entity'] = l(t('Edit'), "av/$entity_name/{$row->id}/edit", array('attributes' => array(
      'class' => array('uk-button uk-button-success uk-margin-small-left')
    )));
  }

  $form = array(
    'new_transaction' => array('#markup' => $transaction_buttons_markup),
  );
  foreach ($other_buttons as $k => $btn) {
    $form[$k]['#markup'] = $btn;
  }
  return $form;
}

/**
 * Entity Overview entity info form.
 * @param $form_state
 * @param $entity_name
 *
 * @return array
 */
function avbase_entity_overview_info_form(&$form_state, $entity_name) {
  module_load_include('inc', "av$entity_name", 'inc/crud');
  $entity_id = empty($form_state['values']['clicked_entity_id']) ? NULL : $form_state['values']['clicked_entity_id'];

  $form = array();
  $row = array();
  $entity_form = array();
  $random_row = $form_state['#random_entity'];
  switch ($entity_name) {
    case 'vendors':
      $row = avvendor_load($entity_id);
      $row = empty($row) ? $random_row : $row;
      $entity_form = avvendors_vendor_form($form, $form_state, $row, TRUE);
      $entity_form['#theme'] = 'avvendors_vendor_form';
      break;
    case 'customers':
      $row = avcustomer_load($entity_id);
      $row = empty($row) ? $random_row : $row;
      $entity_form = avcustomers_customer_form($form, $form_state, $row, TRUE);
      $entity_form['#theme'] = 'avcustomers_customer_form';
      break;
  }




  $form['entity_info_container'] = array(
    '#type' => 'container',
    '#attributes' => array('id' => 'entity-info-wrapper'),
  );
  $form['entity_info_container']['buttons'] = avbase_entity_overview_buttons_form($form_state, $entity_name, $row);
  $form['entity_info_container']['buttons']['#prefix'] = '<div style="position: absolute; top: -52px; right: 0;">';
  $form['entity_info_container']['buttons']['#suffix'] = '</div>';

  $form['entity_info_container']['entity_form'] = $entity_form;
  return $form;
}

