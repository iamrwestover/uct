<?php

/**
 * @file
 * Returns HTML for the secondary menu links.
 *
 * Available variables:
 * - $variables['links']: An associative array of links to be themed. The key
 *   for each link is used as its CSS class. Each link should be itself an
 *   array, with the following elements:
 *   - title: The link text.
 *   - href: The link URL. If omitted, the 'title' is shown as a plain text item
 *     in the links list.
 *   - html: (optional) Whether or not 'title' is HTML. If set, the title will
 *     not be passed through check_plain().
 *   - attributes: (optional) Attributes for the anchor, or for the <span> tag
 *     used in its place if no 'href' is supplied. If element 'class' is
 *     included, it must be an array of one or more class names.
 *   If the 'href' element is supplied, the entire link array is passed to l()
 *   as its $options parameter.
 * - $variables['attributes']: A keyed array of attributes for the UL containing
 *   the list of links.
 * - $variables['heading']: (optional) A heading to precede the links. May be an
 *   associative array or a string. If it's an array, it can have the following
 *   elements:
 *   - text: The heading text.
 *   - level: The heading level (e.g. 'h2', 'h3').
 *   - class: (optional) An array of the CSS classes for the heading. When using
 *     a string it will be used as the text of the heading and the level will
 *     default to 'h2'. Headings should be used on navigation menus and any list
 *     of links that consistently appears on multiple pages. To make the heading
 *     invisible use the 'element-invisible' CSS class. Do not use
 *     'display:none', which removes it from screen-readers and assistive
 *     technology. Headings allow screen-reader and keyboard only users to
 *     navigate to or skip the links. See
 *     http://juicystudio.com/article/screen-readers-display-none.php and
 *     http://www.w3.org/TR/WCAG-TECHS/H42.html for more information.
 *
 * @see uikit_preprocess_links()
 * @see theme_links()
 * @see links.tpl.php
 *
 * @ingroup uikit_themeable
 */

$menu_name = variable_get('menu_secondary_links_source', 'user-menu');
$menu_tree = menu_tree($menu_name);
$menu_tree['#theme_wrappers'] = array('menu_tree__system_secondary');

// Create custom theme wrapper to theme the offcanvas menu.
if (in_array('uk-nav-offcanvas', $variables['attributes']['class'])) {
  $menu_tree['#theme_wrappers'] = array('menu_tree__system_secondary__offcanvas');
}

$dropdown_support = theme_get_setting('secondary_menu_dropdown_support');

$icons = array(
  'user' => 'user',
  'user/logout' => 'sign-out',
);
//foreach (element_children($menu_tree) as $v) {
//  dpm($v . '=' . $menu_tree[$v]['#title'] . '=' . $menu_tree[$v]['#href']);
//}

foreach ($menu_tree as $key => $value) {
  // Add icons.
  if (isset($value['#href']) && array_key_exists($value['#href'], $icons)) {
    $menu_tree[$key]['#localized_options']['icon_key'] = $icons[$value['#href']];
    $menu_tree[$key]['#localized_options']['icon_classes'] = array('uk-icon-small', 'uk-margin-right');
    // Notification badge test.
//    if ($value['#href'] == 'user') {
//      $menu_tree[$key]['#title'] .= '<span class="uk-badge uk-badge-danger uk-badge-notification uk-margin-left">3</span>';
//    }
  }

  if (isset($value['#below']) && !empty($value['#below']) && $dropdown_support) {
    $menu_tree[$key]['#attributes']['class'][] = 'uk-parent';
    $menu_tree[$key]['#attributes']['data-uk-dropdown'] = '';
    $menu_tree[$key]['#below']['#theme_wrappers'] = array('menu_tree__system_secondary__sub_menu');
  }
  elseif (isset($value['#below']) && !empty($value['#below']) && !$dropdown_support) {
    $menu_tree[$key]['#below'] = '';
  }
}
print drupal_render($menu_tree);
