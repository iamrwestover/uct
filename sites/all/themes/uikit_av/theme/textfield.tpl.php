<?php

/**
 * @file
 * Returns HTML for a textfield form element.
 *
 * Available variables:
 * - $variables['element']: An associative array containing the properties of
 *   the element. Properties used: #title, #value, #description, #size,
 *   #maxlength, #required, #attributes, #autocomplete_path.
 *
 * @see theme_textfield()
 *
 * @ingroup uikit_themeable
 */

$element = $variables['element'];
$element['#attributes']['type'] = isset($element['#special_element_type']) ? $element['#special_element_type'] : 'text';
element_set_attributes($element, array(
  'id',
  'name',
  'value',
  'size',
  'maxlength',
));
$classes = array('form-text'/*, 'uk-width-1-1'*/);
if (isset($element['#parents']) && form_get_error($element)) {
  $classes[] = 'uk-form-danger';
}
_form_set_class($element, $classes);

//$element['#attributes']['type'] = 'text';
//$element['#attributes']['size'] = '25';

$extra = '';
if ($element['#autocomplete_path'] && !empty($element['#autocomplete_input'])) {
  drupal_add_library('system', 'drupal.autocomplete');
  $element['#attributes']['class'][] = 'form-autocomplete';
  $element['#icon_key'] = 'circle-o-notch';

  $attributes = array();
  $attributes['type'] = 'hidden';
  $attributes['id'] = $element['#autocomplete_input']['#id'];
  $attributes['value'] = $element['#autocomplete_input']['#url_value'];
  $attributes['disabled'] = 'disabled';
  $attributes['class'][] = 'autocomplete';
  $extra = '<input' . drupal_attributes($attributes) . ' />';
}

if (!empty($element['#avbase_autocomplete'])) {
  drupal_add_library('avbase', 'avbase.autocompleteActions');
  $element['#attributes']['class'][] = 'avbase-autocomplete-actions';
  $element['#attributes']['data-avbase-entity-group'] = $element['#avbase_autocomplete']['entity_group'];
}

if (!empty($element['#av_dropdown'])) {
  $element['#attributes']['readonly'] = 'readonly';
}

$output = '<input' . drupal_attributes($element['#attributes']) . ' />';

// Add element icon if set.
if (isset($element['#icon_key'])) {
  $output = '<div class="uk-form-icon uk-width-1-1"><i class="uk-icon-' . $element['#icon_key'] . '"></i>' . $output . '</div>';
}

if (!empty($element['#av_dropdown'])) {
  $wrapper_id = $element['#id'] . '-wrapper';
  $output = <<<HTML
<div id="$wrapper_id" class="uk-autocomplete uk-form av-dropdown">
    <div class="uk-form-icon uk-form-icon-flip"><i class="uk-icon-caret-down"></i>$output</div>
    <script type="text/autocomplete">
        <ul class="uk-nav uk-nav-autocomplete uk-autocomplete-results">
            {{~items}}
            <li data-value="{{ \$item.value }}" data-qtyperuom="{{ \$item.qtyPerUOM }}">
                <a>
                    {{ \$item.title }}
                    <div class="uk-text-muted">{{{ \$item.text }}}</div>
                </a>
            </li>
            {{/items}}
        </ul>
    </script>
</div>
HTML;
}

print $output . $extra;
