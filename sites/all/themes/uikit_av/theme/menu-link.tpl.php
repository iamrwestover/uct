<?php
$element = $variables['element'];
$sub_menu = '';

// Render as nav header if set.
if (strstr($element['#title'], '----')) {
  $element['#title'] = str_replace('----', '', $element['#title']);
  print '<li class="uk-nav-header">' . $element['#title'] . '</li>';
}
else {
  if ($element['#below']) {
    $sub_menu = drupal_render($element['#below']);
  }
  $output = l($element['#title'], $element['#href'], $element['#localized_options']);
  print '<li' . drupal_attributes($element['#attributes']) . '>' . $output . $sub_menu . "</li>\n";
}
