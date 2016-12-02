<?php

/**
 * @file
 * Conditional logic and data processing for the UIkit sub-theme.
 *
 * The functions below are just examples of the most common functions a
 * sub-theme can implement. Feel free to remove these functions or implement
 * other functions.
 */

/**
 * Implements template_preprocess_html().
 */
function uikit_av_preprocess_html(&$variables) {
}

/**
 * Implements template_process_html().
 */
function uikit_av_process_html(&$variables) {
}

/**
 * Implements template_preprocess_page().
 */
function uikit_av_preprocess_page(&$variables) {

}

/**
 * Implements template_process_page().
 */
function uikit_av_process_page(&$variables) {
}

/**
 * Implements template_preprocess_node().
 */
function uikit_av_preprocess_node(&$variables) {
}

/**
 * Implements hook_process_HOOK() for node.tpl.php.
 */
function uikit_av_process_node(&$variables) {
}

/**
 * Implements template_preprocess_region().
 */
function uikit_av_preprocess_region(&$variables) {
}

/**
 * Implements hook_process_HOOK() for region.tpl.php.
 */
function uikit_av_process_region(&$variables) {
}

/**
 * Implements template_preprocess_block().
 */
function uikit_av_preprocess_block(&$variables) {
}

/**
 * Implements hook_process_HOOK() for block.tpl.php.
 */
function uikit_av_process_block(&$variables) {
}

/**
 * Overrides theme_textfield().
 * @param $variables
 * @return string
 */
function uikit_av_textfield($variables) {
  $element = $variables['element'];
  $element['#attributes']['type'] = 'text';
  element_set_attributes($element, array('id', 'name', 'value', 'size', 'maxlength'));
  $classes = array('form-text'/*, 'uk-width-1-1'*/);
  if (isset($element['#parents']) && form_get_error($element)) {
    $classes[] = 'uk-form-danger';
  }
  _form_set_class($element, $classes);

  $extra = '';
  if ($element['#autocomplete_path'] && !empty($element['#autocomplete_input'])) {
    drupal_add_library('system', 'drupal.autocomplete');
    $element['#attributes']['class'][] = 'form-autocomplete';

    $attributes = array();
    $attributes['type'] = 'hidden';
    $attributes['id'] = $element['#autocomplete_input']['#id'];
    $attributes['value'] = $element['#autocomplete_input']['#url_value'];
    $attributes['disabled'] = 'disabled';
    $attributes['class'][] = 'autocomplete';
    $extra = '<input' . drupal_attributes($attributes) . ' />';
  }

  $output = '<input' . drupal_attributes($element['#attributes']) . ' />';

  // Add element icon if set.
  if (isset($element['#icon_key'])) {
    $output = '<div class="uk-form-icon"><i class="uk-icon-' . $element['#icon_key'] . '"></i>' . $output . '</div>';
  }
  return $output . $extra;
}
