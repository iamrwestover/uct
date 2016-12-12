<?php

/**
 * @file
 * Common get functions.
 */

function uikit_get_cdn_assets($jquery_version = '2.1.4', $uikit_version = '2.26.4') {
  global $theme_key;
  $uikit_style = theme_get_setting('base_style', $theme_key);
  $uikit_css = 'uikit.min.css';

  switch ($uikit_style) {
    case 'almost-flat':
      $uikit_css = 'uikit.almost-flat.min.css';
      break;

    case 'gradient':
      $uikit_css = 'uikit.gradient.min.css';
      break;
  }

  drupal_add_css(UIKIT_SRC . "/css/$uikit_css", array(
    'type' => 'external',
    'group' => CSS_THEME,
    'every_page' => TRUE,
    'weight' => -100,
    'version' => $uikit_version,
  ));

//  drupal_add_js("//cdnjs.cloudflare.com/ajax/libs/jquery/$jquery_version/jquery.min.js", array(
//    'type' => 'external',
//    'group' => JS_THEME,
//    'every_page' => TRUE,
//    'weight' => -20,
//    'version' => $jquery_version,
//  ));

//  drupal_add_js('//cdnjs.cloudflare.com/ajax/libs/jquery-migrate/1.4.1/jquery-migrate.min.js', array(
//    'type' => 'external',
//    'group' => JS_THEME,
//    'every_page' => TRUE,
//    'weight' => -20,
//    'version' => '3.0.0',
//  ));

  drupal_add_js(UIKIT_SRC . "/js/uikit.min.js", array(
    'type' => 'external',
    'group' => JS_THEME,
    'every_page' => TRUE,
    'weight' => -20,
    'version' => $uikit_version,
  ));

  // Add extra UIKIT components.
  $css_extension = $uikit_style ? $uikit_style . '.' : '';
  $css_extension .= 'min.css';
  // Progress bars.
  drupal_add_css(UIKIT_SRC . "/css/components/progress." . $css_extension, array(
    'type' => 'external',
    'group' => CSS_THEME,
    'every_page' => TRUE,
    'version' => $uikit_version,
  ));

  // Date picker.
  drupal_add_css(UIKIT_SRC . "/css/components/datepicker." . $css_extension, array(
    'type' => 'external',
    'group' => CSS_THEME,
    'every_page' => TRUE,
    'version' => $uikit_version,
  ));
  drupal_add_js(UIKIT_SRC . "/js/components/datepicker.min.js", array(
    'type' => 'external',
    'group' => JS_THEME,
    'every_page' => TRUE,
    'version' => $uikit_version,
  ));

  // Tooltips.
  drupal_add_css(UIKIT_SRC . "/css/components/tooltip." . $css_extension, array(
    'type' => 'external',
    'group' => CSS_THEME,
    'every_page' => TRUE,
    'version' => $uikit_version,
  ));
  drupal_add_js(UIKIT_SRC . "/js/components/tooltip.min.js", array(
    'type' => 'external',
    'group' => JS_THEME,
    'every_page' => TRUE,
    'version' => $uikit_version,
  ));

  // Nestable.
  drupal_add_css(UIKIT_SRC . "/css/components/nestable." . $css_extension, array(
    'type' => 'external',
    'group' => CSS_THEME,
    'every_page' => TRUE,
    'version' => $uikit_version,
  ));
  drupal_add_js(UIKIT_SRC . "/js/components/nestable.min.js", array(
    'type' => 'external',
    'group' => JS_THEME,
    'every_page' => TRUE,
    'version' => $uikit_version,
  ));
}
