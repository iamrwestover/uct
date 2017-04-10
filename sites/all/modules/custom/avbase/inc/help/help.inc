<?php

function avbase_help_home() {
  $output = '';
  $help_items = array();
  $help_items[] = array(
    'data' => l('Printing with dot matrix printers', 'av/help', array('fragment' => 'help-print')),
  );
  $item_list = array('items' => $help_items);
  $output .= theme('item_list', $item_list);

  $output .= theme('avbase_help_print', array('help_title' => 'Printing with dot matrix printers'));
  return $output;
}