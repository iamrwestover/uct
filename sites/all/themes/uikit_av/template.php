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
//function uikit_av_textfield($variables) {
//  $element = $variables['element'];
//  $element['#attributes']['type'] = isset($element['#special_element_type']) ? $element['#special_element_type'] : 'text';
//  element_set_attributes($element, array('id', 'name', 'value', 'size', 'maxlength'));
//  $classes = array('form-text'/*, 'uk-width-1-1'*/);
//  if (isset($element['#parents']) && form_get_error($element)) {
//    $classes[] = 'uk-form-danger';
//  }
//  _form_set_class($element, $classes);
//
//  $extra = '';
//  if ($element['#autocomplete_path'] && !empty($element['#autocomplete_input'])) {
//    drupal_add_library('system', 'drupal.autocomplete');
//    $element['#attributes']['class'][] = 'form-autocomplete';
//    $element['#icon_key'] = 'circle-o-notch';
//
//    $attributes = array();
//    $attributes['type'] = 'hidden';
//    $attributes['id'] = $element['#autocomplete_input']['#id'];
//    $attributes['value'] = $element['#autocomplete_input']['#url_value'];
//    $attributes['disabled'] = 'disabled';
//    $attributes['class'][] = 'autocomplete';
//    $extra = '<input' . drupal_attributes($attributes) . ' />';
//  }
//
//  if (!empty($element['#avbase_autocomplete'])) {
//    drupal_add_library('avbase', 'avbase.autocompleteActions');
//    $element['#attributes']['class'][] = 'avbase-autocomplete-actions';
//    $element['#attributes']['data-avbase-entity-group'] = $element['#avbase_autocomplete']['entity_group'];
//  }
//
//  if (!empty($element['#av_dropdown'])) {
//    $element['#attributes']['readonly'] = 'readonly';
//  }
//  $output = '<input' . drupal_attributes($element['#attributes']) . ' />';
//
//  // Add element icon if set.
//  if (isset($element['#icon_key'])) {
//    $output = '<div class="uk-form-icon uk-width-1-1"><i class="uk-icon-' . $element['#icon_key'] . '"></i>' . $output . '</div>';
//  }
//
//  if (!empty($element['#av_dropdown'])) {
//    $wrapper_id = $element['#id'] . '-wrapper';
//    $output = <<<HTML
//<div id="$wrapper_id" class="uk-autocomplete uk-form av-dropdown">
//    <div class="uk-form-icon uk-form-icon-flip"><i class="uk-icon-caret-down"></i>$output</div>
//    <script type="text/autocomplete">
//        <ul class="uk-nav uk-nav-autocomplete uk-autocomplete-results">
//            {{~items}}
//            <li data-value="{{ \$item.value }}" data-qtyperuom="{{ \$item.qtyPerUOM }}">
//                <a>
//                    {{ \$item.title }}
//                    <div class="uk-text-muted">{{{ \$item.text }}}</div>
//                </a>
//            </li>
//            {{/items}}
//        </ul>
//    </script>
//</div>
//HTML;
//  }
//  return $output . $extra;
//}

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
  $attributes = empty($element['#attributes']) ? array() : $element['#attributes'];
  $classes = empty($element['#attributes']['class']) ? array() : $element['#attributes']['class'];
  if (isset($element['#id'])) {
    $attributes['id'] = $element['#id'];
  }
  $attributes['class'] = 'form-radios';
  if (!empty($element['#attributes']['class'])) {
    $attributes['class'] .= ' ' . implode(' ', $element['#attributes']['class']);
  }
  if (isset($element['#attributes']['title'])) {
    $attributes['title'] = $element['#attributes']['title'];
  }

  if (in_array('uk-button-group', $classes)) {
    $attributes['data-uk-button-radio'] = "{activeClass: 'uk-button-success'}";
  }

  return '<div' . drupal_attributes($attributes) . '>' . (!empty($element['#children']) ? $element['#children'] : '') . '</div>';
}
//
///**
// * Override theme_radio().
// * @param $variables
// *
// * @return string
// */
//function uikit_av_radio($variables) {
//  $element = $variables['element'];
//  $element['#attributes']['type'] = 'radio';
//  element_set_attributes($element, array('id', 'name', '#return_value' => 'value'));
//
//  if (isset($element['#return_value']) && $element['#value'] !== FALSE && $element['#value'] == $element['#return_value']) {
//    $element['#attributes']['checked'] = 'checked';
//  }
//  _form_set_class($element, array('form-radio'));
//dpm($element);
//  $element['#xxx'] = 'sss';
//
//  return '<input' . drupal_attributes($element['#attributes']) . ' />';
//}
//
///**
// * Override theme_checkbox().
// * @param $variables
// *
// * @return string
// */
//function uikit_av_checkbox($variables) {
//  $element = $variables['element'];
//  $element['#attributes']['type'] = 'checkbox';
//  element_set_attributes($element, array('id', 'name', '#return_value' => 'value'));
//
//  // Unchecked checkbox has #value of integer 0.
//  if (!empty($element['#checked'])) {
//    $element['#attributes']['checked'] = 'checked';
//  }
//  _form_set_class($element, array('form-checkbox'));
//
//  if (!empty($element['#button_checkbox'])) {
//    $element['#attributes']['class'][] = 'uk-hidden';
//  }
//
//  return '<input' . drupal_attributes($element['#attributes']) . ' />';
//}


/**
 * Implements hook_preprocess_HOOK() for theme_radios().
 * @param $variables
 */
//function uikit_av_preprocess_radios(&$variables) {
//  $element = &$variables['element'];
//  $element['#attributes']['data-uk-button-radio'] = "{activeClass: 'uk-button-success'}";
//  $element['#attributes']['style'][] = 'background: red;';
//}

/**
 * Implements hook_preprocess_HOOK() for theme_radio().
 * @param $variables
 */
function uikit_av_preprocess_radio(&$variables) {
  $element = &$variables['element'];
  $classes = empty($element['#attributes']['class']) ? array() : $element['#attributes']['class'];
  if (in_array('uk-button-group', $classes)) {
    $element['#attributes']['class'][] = 'uk-hidden';
  }
}

/**
 * Implements hook_preprocess_HOOK() for theme_checkbox().
 * @param $variables
 */
function uikit_av_preprocess_checkbox(&$variables) {
  $element = &$variables['element'];
  if (!empty($element['#button_checkbox'])) {
    //$element['#attributes']['class'][] = 'uk-hidden';
  }
}


/**
 * Returns HTML for the administer permissions page.
 *
 * @param $variables
 *   An associative array containing:
 *   - form: A render element representing the form.
 *
 * @ingroup themeable
 */
function uikit_av_user_admin_permissions($variables) {
  global $user;
  $form = $variables['form'];

  $roles = user_roles();
  foreach (element_children($form['permission']) as $key) {
    $row = array();
    // Module name
    if (is_numeric($key)) {
      $row[] = array('data' => drupal_render($form['permission'][$key]), 'class' => array('module'), 'id' => 'module-' . $form['permission'][$key]['#id'], 'colspan' => count($form['role_names']['#value']) + 1);
    }
    else {
      // Permission row.
      $row[] = array(
        'data' => drupal_render($form['permission'][$key]),
        'class' => array('permission'),
      );
      foreach (element_children($form['checkboxes']) as $rid) {
        $form['checkboxes'][$rid][$key]['#title'] = $roles[$rid] . ': ' . $form['permission'][$key]['#markup'];
        $form['checkboxes'][$rid][$key]['#title_display'] = 'invisible';
        $row[] = array('data' => drupal_render($form['checkboxes'][$rid][$key]), 'class' => array('checkbox uk-text-center'));
      }
    }
    $rows[] = $row;
  }
  $header[] = (t('Permission'));
  foreach (element_children($form['role_names']) as $rid) {
    $header[] = array('data' => drupal_render($form['role_names'][$rid]), 'class' => array('checkbox uk-text-center'));
  }
  $output = $user->uid == 1 ? theme('system_compact_link') : '';
  $output .= theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => array('id' => 'permissions', 'class' => array('uk-table-hover'))));
  $output .= drupal_render_children($form);
  return $output;
}

/**
 * Theme the quick backup form.
 */
function uikit_av_backup_migrate_ui_manual_quick_backup_form_inline($form) {
  drupal_add_css(drupal_get_path('module', 'backup_migrate') .'/backup_migrate.css');

  $form = $form['form'];

  // Remove the titles so that the pulldowns can be displayed inline.
  unset($form['quickbackup']['source_id']['#title']);
  unset($form['quickbackup']['destination']['destination_id']['#title']);
  unset($form['quickbackup']['destination']['destination_id']['#description']);
  unset($form['quickbackup']['profile_id']['#title']);
  unset($form['quickbackup']['destination']['copy_destination']['copy_destination_id']['#title']);

  $replacements = array(
    '!from' => drupal_render($form['quickbackup']['source_id']),
    '!to' => drupal_render($form['quickbackup']['destination']['destination_id']),
    '!profile' => drupal_render($form['quickbackup']['profile_id']),
    //'!submit' => drupal_render($form['quickbackup']['submit']),
  );
  $form['quickbackup']['markup'] = array(
    '#type'   => 'markup',
    "#prefix" => '<div class="container-inline backup-migrate-inline">',
    "#suffix" => '</div>',
    '#markup'  => t('Backup my !from to !to using !profile', $replacements),
  );

  $replacements = array(
    '!to' => drupal_render($form['quickbackup']['destination']['copy_destination']['copy_destination_id']),
  );
  $form['quickbackup']['destination']['copy']["#prefix"] = '<div class="container-inline backup-migrate-inline">';
  $form['quickbackup']['destination']['copy']["#suffix"] = $replacements['!to'] . '</div>';
  // This is not good translation practice as it relies on the structure of english
  // If I add the pulldown to the label, howerver, the box toggles when the pulldown is clicked.
  //$form['quickbackup']['destination']['copy']['#title']  = t('Save a copy to');

  unset($form['quickbackup']['source_id']);
  unset($form['quickbackup']['destination']['destination_id']);
  unset($form['quickbackup']['destination']['copy_destination']);
  unset($form['quickbackup']['profile_id']);
  //unset($form['quickbackup']['submit']);
  return drupal_render_children($form);
}
