<?php

/**
 * @file
 * Returns HTML for a set of filter tips.
 *
 * Available variables:
 * - $variables['tips']: An array containing descriptions and a CSS ID in the
 *   form of 'module-name/filter-id' (only used when $long is TRUE) for each
 *   filter in one or more text formats. Example:
 *   @code
 *     array(
 *       'Full HTML' => array(
 *         0 => array(
 *           'tip' => 'Web page addresses and e-mail addresses turn into links automatically.',
 *           'id' => 'filter/2',
 *         ),
 *       ),
 *     );
 *   @endcode
 * - $variables['long']: (optional) Whether the passed-in filter tips contain
 *   extended explanations, i.e. intended to be output on the path 'filter/tips'
 *   (TRUE), or are in a short format, i.e. suitable to be displayed below a
 *   form element. Defaults to FALSE.
 *
 * @see _filter_tips()
 * @see theme_filter_tips()
 *
 * @ingroup uikit_themeable
 */

$tips = $variables['tips'];
$long = $variables['long'];
$output = '';

$multiple = count($tips) > 1;
if ($multiple) {
  $output = '<h2>' . t('Text Formats') . '</h2>';
}

if (count($tips)) {
  if ($multiple) {
    $output .= '<div class="compose-tips">';
  }
  foreach ($tips as $name => $tiplist) {
    if ($multiple) {
      $output .= '<div class="filter-type filter-' . drupal_html_class($name) . '">';
      $output .= '<h3>' . check_plain($name) . '</h3>';
    }

    if (count($tiplist) > 0) {
      $output .= '<ul class="tips uk-list uk-list-space">';
      foreach ($tiplist as $tip) {
        $output .= '<li' . ($long ? ' id="filter-' . str_replace("/", "-", $tip['id']) . '">' : '>') . $tip['tip'] . '</li>';
      }
      $output .= '</ul>';
    }

    if ($multiple) {
      $output .= '</div><hr class="uk-article-divider">';
    }
  }
  if ($multiple) {
    $output .= '</div>';
  }
}

print $output;
