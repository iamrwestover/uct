<div class="uk-grid uk-grid-small uk-margin-top">
  <div class="uk-width-1-10">
    <?php print drupal_render($form['avtrans.id']); ?>
  </div>
  <div class="uk-width-9-10">
    <?php print drupal_render($form['search_text']); ?>
    <?php print drupal_render($form['search_button']); ?>
  </div>
</div>
<?php
print drupal_render_children($form);
