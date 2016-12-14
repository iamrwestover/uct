<?php
  $form['submit']['#attributes']['class'][] = 'uk-button-primary';
  $buttons = array();
  foreach (element_children($form) as $key) {
    $position = empty($form[$key]['#avposition']) ? 'normal' : $form[$key]['#avposition'];
    $buttons[$position][] = $form[$key];
  }
?>
<div class="uk-grid uk-margin-top">
  <div class="uk-width-1-2"><?php print drupal_render($buttons['opposite']); ?></div>
  <div class="uk-width-1-2 uk-text-right"><?php print drupal_render($buttons['normal']); ?></div>
</div>
