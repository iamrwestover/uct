<?php

/**
 * @file
 * Returns HTML for a single local task link.
 *
 * Available variables:
 * $variables['element']: A render element containing:
 *   - #link: A menu link array with 'title', 'href', and 'localized_options'
 *     keys.
 *   - #active: A boolean indicating whether the local task is active.
 *
 * @see theme_menu_local_task()
 *
 * @ingroup uikit_themeable
 */

$link = $variables['element']['#link'];
$link_text = $link['title'];

// Add keyboard shortcuts.
if (!empty($link['path'])) {
  $ks = '';
  if (strstr($link['path'], '/view')) {
    $ks = 'F1';
  }
  elseif (strstr($link['path'], '/edit')) {
    $ks = 'F2';
  }
  elseif (strstr($link['path'], '/new')) {
    //$ttype = arg(1);
    //$ttd = avtxns_txn_types($ttype);
    //$ks = empty($ttd['data-ks']) ? '' : $ttd['data-ks'];
    $ks = 'F4';
  }
  elseif (strstr($link['path'], '/return')) {
    $ks = 'F6';
  }
  elseif (strstr($link['path'], '/apply-credits')) {
    $ks = 'F8';
  }
  elseif (strstr($link['path'], '/related')) {
    $ks = 'F9';
  }
  if (!empty($ks)) {
    $link['localized_options']['attributes']['data-ks'][] = $ks;
  }
}

if (!empty($variables['element']['#active'])) {
  // Add text to indicate active tab for non-visual users.
  $active = '<span class="uk-hidden">' . t('(active tab)') . '</span>';

  // If the link does not contain HTML already, check_plain() it now.
  // After we set 'html'=TRUE the link will not be sanitized by l().
  if (empty($link['localized_options']['html'])) {
    $link['title'] = check_plain($link['title']);
  }
  $link['localized_options']['html'] = TRUE;
  $link_text = t('!local-task-title!active', array('!local-task-title' => $link['title'], '!active' => $active));
}

print '<li' . (!empty($variables['element']['#active']) ? ' class="uk-active"' : '') . '>' . l($link_text, $link['href'], $link['localized_options']) . "</li>\n";
