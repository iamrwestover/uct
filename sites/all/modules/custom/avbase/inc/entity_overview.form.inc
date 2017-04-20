<?php
module_load_include('inc', 'avbase', 'inc/entity_overview.handlers');



/**
 * Search form.
 * @param $form_state
 * @param $entity_type
 *
 * @return mixed
 * @throws Exception
 */
function avbase_entity_overview_search_form(&$form_state, $entity_type = NULL) {
  $search_string = empty($form_state['values']['search_text']) ? '' : $form_state['values']['search_text'];

  $form['filter']['search_text'] = array(
    '#id' => 'entity-search-text',
    '#type' => 'textfield',
    '#attributes' => array(
      //'placeholder' => t('search')
    ),
    '#icon_key' => 'search',
  );

  $form['filter']['search'] = array(
    '#id' => 'entity-search-btn',
    '#type' => 'button',
    '#value' => t('Search entity'),
    //'#submit' => array('avbase_entity_overview_search_form_submit'),
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

  $where = array();
  $args = array();
  $where[] = "client_type = :client_type";
  $args[':client_type'] = $entity_type;

  if (!empty($search_string)) {
    $where_or = array();
    $string_arr = explode(' ', $search_string);
    $like_where = array();
    foreach ($string_arr as $k => $s) {
      $like_where[] = 'display_name LIKE :string_' . $k;
      $args[':string_' . $k] = '%' . db_like($s) . '%';
    }
    $where_or[] = '(' . implode(' AND ', $like_where) . ')';
    $where_or_str = '(' . implode(' OR ', $where_or) . ')';
    $where[] = $where_or_str;
  }

  // Build query.
  $query = "SELECT * FROM avtbl_clients WHERE " . implode(' AND ', $where) . " ORDER BY created DESC LIMIT 20";
  $rs = db_query($query, $args);
  $ordered_rows = avbase_order_db_rows_by_best_match($rs, 'display_name', $search_string);

  $ctd = avclients_client_types($entity_type);
  $menu_path = $ctd['base_path'];

  $action_links_all = array();
  $loaded_entities = array();
  $rows = array();
  $index = 0;
  foreach ($ordered_rows as $row) {
    $index++;
    $status = isset($row->status) ? $row->status : NULL;

    $row->address_string = avclients_address_string_format($row);
    $menu_prefix = "$menu_path/" . $row->id;
    $linked_name = l($row->display_name, $menu_prefix . '/view');
    $rows[$index] = array(
      'data' => array(
        'id' => array('data' => $row->id, 'class' => array('uk-hidden')),
        'display_name' => $linked_name . '<div class="uk-text-muted uk-text-small">' . nl2br($row->address_string) .  '</div>',
      ),
      'class' => array('uk-table-middle'),
      'data-entity-id' => $row->id,
    );

    $csd = avclients_client_status_details($status);
    $color_class = empty($csd['color_class']) ? '' : $csd['color_class'];
    $rows[$index]['class'][] = $color_class;
    $loaded_entities[] = $row;
  }

  $table_attributes = array(
    'id' => "entity-list-table",
    'class' => array('uk-margin-top uk-table-condensed uk-table-hover'),
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
 * @param object $row
 *
 * @return array
 */
function avbase_entity_overview_buttons_form(&$form_state, $row, $entity_type) {
  $ctd = avclients_client_types($entity_type);
  $menu_path = $ctd['base_path'];
  $entity_title = $ctd['name'];
  $other_buttons = array();
  if (user_access(constant("AVCLIENTS_PERM_CREATE_{$ctd['type']}"))) {
    $other_buttons['new_entity'] = l(t('New') . ' ' . $entity_title, "$menu_path/add", array('attributes' => array(
      'class' => array('uk-button uk-button-success uk-margin-small-left')
    )));
  }

  if (!empty($row->id)) {
    $entity_id = empty($row->id) ? '' : $row->id;
    $transaction_buttons = array();
    $transaction_buttons[] = l(t('Sales Order'), 'av/sales-order/new');
    $transaction_buttons[] = l(t('Delivery'), 'av/delivery/new');



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


    if (user_access(constant("AVCLIENTS_PERM_CREATE_{$ctd['type']}"))) {
      $other_buttons['new_entity'] = l(t('New') . ' ' . $entity_title, "$menu_path/add", array(
        'attributes' => array(
          'class' => array('uk-button uk-button-success uk-margin-small-left')
        )
      ));
    }

    if ($entity_id && user_access(constant("AVCLIENTS_PERM_EDIT_{$ctd['type']}"))) {
      $other_buttons['edit_entity'] = l(t('Edit'), "$menu_path/{$row->id}/edit", array(
        'attributes' => array(
          'class' => array('uk-button uk-button-success uk-margin-small-left')
        )
      ));
    }
  }

  $form = array(
    //'new_transaction' => array('#markup' => $transaction_buttons_markup),
  );
  foreach ($other_buttons as $k => $btn) {
    $form[$k]['#markup'] = $btn;
  }
  return $form;
}

/**
 * Entity Overview entity info form.
 * @param $form_state
 * @param $entity_type
 *
 * @return array
 */
function avbase_entity_overview_info_form(&$form_state, $entity_type = NULL) {
  module_load_include('inc', "avclients", 'inc/crud');
  $entity_id = empty($form_state['values']['clicked_entity_id']) ? NULL : $form_state['values']['clicked_entity_id'];

  $form = array();
  $random_row = $form_state['#random_entity'];
  $row = avclient_load($entity_id);
  $row = empty($row) ? $random_row : $row;
  $entity_form = avclients_client_form($form, $form_state, $entity_type, $row, TRUE);
  $entity_form['#theme'] = 'avclients_client_form';




  $form['entity_info_container'] = array(
    '#type' => 'container',
    '#attributes' => array('id' => 'entity-info-wrapper'),
  );
  $form['entity_info_container']['buttons'] = avbase_entity_overview_buttons_form($form_state, $row, $entity_type);
  $form['entity_info_container']['buttons']['#prefix'] = '<div style="position: absolute; top: -52px; right: 0;">';
  $form['entity_info_container']['buttons']['#suffix'] = '</div>';

  $form['entity_info_container']['entity_form'] = $entity_form;
  return $form;
}

