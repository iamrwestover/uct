<?php

/**
 * @file
 * Returns HTML for a button form element.
 *
 * Available variables:
 * - $variables['element']: An associative array containing the properties of
 *   the element. Properties used: #attributes, #button_type, #name, #value.
 *
 * @see uikit_preprocess_button()
 * @see theme_button()
 *
 * @ingroup uikit_themeable
 */

$element = $variables['element'];
$element['#attributes']['type'] = 'submit';
element_set_attributes($element, array('id', 'name', 'value'));
$value = $element['#attributes']['value'];

$element['#attributes']['class'][] = 'form-' . $element['#button_type'];
if (!empty($element['#attributes']['disabled'])) {
  $element['#attributes']['class'][] = 'form-button-disabled';
}
$button_label = isset($element['#button_label']) ? $element['#button_label'] : $value;
if (isset($element['#icon_key'])) {
  $button_label = '<i class="uk-icon-' . $element['#icon_key'] . ' uk-margin-small-right"></i>' . $button_label;
}
print '<button' . drupal_attributes($element['#attributes']) . '>' . $button_label . '</button>';
