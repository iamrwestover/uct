<?php

function avbase_entity_overview_search_form_ajax($form, $form_state) {
  $trigger = $form_state['triggering_element'];
  switch ($trigger['#id']) {
    case 'entity-search-btn':
      return $form['search']['entity_list'];
      break;

    case 'load-entity-btn':
      return $form['entity_info'];
      break;
  }
}
