<?php
$icon_html = '';
if (!empty($options['icon_key'])) {
  $icon_classes = isset($options['icon_classes']) ? (' ' . implode(' ', $options['icon_classes'])) : '';
  $icon_html = '<i class="uk-icon-' . $options['icon_key'] . $icon_classes . '"></i>';
}
print '<a href="' . check_plain(url($variables['path'], $variables['options'])) . '"' . drupal_attributes($variables['options']['attributes']) . '>' . $icon_html . ($variables['options']['html'] ? $variables['text'] : check_plain($variables['text'])) . '</a>';