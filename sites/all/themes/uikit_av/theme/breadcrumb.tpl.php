<?php

/**
 * @file
 * Returns HTML for a breadcrumb trail.
 *
 * Available variables:
 * - $variables['breadcrumb']: An array containing the breadcrumb links.
 *
 * @see theme_breadcrumb()
 *
 * @ingroup uikit_themeable
 */

$breadcrumb = $variables['breadcrumb'];
$path = explode('/', current_path());

// Alter breadcrumb on backup and migrate admin pages.
if (arg(3) == 'backup_migrate' && $user->uid != 1) {
  unset($breadcrumb[1]);
  unset($breadcrumb[2]);
  unset($breadcrumb[3]);
}

if (isset($breadcrumb[1]) && strstr($breadcrumb[1], 'Transactions')) {
  $breadcrumb[1] = '<span data-uk-tooltip="{delay:\'500\', cls: \'tt-ks\'}" title=\'<i class="uk-icon-keyboard-o"></i> F4\'>' . $breadcrumb[1] . '</span>';
}

if (arg(0) == 'user') {
  $breadcrumb[1] = l('Users', 'av/users');
}

// Provide a navigational heading to give context for breadcrumb links to
// screen-reader users. Make the heading invisible with .uk-hidden.
$output = '<h2 class="uk-hidden">' . t('You are here') . '</h2>';
$output .= '<ul class="uk-breadcrumb">';
foreach ($breadcrumb as $crumb) {
  $output .= '<li>' . $crumb . '</li>';
}
$page_title = drupal_is_front_page() ? t('Home') : drupal_get_title();
$output .= '<li>' . $page_title . '</li>';
$output .= '</ul>';

print $output;
