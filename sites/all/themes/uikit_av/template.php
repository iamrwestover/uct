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
  $path_alias = drupal_get_path_alias();
  $paths = array(
    'av/vendors',
    'av/customers',
  );
  if (in_array($path_alias, $paths)) {
    $variables['theme_hook_suggestions'][] = 'region__bare';
  }
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
  $element['#attributes']['type'] = isset($element['#special_element_type']) ? $element['#special_element_type'] : 'text';
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
  return $output . $extra;
}

/**
 * Overrides theme_select().
 */
function uikit_av_select($variables) {
  $element = $variables['element'];
  element_set_attributes($element, array('id', 'name', 'size'));
  $classes = array('form-select');
  if (isset($element['#parents']) && form_get_error($element)) {
    $classes[] = 'uk-form-danger';
  }
  _form_set_class($element, $classes);

  if (!empty($element['#avbase_payment_terms_js'])) {
    drupal_add_library('avbase', 'avbase.paymentTerms');
    $element['#attributes']['class'][] = 'avbase-payment-terms-js';
    $element['#attributes']['data-prevent-children-hide'] = $element['#avbase_payment_terms_js']['preventChildrenHide'];
  }

  return '<select' . drupal_attributes($element['#attributes']) . '>' . form_select_options($element) . '</select>';
}

/**
 * Process variables for user-picture.tpl.php.
 *
 * The $variables array contains the following arguments:
 * - $account: A user, node or comment object with 'name', 'uid' and 'picture'
 *   fields.
 *
 * @see user-picture.tpl.php
 */
function uikit_av_preprocess_user_picture(&$variables) {
  $variables['user_picture'] = '';
  if (variable_get('user_pictures', 0)) {
    $account = $variables['account'];
    if (!empty($account->picture)) {
      // @TODO: Ideally this function would only be passed file objects, but
      // since there's a lot of legacy code that JOINs the {users} table to
      // {node} or {comments} and passes the results into this function if we
      // a numeric value in the picture field we'll assume it's a file id
      // and load it for them. Once we've got user_load_multiple() and
      // comment_load_multiple() functions the user module will be able to load
      // the picture files in mass during the object's load process.
      if (is_numeric($account->picture)) {
        $account->picture = file_load($account->picture);
      }
      if (!empty($account->picture->uri)) {
        $filepath = $account->picture->uri;
      }
    }
    elseif (variable_get('user_picture_default', '')) {
      $filepath = variable_get('user_picture_default', '');
    }
    if (isset($filepath)) {
      $alt = t("@user's picture", array('@user' => format_username($account)));
      // If the image does not have a valid Drupal scheme (for eg. HTTP),
      // don't load image styles.
      if (module_exists('image') && file_valid_uri($filepath) && $style = variable_get('user_picture_style', '')) {
        $variables['user_picture'] = theme('image_style', array('style_name' => $style, 'path' => $filepath, 'alt' => $alt, 'title' => $alt));
      }
      else {
        $variables['user_picture'] = theme('image', array('attributes' => array('class' => array('uk-thumbnail')), 'path' => $filepath, 'alt' => $alt, 'title' => $alt));
      }
      if (!empty($account->uid) && user_access('access user profiles')) {
        $attributes = array('attributes' => array('title' => t('View user profile.')), 'html' => TRUE);
        $variables['user_picture'] = l($variables['user_picture'], "user/$account->uid", $attributes);
      }
    }
  }
}

/**
 * Override theme_radios().
 * @param $variables
 *
 * @return string
 */
function uikit_av_radios($variables) {
  $element = $variables['element'];
  $attributes = array();
  if (isset($element['#id'])) {
    $attributes['id'] = $element['#id'];
  }


  $element_classes = empty($element['#attributes']['class']) ? array() : $element['#attributes']['class'];
  if (is_numeric($k = array_search('uk-button-group', $element_classes))) {
    unset($element['#attributes']['class'][$k]);
    $attributes['class'] = 'uk-button-group';
    $attributes['data-uk-button-radio'] = "{activeClass: 'uk-button-success'}";
  }
  else {
    $attributes['class'] = 'form-radios';
  }

  if (!empty($element['#attributes']['class'])) {
    $attributes['class'] .= ' ' . implode(' ', $element['#attributes']['class']);
  }
  if (isset($element['#attributes']['title'])) {
    $attributes['title'] = $element['#attributes']['title'];
  }
  return '<div' . drupal_attributes($attributes) . '>' . (!empty($element['#children']) ? $element['#children'] : '') . '</div>';

  //<div class="uk-button-group" data-uk-button-radio="{activeClass: 'uk-button-success'}">
  //  <label class="uk-button">
  //    <input type="radio" name="gender" value="0" style="display:none;"/>
  //    Leave unchanged
  //</label>
  //  <label class="uk-button">
  //    <input type="radio" name="gender" value="1" style="display:none;"/>
  //    Update <strong>cost</strong>
  //  </label>
  //  <label class="uk-button">
  //    <input type="radio" name="gender" value="1" style="display:none;"/>
  //    Update both <strong>cost and sales price</strong>
  //  </label>
  //</div>
}

/**
 * Override theme_radio().
 * @param $variables
 *
 * @return string
 */
function uikit_av_radio($variables) {
  $element = $variables['element'];
  $element['#attributes']['type'] = 'radio';
  element_set_attributes($element, array('id', 'name', '#return_value' => 'value'));

  if (isset($element['#return_value']) && $element['#value'] !== FALSE && $element['#value'] == $element['#return_value']) {
    $element['#attributes']['checked'] = 'checked';
  }

  $element_classes = empty($element['#attributes']['class']) ? array() : $element['#attributes']['class'];
  if (is_numeric($k = array_search('uk-button-group', $element_classes))) {
    unset($element['#attributes']['class'][$k]);
    $element['#attributes']['class'][] = 'uk-hidden';
  }

  _form_set_class($element, array('form-radio'));

  return '<input' . drupal_attributes($element['#attributes']) . ' />';
}
