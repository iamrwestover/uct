<?php
hide($form['pager']);
print drupal_render_children($form);
?>
<div class="uk-margin-top">
  <?php print render($form['pager']) ?>
</div>
