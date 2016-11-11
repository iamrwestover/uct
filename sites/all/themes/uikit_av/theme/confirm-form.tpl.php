<?php
$form['actions']['submit']['#attributes']['class'][] = 'uk-button-danger';
hide($form['actions']);
print drupal_render_children($form);
?>
<div class="uk-margin">
  <?php print render($form['actions']); ?>
</div>
