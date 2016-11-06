<?php
print '<div class="uk-panel uk-panel-box">' . render($variables['form']['title']) . '</div>';
hide($variables['form']['first_name']);
print drupal_render_children($variables['form']);
?>
x
